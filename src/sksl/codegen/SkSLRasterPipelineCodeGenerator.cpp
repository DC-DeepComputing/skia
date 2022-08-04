/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkSLDefines.h"
#include "include/private/SkSLLayout.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkSLString.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "include/private/SkTPin.h"
#include "include/sksl/SkSLOperator.h"
#include "include/sksl/SkSLPosition.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLChildCall.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorArrayCast.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExternalFunction.h"
#include "src/sksl/ir/SkSLExternalFunctionCall.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <string_view>
#include <type_traits>
#include <utility>

using StockStage = SkRasterPipeline::StockStage;

namespace SkSL {
class IRNode;

namespace {

// Model:
// We have two "registers", each is 4-wide
// Here we call them r0 and r1, but RP calls them 'r,g,b,a' and 'dr,dg,db,da'
//
// After figuring out the set of stages needed for a program, we allocate two
// blocks of memory from the arena - one for (scalar) uniforms, and one for
// (vector) variables or temporaries.
//
// We use pointers into those blocks as context parameters for memory ops.

enum class Storage {
    kRegister,  // R0/R1 (limited to values with 4 slots)
    kUniform,   // Uniform storage (values are scalar, not vector)
    kStack,     // Variables and temporaries needed for expression evaluation
    kLiteral,   // Literal values used within the program (de-deduped in the generator)
//    kIndirect,  // We probably need this eventually, to handle indirect references
};

// Location is enough to identify where the storage of a value (variable, temporary, etc...)
// lives, but only encodes the "start address". You also need the type's slot count to know
// how many slots it uses.
// Note that R0 and R1 can only hold four slots, but the other storage types support larger
// types (eg, matrices, structs, arrays, etc).
struct Location {
    Storage fStorage;
    int     fSlot;

    static Location R0() { return { Storage::kRegister, 0 }; }
    static Location R1() { return { Storage::kRegister, 1 }; }
};

struct Instruction {
    StockStage stage;
    Location   arg;    // Meaning varies depending on stage
};

}  // namespace

class SkRPGenerator {
public:
    SkRPGenerator(const Program& program, const SkStageRec& rec, SkRPCallbacks* callbacks);

    void writeProgram(SkSpan<skvm::Val> uniforms,
                      skvm::Coord device,
                      const FunctionDefinition& function,
                      SkSpan<skvm::Val> arguments,
                      SkSpan<skvm::Val> outReturn);

private:
    /**
     * In SkSL, a Variable represents a named, typed value (along with qualifiers, etc).
     * Every Variable is mapped to one (or several, contiguous) values in the kStack or kGlobal
     * storage block. Those blocks are "RAM".
     */

    /** Emits an SkSL function. Returns the slot index of the SkSL function's return value. */
    size_t writeFunction(const IRNode& caller,
                         const FunctionDefinition& function,
                         SkSpan<skvm::Val> arguments);

    size_t fieldSlotOffset(const FieldAccess& expr);
    size_t indexSlotOffset(const IndexExpression& expr);

    // Where do we want the expression result to end up, if possible?
    enum class Request { R0, R1 };

    Location writeExpression(const Expression& expr);
    Location writeBinaryExpression(const BinaryExpression& b);
    Location writeAggregationConstructor(const AnyConstructor& c);
    Location writeChildCall(const ChildCall& c);
    Location writeConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c);
    Location writeConstructorMatrixResize(const ConstructorMatrixResize& c);
    Location writeConstructorCast(const AnyConstructor& c);
    Location writeConstructorSplat(const ConstructorSplat& c);
    Location writeFunctionCall(const FunctionCall& c);
    Location writeExternalFunctionCall(const ExternalFunctionCall& c);
    Location writeFieldAccess(const FieldAccess& expr);
    Location writeLiteral(const Literal& l, Request);
    Location writeIndexExpression(const IndexExpression& expr);
    Location writeIntrinsicCall(const FunctionCall& c);
    Location writePostfixExpression(const PostfixExpression& p);
    Location writePrefixExpression(const PrefixExpression& p);
    Location writeSwizzle(const Swizzle& swizzle);
    Location writeTernaryExpression(const TernaryExpression& t);
    Location writeVariableExpression(const VariableReference& expr);

    Location writeTypeConversion(const Value& src, Type::NumberKind srcKind, Type::NumberKind dstKind);

    void writeStatement(const Statement& s);
    void writeBlock(const Block& b);
    void writeBreakStatement();
    void writeContinueStatement();
    void writeForStatement(const ForStatement& f);
    void writeIfStatement(const IfStatement& stmt);
    void writeReturnStatement(const ReturnStatement& r);
    void writeSwitchStatement(const SwitchStatement& s);
    void writeVarDeclaration(const VarDeclaration& decl);

    Location writeStore(const Expression& lhs, const Value& rhs);
    skvm::Val writeConditionalStore(skvm::Val lhs, skvm::Val rhs, skvm::I32 mask);

    Location pushStack(int slots) {
        Location loc = { Storage::kStack, fStackSize };
        fStackSize += slots;
        fStackMaxSize = std::max(fStackSize, fStackMaxSize);
        return loc;
    }

    struct StackScope {
        StackScope(SkRPGenerator* generator)
                : fGenerator(generator)
                , fStartingSize(fGenerator->fStackSize) {}
        ~StackScope() {
            SkASSERT(fGenerator->fStackSize >= fStartingSize);
            fGenerator->fStackSize = fStartingSize;
        }

        SkRPGenerator* fGenerator;
        int fStartingSize;
    };

    // Used when appending a stage (eg, load) that can directly target src (R0) or dst (R1).
    // Assumes that the `_dst` version immediately follows the src version.
    void append(StockStage stage, Request req, Location arg) {
        if (req == Request::R1) {
            stage = static_cast<StockStage>(stage + 1);
        }
        fInstructions.push_back({ stage, arg });
    }

    //
    // Global state for the lifetime of the generator:
    //
    const Program&    fProgram;
    const SkStageRec& fRec;
    SkRPCallbacks*    fCallbacks;

    std::vector<Instruction> fInstructions;

    int fStackSize = 0;
    int fStackMaxSize = 0;

    std::vector<int32_t> fLiterals;

    Location fConditionMask;
    Location fLoopMask;
    Location fContinueMask;
};

static Type::NumberKind base_number_kind(const Type& type) {
    if (type.typeKind() == Type::TypeKind::kMatrix || type.typeKind() == Type::TypeKind::kVector) {
        return base_number_kind(type.componentType());
    }
    return type.numberKind();
}

static inline bool is_uniform(const SkSL::Variable& var) {
    return var.modifiers().fFlags & Modifiers::kUniform_Flag;
}

SkRPGenerator::SkRPGenerator(const Program& program,
                             const SkStageRec& rec,
                             SkRPCallbacks* callbacks)
        : fProgram(program)
        , fRec(rec)
        , fCallbacks(callbacks) {}

void SkRPGenerator::writeProgram(SkSpan<skvm::Val> uniforms,
                                 skvm::Coord device,
                                 const FunctionDefinition& function,
                                 SkSpan<skvm::Val> arguments,
                                 SkSpan<skvm::Val> outReturn) {
    fConditionMask = fLoopMask = fBuilder->splat(0xffff'ffff);

    this->setupGlobals(uniforms, device);
    size_t returnSlot = this->writeFunction(function, function, arguments);

    // Copy the value from the return slot into outReturn.
    SkASSERT(function.declaration().returnType().slotCount() == outReturn.size());
    for (size_t i = 0; i < outReturn.size(); ++i) {
        outReturn[i] = fSlots[returnSlot + i].val;
    }
}

void SkRPGenerator::determineLineOffsets() {
    SkASSERT(fLineOffsets.empty());
    fLineOffsets.push_back(0);
    for (size_t i = 0; i < fProgram.fSource->length(); ++i) {
        if ((*fProgram.fSource)[i] == '\n') {
            fLineOffsets.push_back(i);
        }
    }
    fLineOffsets.push_back(fProgram.fSource->length());
}

void SkRPGenerator::setupGlobals(SkSpan<skvm::Val> uniforms, skvm::Coord device) {
    // Add storage for each global variable (including uniforms) to fSlots, and entries in
    // fSlotMap to remember where every variable is stored.
    const skvm::Val* uniformIter = uniforms.begin();
    size_t fpCount = 0;
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const GlobalVarDeclaration& gvd = e->as<GlobalVarDeclaration>();
            const VarDeclaration& decl = gvd.declaration()->as<VarDeclaration>();
            const Variable& var = decl.var();
            SkASSERT(!fSlotMap.find(&var));

            // For most variables, fSlotMap stores an index into fSlots, but for children,
            // fSlotMap stores the index to pass to fSample(Shader|ColorFilter|Blender)
            if (var.type().isEffectChild()) {
                fSlotMap.set(&var, fpCount++);
                continue;
            }

            // Opaque types include child processors and GL objects (samplers, textures, etc).
            // Of those, only child processors are legal variables.
            SkASSERT(!var.type().isVoid());
            SkASSERT(!var.type().isOpaque());

            // getSlot() allocates space for the variable's value in fSlots, initializes it to zero,
            // and populates fSlotMap.
            size_t slot   = this->getSlot(var),
                   nslots = var.type().slotCount();

            // builtin variables are system-defined, with special semantics. The only builtin
            // variable exposed to runtime effects is sk_FragCoord.
            if (int builtin = var.modifiers().fLayout.fBuiltin; builtin >= 0) {
                switch (builtin) {
                    case SK_FRAGCOORD_BUILTIN:
                        SkASSERT(nslots == 4);
                        this->writeToSlot(slot + 0, device.x.id);
                        this->writeToSlot(slot + 1, device.y.id);
                        this->writeToSlot(slot + 2, fBuilder->splat(0.0f).id);
                        this->writeToSlot(slot + 3, fBuilder->splat(1.0f).id);
                        break;
                    default:
                        SkDEBUGFAILF("Unsupported builtin %d", builtin);
                }
                continue;
            }

            // For uniforms, copy the supplied IDs over
            if (is_uniform(var)) {
                SkASSERT(uniformIter + nslots <= uniforms.end());
                for (size_t i = 0; i < nslots; ++i) {
                    this->writeToSlot(slot + i, uniformIter[i]);
                }
                uniformIter += nslots;
                continue;
            }

            // For other globals, populate with the initializer expression (if there is one)
            if (decl.value()) {
                Value val = this->writeExpression(*decl.value());
                for (size_t i = 0; i < nslots; ++i) {
                    this->writeToSlot(slot + i, val[i]);
                }
            }
        }
    }
    SkASSERT(uniformIter == uniforms.end());
}

Value SkRPGenerator::getSlotValue(size_t slot, size_t nslots) {
    Value val(nslots);
    for (size_t i = 0; i < nslots; ++i) {
        val[i] = fSlots[slot + i].val;
    }
    return val;
}

size_t SkRPGenerator::writeFunction(const IRNode& caller,
                                    const FunctionDefinition& function,
                                    SkSpan<skvm::Val> arguments) {
    const FunctionDeclaration& decl = function.declaration();

    size_t returnSlot = this->getFunctionSlot(caller, function);
    fFunctionStack.push_back({/*fReturnSlot=*/returnSlot, /*fReturned=*/fBuilder->splat(0)});

    // For all parameters, copy incoming argument IDs to our vector of (all) variable IDs
    size_t argIdx = 0;
    for (const Variable* p : decl.parameters()) {
        size_t paramSlot = this->getSlot(*p),
               nslots    = p->type().slotCount();

        for (size_t i = 0; i < nslots; ++i) {
            fSlots[paramSlot + i].writtenTo = false;
            this->writeToSlot(paramSlot + i, arguments[argIdx + i]);
        }
        argIdx += nslots;
    }
    SkASSERT(argIdx == arguments.size());

    this->writeBlock(function.body()->as<Block>());

    // Copy 'out' and 'inout' parameters back to their caller-supplied argument storage
    argIdx = 0;
    for (const Variable* p : decl.parameters()) {
        size_t nslots = p->type().slotCount();

        if (p->modifiers().fFlags & Modifiers::kOut_Flag) {
            size_t paramSlot = this->getSlot(*p);
            for (size_t i = 0; i < nslots; ++i) {
                arguments[argIdx + i] = fSlots[paramSlot + i].val;
            }
        }
        argIdx += nslots;
    }
    SkASSERT(argIdx == arguments.size());

    fFunctionStack.pop_back();

    return returnSlot;
}

void SkRPGenerator::writeToSlot(int slot, skvm::Val value) {
    fSlots[slot].val = value;
}

size_t SkRPGenerator::createSlot(const std::string& name,
                                 const Type& type,
                                 int line,
                                 int fnReturnValue) {
    size_t slot   = fSlots.size(),
           nslots = type.slotCount();

    if (nslots > 0) {
        // Create brand new slots initialized to zero.
        skvm::Val initialValue = fBuilder->splat(0.0f).id;
        fSlots.insert(fSlots.end(), nslots, Slot{initialValue});
    }
    return slot;
}

size_t SkRPGenerator::getSlot(const Variable& v) {
    size_t* entry = fSlotMap.find(&v);
    if (entry != nullptr) {
        return *entry;
    }

    size_t slot = this->createSlot(std::string(v.name()), v.type(), this->getLine(v.fPosition),
            /*fnReturnValue=*/-1);
    fSlotMap.set(&v, slot);
    return slot;
}

size_t SkRPGenerator::getFunctionSlot(const IRNode& callSite, const FunctionDefinition& fn) {
    size_t* entry = fSlotMap.find(&callSite);
    if (entry != nullptr) {
        return *entry;
    }

    const FunctionDeclaration& decl = fn.declaration();
    size_t slot = this->createSlot("[" + std::string(decl.name()) + "].result",
                                   decl.returnType(),
                                   this->getLine(fn.fPosition),
                                   /*fnReturnValue=*/1);
    fSlotMap.set(&callSite, slot);
    return slot;
}

void SkRPGenerator::recursiveBinaryCompare(
        const Value& lVal,
        const Type& lType,
        const Value& rVal,
        const Type& rType,
        size_t* slotOffset,
        Value* result,
        const std::function<Value(skvm::F32 x, skvm::F32 y)>& float_comp,
        const std::function<Value(skvm::I32 x, skvm::I32 y)>& int_comp) {
    switch (lType.typeKind()) {
        case Type::TypeKind::kStruct:
            SkASSERT(rType.typeKind() == Type::TypeKind::kStruct);
            // Go through all the fields
            for (size_t f = 0; f < lType.fields().size(); ++f) {
                const Type::Field& lField = lType.fields()[f];
                const Type::Field& rField = rType.fields()[f];
                this->recursiveBinaryCompare(lVal,
                                             *lField.fType,
                                             rVal,
                                             *rField.fType,
                                             slotOffset,
                                             result,
                                             float_comp,
                                             int_comp);
            }
            break;

        case Type::TypeKind::kArray:
        case Type::TypeKind::kVector:
        case Type::TypeKind::kMatrix:
            SkASSERT(lType.typeKind() == rType.typeKind());
            // Go through all the elements
            for (int c = 0; c < lType.columns(); ++c) {
                this->recursiveBinaryCompare(lVal,
                                             lType.componentType(),
                                             rVal,
                                             rType.componentType(),
                                             slotOffset,
                                             result,
                                             float_comp,
                                             int_comp);
            }
            break;
        default:
            SkASSERT(lType.typeKind() == rType.typeKind() &&
                     lType.slotCount() == rType.slotCount());
            Type::NumberKind nk = base_number_kind(lType);
            auto L = lVal[*slotOffset];
            auto R = rVal[*slotOffset];
            (*result)[*slotOffset] =
                    i32(nk == Type::NumberKind::kFloat
                          ? float_comp(f32(L), f32(R))
                          : int_comp(i32(L), i32(R))).id;
            *slotOffset += lType.slotCount();
            break;
    }
}

Location SkRPGenerator::writeBinaryExpression(const BinaryExpression& b) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Operator op = b.getOperator();
    if (op.kind() == Operator::Kind::EQ) {
        return this->writeStore(left, this->writeExpression(right));
    }

    const Type& lType = left.type();
    const Type& rType = right.type();
    bool lVecOrMtx = (lType.isVector() || lType.isMatrix());
    bool rVecOrMtx = (rType.isVector() || rType.isMatrix());
    bool isAssignment = op.isAssignment();
    if (isAssignment) {
        op = op.removeAssignment();
    }
    Type::NumberKind nk = base_number_kind(lType);

    // A few ops require special treatment:
    switch (op.kind()) {
        case Operator::Kind::LOGICALAND: {
            SkASSERT(!isAssignment);
            SkASSERT(nk == Type::NumberKind::kBoolean);
            skvm::I32 lVal = i32(this->writeExpression(left));
            ScopedCondition shortCircuit(this, lVal);
            skvm::I32 rVal = i32(this->writeExpression(right));
            return lVal & rVal;
        }
        case Operator::Kind::LOGICALOR: {
            SkASSERT(!isAssignment);
            SkASSERT(nk == Type::NumberKind::kBoolean);
            skvm::I32 lVal = i32(this->writeExpression(left));
            ScopedCondition shortCircuit(this, ~lVal);
            skvm::I32 rVal = i32(this->writeExpression(right));
            return lVal | rVal;
        }
        case Operator::Kind::COMMA:
            // We write the left side of the expression to preserve its side effects, even though we
            // immediately discard the result.
            this->writeExpression(left);
            return this->writeExpression(right);
        default:
            break;
    }

    // All of the other ops always evaluate both sides of the expression
    Value lVal = this->writeExpression(left),
          rVal = this->writeExpression(right);

    // Special case for M*V, V*M, M*M (but not V*V!)
    if (op.kind() == Operator::Kind::STAR
        && lVecOrMtx && rVecOrMtx && !(lType.isVector() && rType.isVector())) {
        int rCols = rType.columns(),
            rRows = rType.rows(),
            lCols = lType.columns(),
            lRows = lType.rows();
        // M*V treats the vector as a column
        if (rType.isVector()) {
            std::swap(rCols, rRows);
        }
        SkASSERT(lCols == rRows);
        SkASSERT(b.type().slotCount() == static_cast<size_t>(lRows * rCols));
        Value result(lRows * rCols);
        size_t resultIdx = 0;
        const skvm::F32 zero = fBuilder->splat(0.0f);
        for (int c = 0; c < rCols; ++c)
        for (int r = 0; r < lRows; ++r) {
            skvm::F32 sum = zero;
            for (int j = 0; j < lCols; ++j) {
                sum += f32(lVal[j*lRows + r]) * f32(rVal[c*rRows + j]);
            }
            result[resultIdx++] = sum;
        }
        SkASSERT(resultIdx == result.slots());
        return isAssignment ? this->writeStore(left, result) : result;
    }

    size_t nslots = std::max(lVal.slots(), rVal.slots());

    auto binary = [&](const std::function <Value(skvm::F32 x, skvm::F32 y)>& f_fn,
                      const std::function <Value(skvm::I32 x, skvm::I32 y)>& i_fn,
                      bool foldResults = false) -> Value {

        Value result(nslots);
        if (op.isEquality() && (lType.isStruct() || lType.isArray())) {
            // Shifting over lVal and rVal
            size_t slotOffset = 0;
            this->recursiveBinaryCompare(
                    lVal, lType, rVal, rType, &slotOffset, &result, f_fn, i_fn);
            SkASSERT(slotOffset == nslots);
        } else {
            for (size_t slot = 0; slot < nslots; ++slot) {
                // If one side is scalar, replicate it to all channels
                skvm::Val L = lVal.slots() == 1 ? lVal[0] : lVal[slot],
                          R = rVal.slots() == 1 ? rVal[0] : rVal[slot];

                if (nk == Type::NumberKind::kFloat) {
                    result[slot] = i32(f_fn(f32(L), f32(R)));
                } else {
                    result[slot] = i32(i_fn(i32(L), i32(R)));
                }
            }
        }

        if (foldResults && nslots > 1) {
            SkASSERT(op.isEquality());
            skvm::I32 folded = i32(result[0]);
            for (size_t i = 1; i < nslots; ++i) {
                if (op.kind() == Operator::Kind::NEQ) {
                    folded |= i32(result[i]);
                } else {
                    folded &= i32(result[i]);
                }
            }
            return folded;
        }

        return isAssignment ? this->writeStore(left, result) : result;
    };

    auto unsupported_f = [&](skvm::F32, skvm::F32) {
        SkDEBUGFAIL("Unsupported operator");
        return skvm::F32{};
    };

    switch (op.kind()) {
        case Operator::Kind::EQEQ:
            SkASSERT(!isAssignment);
            return binary([](skvm::F32 x, skvm::F32 y) { return x == y; },
                          [](skvm::I32 x, skvm::I32 y) { return x == y; }, /*foldResults=*/ true);
        case Operator::Kind::NEQ:
            SkASSERT(!isAssignment);
            return binary([](skvm::F32 x, skvm::F32 y) { return x != y; },
                          [](skvm::I32 x, skvm::I32 y) { return x != y; }, /*foldResults=*/ true);
        case Operator::Kind::GT:
            return binary([](skvm::F32 x, skvm::F32 y) { return x > y; },
                          [](skvm::I32 x, skvm::I32 y) { return x > y; });
        case Operator::Kind::GTEQ:
            return binary([](skvm::F32 x, skvm::F32 y) { return x >= y; },
                          [](skvm::I32 x, skvm::I32 y) { return x >= y; });
        case Operator::Kind::LT:
            return binary([](skvm::F32 x, skvm::F32 y) { return x < y; },
                          [](skvm::I32 x, skvm::I32 y) { return x < y; });
        case Operator::Kind::LTEQ:
            return binary([](skvm::F32 x, skvm::F32 y) { return x <= y; },
                          [](skvm::I32 x, skvm::I32 y) { return x <= y; });

        case Operator::Kind::PLUS:
            return binary([](skvm::F32 x, skvm::F32 y) { return x + y; },
                          [](skvm::I32 x, skvm::I32 y) { return x + y; });
        case Operator::Kind::MINUS:
            return binary([](skvm::F32 x, skvm::F32 y) { return x - y; },
                          [](skvm::I32 x, skvm::I32 y) { return x - y; });
        case Operator::Kind::STAR:
            return binary([](skvm::F32 x, skvm::F32 y) { return x ** y; },
                          [](skvm::I32 x, skvm::I32 y) { return x * y; });
        case Operator::Kind::SLASH:
            // Minimum spec (GLSL ES 1.0) has very loose requirements for integer operations.
            // (Low-end GPUs may not have integer ALUs). Given that, we are allowed to do floating
            // point division plus rounding. Section 10.28 of the spec even clarifies that the
            // rounding mode is undefined (but round-towards-zero is the obvious/common choice).
            return binary([](skvm::F32 x, skvm::F32 y) { return x / y; },
                          [](skvm::I32 x, skvm::I32 y) {
                              return skvm::trunc(skvm::to_F32(x) / skvm::to_F32(y));
                          });

        case Operator::Kind::BITWISEXOR:
        case Operator::Kind::LOGICALXOR:
            return binary(unsupported_f, [](skvm::I32 x, skvm::I32 y) { return x ^ y; });
        case Operator::Kind::BITWISEAND:
            return binary(unsupported_f, [](skvm::I32 x, skvm::I32 y) { return x & y; });
        case Operator::Kind::BITWISEOR:
            return binary(unsupported_f, [](skvm::I32 x, skvm::I32 y) { return x | y; });

        // These three operators are all 'reserved' (illegal) in our minimum spec, but will require
        // implementation in the future.
        case Operator::Kind::PERCENT:
        case Operator::Kind::SHL:
        case Operator::Kind::SHR:
        default:
            SkDEBUGFAIL("Unsupported operator");
            return {};
    }
}

Value SkRPGenerator::writeAggregationConstructor(const AnyConstructor& c) {
    Value result(c.type().slotCount());
    size_t resultIdx = 0;
    for (const auto &arg : c.argumentSpan()) {
        Value tmp = this->writeExpression(*arg);
        for (size_t tmpSlot = 0; tmpSlot < tmp.slots(); ++tmpSlot) {
            result[resultIdx++] = tmp[tmpSlot];
        }
    }
    return result;
}

Value SkRPGenerator::writeTypeConversion(const Value& src,
                                         Type::NumberKind srcKind,
                                         Type::NumberKind dstKind) {
    // Conversion among "similar" types (floatN <-> halfN), (shortN <-> intN), etc. is a no-op.
    if (srcKind == dstKind) {
        return src;
    }

    // TODO: Handle signed vs. unsigned. GLSL ES 1.0 only has 'int', so no problem yet.
    Value dst(src.slots());
    switch (dstKind) {
        case Type::NumberKind::kFloat:
            if (srcKind == Type::NumberKind::kSigned) {
                // int -> float
                for (size_t i = 0; i < src.slots(); ++i) {
                    dst[i] = skvm::to_F32(i32(src[i]));
                }
                return dst;
            }
            if (srcKind == Type::NumberKind::kBoolean) {
                // bool -> float
                for (size_t i = 0; i < src.slots(); ++i) {
                    dst[i] = skvm::select(i32(src[i]), 1.0f, 0.0f);
                }
                return dst;
            }
            break;

        case Type::NumberKind::kSigned:
            if (srcKind == Type::NumberKind::kFloat) {
                // float -> int
                for (size_t i = 0; i < src.slots(); ++i) {
                    dst[i] = skvm::trunc(f32(src[i]));
                }
                return dst;
            }
            if (srcKind == Type::NumberKind::kBoolean) {
                // bool -> int
                for (size_t i = 0; i < src.slots(); ++i) {
                    dst[i] = skvm::select(i32(src[i]), 1, 0);
                }
                return dst;
            }
            break;

        case Type::NumberKind::kBoolean:
            if (srcKind == Type::NumberKind::kSigned) {
                // int -> bool
                for (size_t i = 0; i < src.slots(); ++i) {
                    dst[i] = i32(src[i]) != 0;
                }
                return dst;
            }
            if (srcKind == Type::NumberKind::kFloat) {
                // float -> bool
                for (size_t i = 0; i < src.slots(); ++i) {
                    dst[i] = f32(src[i]) != 0.0;
                }
                return dst;
            }
            break;

        default:
            break;
    }
    SkDEBUGFAILF("Unsupported type conversion: %d -> %d", (int)srcKind, (int)dstKind);
    return {};
}

Value SkRPGenerator::writeConstructorCast(const AnyConstructor& c) {
    auto arguments = c.argumentSpan();
    SkASSERT(arguments.size() == 1);
    const Expression& argument = *arguments.front();

    const Type& srcType = argument.type();
    const Type& dstType = c.type();
    Type::NumberKind srcKind = base_number_kind(srcType);
    Type::NumberKind dstKind = base_number_kind(dstType);
    Value src = this->writeExpression(argument);
    return this->writeTypeConversion(src, srcKind, dstKind);
}

Value SkRPGenerator::writeConstructorSplat(const ConstructorSplat& c) {
    SkASSERT(c.type().isVector());
    SkASSERT(c.argument()->type().isScalar());
    int columns = c.type().columns();

    // Splat the argument across all components of a vector.
    Value src = this->writeExpression(*c.argument());
    Value dst(columns);
    for (int i = 0; i < columns; ++i) {
        dst[i] = src[0];
    }
    return dst;
}

Value SkRPGenerator::writeConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& ctor) {
    const Type& dstType = ctor.type();
    SkASSERT(dstType.isMatrix());
    SkASSERT(ctor.argument()->type().matches(dstType.componentType()));

    Value src = this->writeExpression(*ctor.argument());
    Value dst(dstType.rows() * dstType.columns());
    size_t dstIndex = 0;

    // Matrix-from-scalar builds a diagonal scale matrix
    const skvm::F32 zero = fBuilder->splat(0.0f);
    for (int c = 0; c < dstType.columns(); ++c) {
        for (int r = 0; r < dstType.rows(); ++r) {
            dst[dstIndex++] = (c == r ? f32(src) : zero);
        }
    }

    SkASSERT(dstIndex == dst.slots());
    return dst;
}

Value SkRPGenerator::writeConstructorMatrixResize(const ConstructorMatrixResize& ctor) {
    const Type& srcType = ctor.argument()->type();
    const Type& dstType = ctor.type();
    Value src = this->writeExpression(*ctor.argument());
    Value dst(dstType.rows() * dstType.columns());

    // Matrix-from-matrix uses src where it overlaps, and fills in missing fields with identity.
    size_t dstIndex = 0;
    for (int c = 0; c < dstType.columns(); ++c) {
        for (int r = 0; r < dstType.rows(); ++r) {
            if (c < srcType.columns() && r < srcType.rows()) {
                dst[dstIndex++] = src[c * srcType.rows() + r];
            } else {
                dst[dstIndex++] = fBuilder->splat(c == r ? 1.0f : 0.0f);
            }
        }
    }

    SkASSERT(dstIndex == dst.slots());
    return dst;
}

size_t SkRPGenerator::fieldSlotOffset(const FieldAccess& expr) {
    size_t offset = 0;
    for (int i = 0; i < expr.fieldIndex(); ++i) {
        offset += (*expr.base()->type().fields()[i].fType).slotCount();
    }
    return offset;
}

Value SkRPGenerator::writeFieldAccess(const FieldAccess& expr) {
    Value base = this->writeExpression(*expr.base());
    Value field(expr.type().slotCount());
    size_t offset = this->fieldSlotOffset(expr);
    for (size_t i = 0; i < field.slots(); ++i) {
        field[i] = base[offset + i];
    }
    return field;
}

size_t SkRPGenerator::indexSlotOffset(const IndexExpression& expr) {
    Value index = this->writeExpression(*expr.index());
    int indexValue = -1;
    SkAssertResult(fBuilder->allImm(index[0], &indexValue));

    // When indexing by a literal, the front-end guarantees that we don't go out of bounds.
    // But when indexing by a loop variable, it's possible to generate out-of-bounds access.
    // The GLSL spec leaves that behavior undefined - we'll just clamp everything here.
    indexValue = SkTPin(indexValue, 0, expr.base()->type().columns() - 1);

    size_t stride = expr.type().slotCount();
    return indexValue * stride;
}

Value SkRPGenerator::writeIndexExpression(const IndexExpression& expr) {
    Value base = this->writeExpression(*expr.base());
    Value element(expr.type().slotCount());
    size_t offset = this->indexSlotOffset(expr);
    for (size_t i = 0; i < element.slots(); ++i) {
        element[i] = base[offset + i];
    }
    return element;
}

Value SkRPGenerator::writeVariableExpression(const VariableReference& expr) {
    size_t slot = this->getSlot(*expr.variable());
    return this->getSlotValue(slot, expr.type().slotCount());
}

Value SkRPGenerator::writeChildCall(const ChildCall& c) {
    size_t* childPtr = fSlotMap.find(&c.child());
    SkASSERT(childPtr != nullptr);

    const Expression* arg = c.arguments()[0].get();
    Value argVal = this->writeExpression(*arg);
    skvm::Color color;

    switch (c.child().type().typeKind()) {
        case Type::TypeKind::kShader: {
            SkASSERT(c.arguments().size() == 1);
            SkASSERT(arg->type().matches(*fProgram.fContext->fTypes.fFloat2));
            skvm::Coord coord = {f32(argVal[0]), f32(argVal[1])};
            color = fCallbacks->sampleShader(*childPtr, coord);
            break;
        }
        case Type::TypeKind::kColorFilter: {
            SkASSERT(c.arguments().size() == 1);
            SkASSERT(arg->type().matches(*fProgram.fContext->fTypes.fHalf4) ||
                     arg->type().matches(*fProgram.fContext->fTypes.fFloat4));
            skvm::Color inColor = {f32(argVal[0]), f32(argVal[1]), f32(argVal[2]), f32(argVal[3])};
            color = fCallbacks->sampleColorFilter(*childPtr, inColor);
            break;
        }
        case Type::TypeKind::kBlender: {
            SkASSERT(c.arguments().size() == 2);
            SkASSERT(arg->type().matches(*fProgram.fContext->fTypes.fHalf4) ||
                     arg->type().matches(*fProgram.fContext->fTypes.fFloat4));
            skvm::Color srcColor = {f32(argVal[0]), f32(argVal[1]), f32(argVal[2]), f32(argVal[3])};

            arg = c.arguments()[1].get();
            argVal = this->writeExpression(*arg);
            SkASSERT(arg->type().matches(*fProgram.fContext->fTypes.fHalf4) ||
                     arg->type().matches(*fProgram.fContext->fTypes.fFloat4));
            skvm::Color dstColor = {f32(argVal[0]), f32(argVal[1]), f32(argVal[2]), f32(argVal[3])};

            color = fCallbacks->sampleBlender(*childPtr, srcColor, dstColor);
            break;
        }
        default: {
            SkDEBUGFAILF("cannot sample from type '%s'", c.child().type().description().c_str());
        }
    }

    Value result(4);
    result[0] = color.r;
    result[1] = color.g;
    result[2] = color.b;
    result[3] = color.a;
    return result;
}

Value SkRPGenerator::writeIntrinsicCall(const FunctionCall& c) {
    IntrinsicKind intrinsicKind = c.function().intrinsicKind();
    SkASSERT(intrinsicKind != kNotIntrinsic);

    const size_t nargs = c.arguments().size();
    const size_t kMaxArgs = 3;  // eg: clamp, mix, smoothstep
    Value args[kMaxArgs];
    SkASSERT(nargs >= 1 && nargs <= std::size(args));

    // All other intrinsics have at most three args, and those can all be evaluated up front:
    for (size_t i = 0; i < nargs; ++i) {
        args[i] = this->writeExpression(*c.arguments()[i]);
    }
    Type::NumberKind nk = base_number_kind(c.arguments()[0]->type());

    auto binary = [&](auto&& fn) {
        // Binary intrinsics are (vecN, vecN), (vecN, float), or (float, vecN)
        size_t nslots = std::max(args[0].slots(), args[1].slots());
        Value result(nslots);
        SkASSERT(args[0].slots() == nslots || args[0].slots() == 1);
        SkASSERT(args[1].slots() == nslots || args[1].slots() == 1);

        for (size_t i = 0; i < nslots; ++i) {
            result[i] = fn({fBuilder, args[0][args[0].slots() == 1 ? 0 : i]},
                           {fBuilder, args[1][args[1].slots() == 1 ? 0 : i]});
        }
        return result;
    };

    auto ternary = [&](auto&& fn) {
        // Ternary intrinsics are some combination of vecN and float
        size_t nslots = std::max({args[0].slots(), args[1].slots(), args[2].slots()});
        Value result(nslots);
        SkASSERT(args[0].slots() == nslots || args[0].slots() == 1);
        SkASSERT(args[1].slots() == nslots || args[1].slots() == 1);
        SkASSERT(args[2].slots() == nslots || args[2].slots() == 1);

        for (size_t i = 0; i < nslots; ++i) {
            result[i] = fn({fBuilder, args[0][args[0].slots() == 1 ? 0 : i]},
                           {fBuilder, args[1][args[1].slots() == 1 ? 0 : i]},
                           {fBuilder, args[2][args[2].slots() == 1 ? 0 : i]});
        }
        return result;
    };

    auto dot = [&](const Value& x, const Value& y) {
        SkASSERT(x.slots() == y.slots());
        skvm::F32 result = f32(x[0]) * f32(y[0]);
        for (size_t i = 1; i < x.slots(); ++i) {
            result += f32(x[i]) * f32(y[i]);
        }
        return result;
    };

    switch (intrinsicKind) {
        case k_radians_IntrinsicKind:
            return unary(args[0], [](skvm::F32 deg) { return deg * (SK_FloatPI / 180); });
        case k_degrees_IntrinsicKind:
            return unary(args[0], [](skvm::F32 rad) { return rad * (180 / SK_FloatPI); });

        case k_sin_IntrinsicKind: return unary(args[0], skvm::approx_sin);
        case k_cos_IntrinsicKind: return unary(args[0], skvm::approx_cos);
        case k_tan_IntrinsicKind: return unary(args[0], skvm::approx_tan);

        case k_asin_IntrinsicKind: return unary(args[0], skvm::approx_asin);
        case k_acos_IntrinsicKind: return unary(args[0], skvm::approx_acos);

        case k_atan_IntrinsicKind: return nargs == 1 ? unary(args[0], skvm::approx_atan)
                                                 : binary(skvm::approx_atan2);

        case k_pow_IntrinsicKind:
            return binary([](skvm::F32 x, skvm::F32 y) { return skvm::approx_powf(x, y); });
        case k_exp_IntrinsicKind:  return unary(args[0], skvm::approx_exp);
        case k_log_IntrinsicKind:  return unary(args[0], skvm::approx_log);
        case k_exp2_IntrinsicKind: return unary(args[0], skvm::approx_pow2);
        case k_log2_IntrinsicKind: return unary(args[0], skvm::approx_log2);

        case k_sqrt_IntrinsicKind: return unary(args[0], skvm::sqrt);
        case k_inversesqrt_IntrinsicKind:
            return unary(args[0], [](skvm::F32 x) { return 1.0f / skvm::sqrt(x); });

        case k_abs_IntrinsicKind: return unary(args[0], skvm::abs);
        case k_sign_IntrinsicKind:
            return unary(args[0], [](skvm::F32 x) { return select(x < 0, -1.0f,
                                                           select(x > 0, +1.0f, 0.0f)); });
        case k_floor_IntrinsicKind: return unary(args[0], skvm::floor);
        case k_ceil_IntrinsicKind:  return unary(args[0], skvm::ceil);
        case k_fract_IntrinsicKind: return unary(args[0], skvm::fract);
        case k_mod_IntrinsicKind:
            return binary([](skvm::F32 x, skvm::F32 y) { return x - y*skvm::floor(x / y); });

        case k_min_IntrinsicKind:
            return binary([](skvm::F32 x, skvm::F32 y) { return skvm::min(x, y); });
        case k_max_IntrinsicKind:
            return binary([](skvm::F32 x, skvm::F32 y) { return skvm::max(x, y); });
        case k_clamp_IntrinsicKind:
            return ternary(
                    [](skvm::F32 x, skvm::F32 lo, skvm::F32 hi) { return skvm::clamp(x, lo, hi); });
        case k_saturate_IntrinsicKind:
            return unary(args[0], [](skvm::F32 x) { return skvm::clamp01(x); });
        case k_mix_IntrinsicKind:
            return ternary(
                    [](skvm::F32 x, skvm::F32 y, skvm::F32 t) { return skvm::lerp(x, y, t); });
        case k_step_IntrinsicKind:
            return binary([](skvm::F32 edge, skvm::F32 x) { return select(x < edge, 0.0f, 1.0f); });
        case k_smoothstep_IntrinsicKind:
            return ternary([](skvm::F32 edge0, skvm::F32 edge1, skvm::F32 x) {
                skvm::F32 t = skvm::clamp01((x - edge0) / (edge1 - edge0));
                return t ** t ** (3 - 2 ** t);
            });

        case k_length_IntrinsicKind: return skvm::sqrt(dot(args[0], args[0]));
        case k_distance_IntrinsicKind: {
            Value vec = binary([](skvm::F32 x, skvm::F32 y) { return x - y; });
            return skvm::sqrt(dot(vec, vec));
        }
        case k_dot_IntrinsicKind: return dot(args[0], args[1]);
        case k_cross_IntrinsicKind: {
            skvm::F32 ax = f32(args[0][0]), ay = f32(args[0][1]), az = f32(args[0][2]),
                      bx = f32(args[1][0]), by = f32(args[1][1]), bz = f32(args[1][2]);
            Value result(3);
            result[0] = ay**bz - az**by;
            result[1] = az**bx - ax**bz;
            result[2] = ax**by - ay**bx;
            return result;
        }
        case k_normalize_IntrinsicKind: {
            skvm::F32 invLen = 1.0f / skvm::sqrt(dot(args[0], args[0]));
            return unary(args[0], [&](skvm::F32 x) { return x ** invLen; });
        }
        case k_faceforward_IntrinsicKind: {
            const Value &N    = args[0],
                        &I    = args[1],
                        &Nref = args[2];

            skvm::F32 dotNrefI = dot(Nref, I);
            return unary(N, [&](skvm::F32 n) { return select(dotNrefI<0, n, -n); });
        }
        case k_reflect_IntrinsicKind: {
            const Value &I = args[0],
                        &N = args[1];

            skvm::F32 dotNI = dot(N, I);
            return binary([&](skvm::F32 i, skvm::F32 n) {
                return i - 2**dotNI**n;
            });
        }
        case k_refract_IntrinsicKind: {
            const Value &I  = args[0],
                        &N  = args[1];
            skvm::F32   eta = f32(args[2]);

            skvm::F32 dotNI = dot(N, I),
                      k     = 1 - eta**eta**(1 - dotNI**dotNI);
            return binary([&](skvm::F32 i, skvm::F32 n) {
                return select(k<0, 0.0f, eta**i - (eta**dotNI + sqrt(k))**n);
            });
        }

        case k_matrixCompMult_IntrinsicKind:
            return binary([](skvm::F32 x, skvm::F32 y) { return x ** y; });
        case k_inverse_IntrinsicKind: {
            switch (args[0].slots()) {
                case  4: return this->writeMatrixInverse2x2(args[0]);
                case  9: return this->writeMatrixInverse3x3(args[0]);
                case 16: return this->writeMatrixInverse4x4(args[0]);
                default:
                    SkDEBUGFAIL("Invalid call to inverse");
                    return {};
            }
        }

        case k_lessThan_IntrinsicKind:
            return nk == Type::NumberKind::kFloat
                           ? binary([](skvm::F32 x, skvm::F32 y) { return x < y; })
                           : binary([](skvm::I32 x, skvm::I32 y) { return x < y; });
        case k_lessThanEqual_IntrinsicKind:
            return nk == Type::NumberKind::kFloat
                           ? binary([](skvm::F32 x, skvm::F32 y) { return x <= y; })
                           : binary([](skvm::I32 x, skvm::I32 y) { return x <= y; });
        case k_greaterThan_IntrinsicKind:
            return nk == Type::NumberKind::kFloat
                           ? binary([](skvm::F32 x, skvm::F32 y) { return x > y; })
                           : binary([](skvm::I32 x, skvm::I32 y) { return x > y; });
        case k_greaterThanEqual_IntrinsicKind:
            return nk == Type::NumberKind::kFloat
                           ? binary([](skvm::F32 x, skvm::F32 y) { return x >= y; })
                           : binary([](skvm::I32 x, skvm::I32 y) { return x >= y; });

        case k_equal_IntrinsicKind:
            return nk == Type::NumberKind::kFloat
                           ? binary([](skvm::F32 x, skvm::F32 y) { return x == y; })
                           : binary([](skvm::I32 x, skvm::I32 y) { return x == y; });
        case k_notEqual_IntrinsicKind:
            return nk == Type::NumberKind::kFloat
                           ? binary([](skvm::F32 x, skvm::F32 y) { return x != y; })
                           : binary([](skvm::I32 x, skvm::I32 y) { return x != y; });

        case k_any_IntrinsicKind: {
            skvm::I32 result = i32(args[0][0]);
            for (size_t i = 1; i < args[0].slots(); ++i) {
                result |= i32(args[0][i]);
            }
            return result;
        }
        case k_all_IntrinsicKind: {
            skvm::I32 result = i32(args[0][0]);
            for (size_t i = 1; i < args[0].slots(); ++i) {
                result &= i32(args[0][i]);
            }
            return result;
        }
        case k_not_IntrinsicKind: return unary(args[0], [](skvm::I32 x) { return ~x; });

        case k_toLinearSrgb_IntrinsicKind: {
            skvm::Color color = {
                    f32(args[0][0]), f32(args[0][1]), f32(args[0][2]), fBuilder->splat(1.0f)};
            color = fCallbacks->toLinearSrgb(color);
            Value result(3);
            result[0] = color.r;
            result[1] = color.g;
            result[2] = color.b;
            return result;
        }
        case k_fromLinearSrgb_IntrinsicKind: {
            skvm::Color color = {
                    f32(args[0][0]), f32(args[0][1]), f32(args[0][2]), fBuilder->splat(1.0f)};
            color = fCallbacks->fromLinearSrgb(color);
            Value result(3);
            result[0] = color.r;
            result[1] = color.g;
            result[2] = color.b;
            return result;
        }

        default:
            SkDEBUGFAILF("unsupported intrinsic %s", c.function().description().c_str());
            return {};
    }
    SkUNREACHABLE;
}

Value SkRPGenerator::writeFunctionCall(const FunctionCall& call) {
    if (call.function().isIntrinsic() && !call.function().definition()) {
        return this->writeIntrinsicCall(call);
    }

    const FunctionDeclaration& decl = call.function();
    SkASSERTF(decl.definition(), "no definition for function '%s'", decl.description().c_str());
    const FunctionDefinition& funcDef = *decl.definition();

    // Evaluate all arguments, gather the results into a contiguous list of IDs
    std::vector<skvm::Val> argVals;
    for (const auto& arg : call.arguments()) {
        Value v = this->writeExpression(*arg);
        for (size_t i = 0; i < v.slots(); ++i) {
            argVals.push_back(v[i]);
        }
    }

    size_t returnSlot;
    {
        // This merges currentFunction().fReturned into fConditionMask. Lanes that conditionally
        // returned in the current function would otherwise resume execution within the child.
        ScopedCondition m(this, ~currentFunction().fReturned);
        returnSlot = this->writeFunction(call, funcDef, SkSpan(argVals));
    }

    // Propagate new values of any 'out' params back to the original arguments
    const std::unique_ptr<Expression>* argIter = call.arguments().begin();
    size_t valIdx = 0;
    for (const Variable* p : decl.parameters()) {
        size_t nslots = p->type().slotCount();
        if (p->modifiers().fFlags & Modifiers::kOut_Flag) {
            Value v(nslots);
            for (size_t i = 0; i < nslots; ++i) {
                v[i] = argVals[valIdx + i];
            }
            const std::unique_ptr<Expression>& arg = *argIter;
            this->writeStore(*arg, v);
        }
        valIdx += nslots;
        argIter++;
    }

    // Create a result Value from the return slot
    return this->getSlotValue(returnSlot, call.type().slotCount());
}

Location SkRPGenerator::writeLiteral(const Literal& l, Request req) {
    int32_t value = 0;
    if (l.type().isFloat()) {
        value = SkFloat2Bits(l.as<Literal>().floatValue());
    } else if (l.type().isInteger()) {
        value = static_cast<int32_t>(l.as<Literal>().intValue());
    } else {
        SkASSERT(l.type().isBoolean());
        value = l.as<Literal>().boolValue() ? ~0 : 0;
    }

    int index = 0;
    if (auto it = std::find(fLiterals.begin(), fLiterals.end(), value); it != fLiterals.end()) {
        index = it - fLiterals.begin();
    } else {
        index = fLiterals.size();
        fLiterals.push_back(value);
    }

    this->append(StockStage::load_r, req, { Storage::kLiteral, index });
    return req == Request::R1 ? Location::R1() : Location::R0();
}

Value SkRPGenerator::writePrefixExpression(const PrefixExpression& p) {
    Value val = this->writeExpression(*p.operand());

    switch (p.getOperator().kind()) {
        case Operator::Kind::PLUSPLUS:
        case Operator::Kind::MINUSMINUS: {
            bool incr = p.getOperator().kind() == Operator::Kind::PLUSPLUS;

            switch (base_number_kind(p.type())) {
                case Type::NumberKind::kFloat:
                    val = f32(val) + fBuilder->splat(incr ? 1.0f : -1.0f);
                    break;
                case Type::NumberKind::kSigned:
                    val = i32(val) + fBuilder->splat(incr ? 1 : -1);
                    break;
                default:
                    SkASSERT(false);
                    return {};
            }
            return this->writeStore(*p.operand(), val);
        }
        case Operator::Kind::MINUS: {
            switch (base_number_kind(p.type())) {
                case Type::NumberKind::kFloat:
                    return this->unary(val, [](skvm::F32 x) { return -x; });
                case Type::NumberKind::kSigned:
                    return this->unary(val, [](skvm::I32 x) { return -x; });
                default:
                    SkASSERT(false);
                    return {};
            }
        }
        case Operator::Kind::LOGICALNOT:
        case Operator::Kind::BITWISENOT:
            return this->unary(val, [](skvm::I32 x) { return ~x; });
        default:
            SkASSERT(false);
            return {};
    }
}

Value SkRPGenerator::writePostfixExpression(const PostfixExpression& p) {
    switch (p.getOperator().kind()) {
        case Operator::Kind::PLUSPLUS:
        case Operator::Kind::MINUSMINUS: {
            Value old = this->writeExpression(*p.operand()),
                  val = old;
            SkASSERT(val.slots() == 1);
            bool incr = p.getOperator().kind() == Operator::Kind::PLUSPLUS;

            switch (base_number_kind(p.type())) {
                case Type::NumberKind::kFloat:
                    val = f32(val) + fBuilder->splat(incr ? 1.0f : -1.0f);
                    break;
                case Type::NumberKind::kSigned:
                    val = i32(val) + fBuilder->splat(incr ? 1 : -1);
                    break;
                default:
                    SkASSERT(false);
                    return {};
            }
            this->writeStore(*p.operand(), val);
            return old;
        }
        default:
            SkASSERT(false);
            return {};
    }
}

Value SkRPGenerator::writeSwizzle(const Swizzle& s) {
    Value base = this->writeExpression(*s.base());
    Value swizzled(s.components().size());
    for (size_t i = 0; i < s.components().size(); ++i) {
        swizzled[i] = base[s.components()[i]];
    }
    return swizzled;
}

Value SkRPGenerator::writeTernaryExpression(const TernaryExpression& t) {
    skvm::I32 test = i32(this->writeExpression(*t.test()));
    Value ifTrue, ifFalse;

    {
        ScopedCondition m(this, test);
        ifTrue = this->writeExpression(*t.ifTrue());
    }
    {
        ScopedCondition m(this, ~test);
        ifFalse = this->writeExpression(*t.ifFalse());
    }

    size_t nslots = ifTrue.slots();
    SkASSERT(nslots == ifFalse.slots());

    Value result(nslots);
    for (size_t i = 0; i < nslots; ++i) {
        result[i] = skvm::select(test, i32(ifTrue[i]), i32(ifFalse[i]));
    }
    return result;
}

Value SkRPGenerator::writeExpression(const Expression& e) {
    switch (e.kind()) {
        case Expression::Kind::kBinary:
            return this->writeBinaryExpression(e.as<BinaryExpression>());
        case Expression::Kind::kChildCall:
            return this->writeChildCall(e.as<ChildCall>());
        case Expression::Kind::kConstructorArray:
        case Expression::Kind::kConstructorCompound:
        case Expression::Kind::kConstructorStruct:
            return this->writeAggregationConstructor(e.asAnyConstructor());
        case Expression::Kind::kConstructorArrayCast:
            return this->writeExpression(*e.as<ConstructorArrayCast>().argument());
        case Expression::Kind::kConstructorDiagonalMatrix:
            return this->writeConstructorDiagonalMatrix(e.as<ConstructorDiagonalMatrix>());
        case Expression::Kind::kConstructorMatrixResize:
            return this->writeConstructorMatrixResize(e.as<ConstructorMatrixResize>());
        case Expression::Kind::kConstructorScalarCast:
        case Expression::Kind::kConstructorCompoundCast:
            return this->writeConstructorCast(e.asAnyConstructor());
        case Expression::Kind::kConstructorSplat:
            return this->writeConstructorSplat(e.as<ConstructorSplat>());
        case Expression::Kind::kFieldAccess:
            return this->writeFieldAccess(e.as<FieldAccess>());
        case Expression::Kind::kIndex:
            return this->writeIndexExpression(e.as<IndexExpression>());
        case Expression::Kind::kVariableReference:
            return this->writeVariableExpression(e.as<VariableReference>());
        case Expression::Kind::kLiteral:
            return this->writeLiteral(e.as<Literal>());
        case Expression::Kind::kFunctionCall:
            return this->writeFunctionCall(e.as<FunctionCall>());
        case Expression::Kind::kPrefix:
            return this->writePrefixExpression(e.as<PrefixExpression>());
        case Expression::Kind::kPostfix:
            return this->writePostfixExpression(e.as<PostfixExpression>());
        case Expression::Kind::kSwizzle:
            return this->writeSwizzle(e.as<Swizzle>());
        case Expression::Kind::kTernary:
            return this->writeTernaryExpression(e.as<TernaryExpression>());
        case Expression::Kind::kExternalFunctionCall:
        case Expression::Kind::kExternalFunctionReference:
        default:
            SkDEBUGFAIL("Unsupported expression");
            return {};
    }
}

Value SkRPGenerator::writeStore(const Expression& lhs, const Value& rhs) {
    SkASSERTF(rhs.slots() == lhs.type().slotCount(),
              "lhs=%s (%s)\nrhs=%zu slot",
              lhs.type().description().c_str(), lhs.description().c_str(), rhs.slots());

    // We need to figure out the collection of slots that we're storing into. The l-value (lhs)
    // is always a VariableReference, possibly wrapped by one or more Swizzle, FieldAccess, or
    // IndexExpressions. The underlying VariableReference has a range of slots for its storage,
    // and each expression wrapped around that selects a sub-set of those slots (Field/Index),
    // or rearranges them (Swizzle).
    SkSTArray<4, size_t, true> slots;
    slots.resize(rhs.slots());

    // Start with the identity slot map - this basically says that the values from rhs belong in
    // slots [0, 1, 2 ... N] of the lhs.
    for (size_t i = 0; i < slots.size(); ++i) {
        slots[i] = i;
    }

    // Now, as we peel off each outer expression, adjust 'slots' to be the locations relative to
    // the next (inner) expression:
    const Expression* expr = &lhs;
    while (!expr->is<VariableReference>()) {
        switch (expr->kind()) {
            case Expression::Kind::kFieldAccess: {
                const FieldAccess& fld = expr->as<FieldAccess>();
                size_t offset = this->fieldSlotOffset(fld);
                for (size_t& s : slots) {
                    s += offset;
                }
                expr = fld.base().get();
            } break;
            case Expression::Kind::kIndex: {
                const IndexExpression& idx = expr->as<IndexExpression>();
                size_t offset = this->indexSlotOffset(idx);
                for (size_t& s : slots) {
                    s += offset;
                }
                expr = idx.base().get();
            } break;
            case Expression::Kind::kSwizzle: {
                const Swizzle& swz = expr->as<Swizzle>();
                for (size_t& s : slots) {
                    s = swz.components()[s];
                }
                expr = swz.base().get();
            } break;
            default:
                // No other kinds of expressions are valid in lvalues. (see Analysis::IsAssignable)
                SkDEBUGFAIL("Invalid expression type");
                return {};
        }
    }

    // When we get here, 'slots' are all relative to the first slot holding 'var's storage
    const Variable& var = *expr->as<VariableReference>().variable();
    size_t varSlot = this->getSlot(var);
    for (size_t& slot : slots) {
        SkASSERT(slot < var.type().slotCount());
        slot += varSlot;
    }

    // `slots` are now absolute indices into `fSlots`.
    skvm::I32 mask = this->mask();
    for (size_t i = 0; i < rhs.slots(); ++i) {
        int slotNum = slots[i];
        skvm::Val conditionalStore = this->writeConditionalStore(fSlots[slotNum].val, rhs[i], mask);
        this->writeToSlot(slotNum, conditionalStore);
    }

    return rhs;
}

skvm::Val SkRPGenerator::writeConditionalStore(skvm::Val lhs, skvm::Val rhs, skvm::I32 mask) {
    return select(mask, f32(rhs), f32(lhs)).id;
}

void SkRPGenerator::writeBlock(const Block& b) {
    StackScope scope(this);  // TODO: Only if isScope?
    for (const std::unique_ptr<Statement>& stmt : b.children()) {
        this->writeStatement(*stmt);
    }
}

void SkRPGenerator::writeBreakStatement() {
    // Any active lanes stop executing for the duration of the current loop
    fLoopMask &= ~this->mask();
}

void SkRPGenerator::writeContinueStatement() {
    // Any active lanes stop executing for the current iteration.
    // Remember them in fContinueMask, to be re-enabled later.
    skvm::I32 mask = this->mask();
    fLoopMask &= ~mask;
    fContinueMask |= mask;
}

void SkRPGenerator::writeForStatement(const ForStatement& f) {
    // We require that all loops be ES2-compliant (unrollable), and actually unroll them here
    SkASSERT(f.unrollInfo());
    const LoopUnrollInfo& loop = *f.unrollInfo();
    SkASSERT(loop.fIndex->type().slotCount() == 1);

    size_t indexSlot = this->getSlot(*loop.fIndex);
    double val = loop.fStart;

    const skvm::I32 zero      = fBuilder->splat(0);
    skvm::I32 oldLoopMask     = fLoopMask,
              oldContinueMask = fContinueMask;

    const Type::NumberKind indexKind = base_number_kind(loop.fIndex->type());

    // We want the loop index to disappear at the end of the loop, so wrap the for statement in a
    // trace scope.
    if (loop.fCount > 0) {
        int line = this->getLine(f.test() ? f.test()->fPosition : f.fPosition);
        skvm::I32 mask = this->mask();

        for (int i = 0; i < loop.fCount; ++i) {
            this->writeToSlot(indexSlot, (indexKind == Type::NumberKind::kFloat)
                                            ? fBuilder->splat(static_cast<float>(val)).id
                                            : fBuilder->splat(static_cast<int>(val)).id);

            fContinueMask = zero;
            this->writeStatement(*f.statement());
            fLoopMask |= fContinueMask;

            val += loop.fDelta;
        }
    }

    fLoopMask     = oldLoopMask;
    fContinueMask = oldContinueMask;
}

void SkRPGenerator::writeIfStatement(const IfStatement& i) {
    Value test = this->writeExpression(*i.test());
    {
        ScopedCondition ifTrue(this, i32(test));
        this->writeStatement(*i.ifTrue());
    }
    if (i.ifFalse()) {
        ScopedCondition ifFalse(this, ~i32(test));
        this->writeStatement(*i.ifFalse());
    }
}

void SkRPGenerator::writeReturnStatement(const ReturnStatement& r) {
    skvm::I32 returnsHere = this->mask();

    if (r.expression()) {
        Value val = this->writeExpression(*r.expression());

        size_t slot = currentFunction().fReturnSlot;
        size_t nslots = r.expression()->type().slotCount();
        for (size_t i = 0; i < nslots; ++i) {
            fSlots[slot + i].writtenTo = false;
            skvm::Val conditionalStore = this->writeConditionalStore(fSlots[slot + i].val, val[i],
                                                                     returnsHere);
            this->writeToSlot(slot + i, conditionalStore);
        }
    }

    currentFunction().fReturned |= returnsHere;
}

void SkRPGenerator::writeSwitchStatement(const SwitchStatement& s) {
    skvm::I32 falseValue = fBuilder->splat( 0);
    skvm::I32 trueValue  = fBuilder->splat(~0);

    // Create a "switchFallthough" scratch variable, initialized to false.
    skvm::I32 switchFallthrough = falseValue;

    // Loop masks behave just like for statements. When a break is encountered, it masks off all
    // lanes for the rest of the body of the switch.
    skvm::I32 oldLoopMask       = fLoopMask;
    Value switchValue           = this->writeExpression(*s.value());

    for (const std::unique_ptr<Statement>& stmt : s.cases()) {
        const SwitchCase& c = stmt->as<SwitchCase>();
        if (!c.isDefault()) {
            Value caseValue = fBuilder->splat((int) c.value());

            // We want to execute this switch case if we're falling through from a previous case, or
            // if the case value matches.
            ScopedCondition conditionalCaseBlock(
                    this,
                    switchFallthrough | (i32(caseValue) == i32(switchValue)));
            this->writeStatement(*c.statement());

            // If we are inside the case block, we set the fallthrough flag to true (`break` still
            // works to stop the flow of execution regardless, since it zeroes out the loop-mask).
            switchFallthrough.id = this->writeConditionalStore(switchFallthrough.id, trueValue.id,
                                                               this->mask());
        } else {
            // This is the default case. Since it's always last, we can just dump in the code.
            this->writeStatement(*c.statement());
        }
    }

    // Restore state.
    fLoopMask = oldLoopMask;
}

void SkRPGenerator::writeVarDeclaration(const VarDeclaration& decl) {
    size_t slot   = this->getSlot(decl.var()),
           nslots = decl.var().type().slotCount();

    Value val = decl.value() ? this->writeExpression(*decl.value()) : Value{};
    for (size_t i = 0; i < nslots; ++i) {
        fSlots[slot + i].writtenTo = false;
        this->writeToSlot(slot + i, val ? val[i] : fBuilder->splat(0.0f).id);
    }
}

void SkRPGenerator::writeStatement(const Statement& s) {
    switch (s.kind()) {
        case Statement::Kind::kBlock:
            this->writeBlock(s.as<Block>());
            break;
        case Statement::Kind::kBreak:
            this->writeBreakStatement();
            break;
        case Statement::Kind::kContinue:
            this->writeContinueStatement();
            break;
        case Statement::Kind::kExpression:
            this->writeExpression(*s.as<ExpressionStatement>().expression());
            break;
        case Statement::Kind::kFor:
            this->writeForStatement(s.as<ForStatement>());
            break;
        case Statement::Kind::kIf:
            this->writeIfStatement(s.as<IfStatement>());
            break;
        case Statement::Kind::kReturn:
            this->writeReturnStatement(s.as<ReturnStatement>());
            break;
        case Statement::Kind::kSwitch:
            this->writeSwitchStatement(s.as<SwitchStatement>());
            break;
        case Statement::Kind::kVarDeclaration:
            this->writeVarDeclaration(s.as<VarDeclaration>());
            break;
        case Statement::Kind::kDiscard:
        case Statement::Kind::kDo:
            SkDEBUGFAIL("Unsupported control flow");
            break;
        case Statement::Kind::kNop:
            break;
        default:
            SkDEBUGFAIL("Unrecognized statement");
            break;
    }
}

// Need: map of Variable -> current location (for parameters, this might be register)?
// Nah, just immediately write the parameters to storage, so we have a copy.
// Much simpler than having to remember to write back.

bool AppendStages(const Program& program,
                  const FunctionDefinition& function,
                  const SkStageRec& rec,
                  SkRPCallbacks* callbacks) {
    skvm::Val args[8];  // At most 8 arguments (half4 srcColor, half4 dstColor)
    size_t argSlots = 0;
    for (const SkSL::Variable* param : function.declaration().parameters()) {
        switch (param->modifiers().fLayout.fBuiltin) {
            case SK_MAIN_COORDS_BUILTIN:
                SkASSERT(param->type().slotCount() == 2);
                SkASSERT((argSlots + 2) <= std::size(args));
                args[argSlots++] = local.x.id;
                args[argSlots++] = local.y.id;
                break;
            case SK_INPUT_COLOR_BUILTIN:
                SkASSERT(param->type().slotCount() == 4);
                SkASSERT((argSlots + 4) <= std::size(args));
                args[argSlots++] = inputColor.r.id;
                args[argSlots++] = inputColor.g.id;
                args[argSlots++] = inputColor.b.id;
                args[argSlots++] = inputColor.a.id;
                break;
            case SK_DEST_COLOR_BUILTIN:
                SkASSERT(param->type().slotCount() == 4);
                SkASSERT((argSlots + 4) <= std::size(args));
                args[argSlots++] = destColor.r.id;
                args[argSlots++] = destColor.g.id;
                args[argSlots++] = destColor.b.id;
                args[argSlots++] = destColor.a.id;
                break;
            default:
                SkDEBUGFAIL("Invalid parameter to main()");
                return {};
        }
    }
    SkASSERT(argSlots <= std::size(args));

    SkRPGenerator generator(program, builder, callbacks);
    generator.writeProgram(uniforms, device, function, {args, argSlots}, SkSpan(result));

    return skvm::Color{{builder, result[0]},
                       {builder, result[1]},
                       {builder, result[2]},
                       {builder, result[3]}};
}

}  // namespace SkSL
