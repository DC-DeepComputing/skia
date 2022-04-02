/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLWGSLCodeGenerator.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

// TODO(skia:13092): This is a temporary debug feature. Remove when the implementation is
// complete and this is no longer needed.
#define DUMP_SRC_IR 0

namespace SkSL {
namespace {

// See https://www.w3.org/TR/WGSL/#memory-view-types
enum class PtrAddressSpace {
    kFunction,
    kPrivate,
    kStorage,
};

std::string pipelineStructPrefix(ProgramKind kind) {
    switch (kind) {
        case ProgramKind::kVertex:
            return "VS";
        case ProgramKind::kFragment:
            return "FS";
        default:
            break;
    }
    return "";
}

std::string addressSpaceToString(PtrAddressSpace addressSpace) {
    switch (addressSpace) {
        case PtrAddressSpace::kFunction:
            return "function";
        case PtrAddressSpace::kPrivate:
            return "private";
        case PtrAddressSpace::kStorage:
            return "storage";
    }
    SkDEBUGFAIL("unsupported ptr address space");
    return "unsupported";
}

std::string toScalarType(const Type& type) {
    SkASSERT(type.typeKind() == Type::TypeKind::kScalar);
    switch (type.numberKind()) {
        // Floating-point numbers in WebGPU currently always have 32-bit footprint and
        // relaxed-precision is not supported without extensions. f32 is the only floating-point
        // number type in WGSL (see the discussion on https://github.com/gpuweb/gpuweb/issues/658).
        case Type::NumberKind::kFloat:
            return "f32";
        case Type::NumberKind::kSigned:
            return "i32";
        case Type::NumberKind::kUnsigned:
            return "u32";
        case Type::NumberKind::kBoolean:
            return "bool";
        case Type::NumberKind::kNonnumeric:
            [[fallthrough]];
        default:
            break;
    }
    return std::string(type.name());
}

// Convert a SkSL type to a WGSL type. Handles all plain types except structure types
// (see https://www.w3.org/TR/WGSL/#plain-types-section).
std::string toWGSLType(const Type& type) {
    // TODO(skia:13092): Handle array, matrix, sampler types.
    switch (type.typeKind()) {
        case Type::TypeKind::kScalar:
            return toScalarType(type);
        case Type::TypeKind::kVector:
            return "vec" + std::to_string(type.columns()) + "<" +
                   toScalarType(type.componentType()) + ">";
        default:
            break;
    }
    return std::string(type.name());
}

std::string toPtrType(const Type& type, PtrAddressSpace addressSpace = PtrAddressSpace::kFunction) {
    return "ptr<" + addressSpaceToString(addressSpace) + ", " + toWGSLType(type) + ">";
}

std::string toWGSLBuiltinName(WGSLCodeGenerator::Builtin kind) {
    using Builtin = WGSLCodeGenerator::Builtin;
    switch (kind) {
        case Builtin::kVertexIndex:
            return "vertex_index";
        case Builtin::kInstanceIndex:
            return "instance_index";
        case Builtin::kPosition:
            return "position";
        case Builtin::kFrontFacing:
            return "front_facing";
        case Builtin::kSampleIndex:
            return "sample_index";
        case Builtin::kFragDepth:
            return "frag_depth";
        case Builtin::kSampleMask:
            return "sample_mask";
        case Builtin::kLocalInvocationId:
            return "local_invocation_id";
        case Builtin::kLocalInvocationIndex:
            return "local_invocation_index";
        case Builtin::kGlobalInvocationId:
            return "global_invocation_id";
        case Builtin::kWorkgroupId:
            return "workgroup_id";
        case Builtin::kNumWorkgroups:
            return "num_workgroups";
        default:
            break;
    }

    SkDEBUGFAIL("unsupported builtin");
    return "unsupported";
}

// Map a SkSL builtin flag to a WGSL builtin kind. Returns std::nullopt if `builtin` is not
// not supported for WGSL.
//
// Also see //src/sksl/sksl_vert.sksl and //src/sksl/sksl_frag.sksl for supported built-ins.
std::optional<WGSLCodeGenerator::Builtin> builtinFromSkSLName(int builtin) {
    using Builtin = WGSLCodeGenerator::Builtin;
    switch (builtin) {
        case SK_POSITION_BUILTIN:
            [[fallthrough]];
        case SK_FRAGCOORD_BUILTIN:
            return {Builtin::kPosition};
        case SK_VERTEXID_BUILTIN:
            return {Builtin::kVertexIndex};
        case SK_INSTANCEID_BUILTIN:
            return {Builtin::kInstanceIndex};
        case SK_CLOCKWISE_BUILTIN:
            return {Builtin::kFrontFacing};
        default:
            break;
    }
    return std::nullopt;
}

std::shared_ptr<SymbolTable> topLevelSymbolTable(const FunctionDefinition& f) {
    return f.body()->as<Block>().symbolTable()->fParent;
}

class FunctionDependencyResolver {
public:
    using Deps = WGSLCodeGenerator::FunctionDependencies;
    using DepsMap = SkTHashMap<const FunctionDeclaration*, Deps>;

    // Walk the program tree record the parameter dependencies for every function definition.
    WGSLCodeGenerator::ProgramRequirements resolve(const Program& program) {
        for (const auto* e : program.elements()) {
            if (e->is<FunctionDefinition>()) {
                this->resolve(e->as<FunctionDefinition>().declaration());
            }
        }
        return {
                .dependencies = std::move(fDeps),
                .mainNeedsCoordsArgument = fMainNeedsCoordsArgument,
        };
    }

private:
    Deps resolve(const FunctionDeclaration& f) {
        if (f.isMain()) {
            for (const Variable* v : f.parameters()) {
                if (v->modifiers().fLayout.fBuiltin == SK_MAIN_COORDS_BUILTIN) {
                    fMainNeedsCoordsArgument = true;
                }
            }
        }
        Deps* found = fDeps.find(&f);
        if (found) {
            return *found;
        }

        fDeps.set(&f, Deps::kNone);
        const FunctionDefinition* funcDef = f.definition();
        if (!funcDef) {
            return Deps::kNone;
        }

        Deps deps = this->resolve(funcDef->body().get());
        fDeps.set(&f, deps);
        return deps;
    }

    Deps resolve(const Statement* s) {
        if (!s) {
            return Deps::kNone;
        }
        switch (s->kind()) {
            case Statement::Kind::kBlock: {
                Deps deps = Deps::kNone;
                for (const auto& child : s->as<Block>().children()) {
                    deps |= this->resolve(child.get());
                }
                return deps;
            }
            case Statement::Kind::kVarDeclaration:
                return this->resolve(s->as<VarDeclaration>().value().get());
            case Statement::Kind::kExpression:
                return this->resolve(s->as<ExpressionStatement>().expression().get());
            case Statement::Kind::kReturn:
                return this->resolve(s->as<ReturnStatement>().expression().get());
            case Statement::Kind::kIf: {
                const IfStatement& i = s->as<IfStatement>();
                return this->resolve(i.test().get()) | this->resolve(i.ifTrue().get()) |
                       this->resolve(i.ifFalse().get());
            }
            case Statement::Kind::kFor: {
                const ForStatement& f = s->as<ForStatement>();
                return this->resolve(f.initializer().get()) | this->resolve(f.test().get()) |
                       this->resolve(f.next().get()) | this->resolve(f.statement().get());
            }
            case Statement::Kind::kDo: {
                const DoStatement& d = s->as<DoStatement>();
                return this->resolve(d.test().get()) | this->resolve(d.statement().get());
            }
            case Statement::Kind::kSwitch: {
                const SwitchStatement& sw = s->as<SwitchStatement>();
                Deps deps = this->resolve(sw.value().get());
                for (const auto& sc : sw.cases()) {
                    deps |= this->resolve(sc->as<SwitchCase>().statement().get());
                }
                return deps;
            }
            default:
                break;
        }
        return Deps::kNone;
    }

    Deps resolve(const Expression* e) {
        if (!e) {
            return Deps::kNone;
        }
        switch (e->kind()) {
            case Expression::Kind::kFunctionCall: {
                const FunctionCall& f = e->as<FunctionCall>();
                Deps deps = this->resolve(f.function());
                for (const auto& arg : f.arguments()) {
                    deps |= this->resolve(arg.get());
                }
                return deps;
            }
            case Expression::Kind::kConstructorCompound:
            case Expression::Kind::kConstructorCompoundCast:
            case Expression::Kind::kConstructorArray:
            case Expression::Kind::kConstructorArrayCast:
            case Expression::Kind::kConstructorDiagonalMatrix:
            case Expression::Kind::kConstructorScalarCast:
            case Expression::Kind::kConstructorSplat:
            case Expression::Kind::kConstructorStruct: {
                const AnyConstructor& c = e->asAnyConstructor();
                Deps deps = Deps::kNone;
                for (const auto& arg : c.argumentSpan()) {
                    deps |= this->resolve(arg.get());
                }
                return deps;
            }
            case Expression::Kind::kFieldAccess:
                return this->resolve(e->as<FieldAccess>().base().get());
            case Expression::Kind::kSwizzle:
                return this->resolve(e->as<Swizzle>().base().get());
            case Expression::Kind::kBinary: {
                const BinaryExpression& b = e->as<BinaryExpression>();
                return this->resolve(b.left().get()) | this->resolve(b.right().get());
            }
            case Expression::Kind::kIndex: {
                const IndexExpression& i = e->as<IndexExpression>();
                return this->resolve(i.base().get()) | this->resolve(i.index().get());
            }
            case Expression::Kind::kPrefix:
                return this->resolve(e->as<PrefixExpression>().operand().get());
            case Expression::Kind::kPostfix:
                return this->resolve(e->as<PostfixExpression>().operand().get());
            case Expression::Kind::kTernary: {
                const TernaryExpression& t = e->as<TernaryExpression>();
                return this->resolve(t.test().get()) | this->resolve(t.ifTrue().get()) |
                       this->resolve(t.ifFalse().get());
            }
            case Expression::Kind::kVariableReference: {
                const VariableReference& v = e->as<VariableReference>();
                const Modifiers& modifiers = v.variable()->modifiers();
                if (v.variable()->storage() == Variable::Storage::kGlobal) {
                    if (modifiers.fFlags & Modifiers::kIn_Flag) {
                        return Deps::kPipelineInputs;
                    }
                    if (modifiers.fFlags & Modifiers::kOut_Flag) {
                        return Deps::kPipelineOutputs;
                    }
                }
                return Deps::kNone;
            }
            default:
                break;
        }
        return Deps::kNone;
    }

    DepsMap fDeps;
    bool fMainNeedsCoordsArgument = false;
};

}  // namespace

bool WGSLCodeGenerator::generateCode() {
    // The resources of a WGSL program are structured in the following way:
    // - Vertex and fragment stage attribute inputs and outputs are bundled
    //   inside synthetic structs called VSIn/VSOut/FSIn/FSOut.
    // - All uniform and storage type resources are declared in global scope.
    this->preprocessProgram();

    StringStream header;
    {
        AutoOutputStream outputToHeader(this, &header, &fIndentation);
        // TODO(skia:13092): Implement the following:
        // - struct definitions
        // - global uniform/storage resource declarations, including interface blocks.
        this->writeStageInputStruct();
        this->writeStageOutputStruct();
    }
    StringStream body;
    {
        AutoOutputStream outputToBody(this, &body, &fIndentation);
        for (const ProgramElement* e : fProgram.elements()) {
            this->writeProgramElement(*e);
        }

// TODO(skia:13092): This is a temporary debug feature. Remove when the implementation is
// complete and this is no longer needed.
#if DUMP_SRC_IR
        this->writeLine("\n----------");
        this->writeLine("Source IR:\n");
        for (const ProgramElement* e : fProgram.elements()) {
            this->writeLine(e->description().c_str());
        }
#endif
    }

    write_stringstream(header, *fOut);
    write_stringstream(body, *fOut);
    fContext.fErrors->reportPendingErrors(Position());
    return fContext.fErrors->errorCount() == 0;
}

void WGSLCodeGenerator::preprocessProgram() {
    FunctionDependencyResolver resolver;
    fRequirements = resolver.resolve(fProgram);
}

void WGSLCodeGenerator::write(std::string_view s) {
    if (s.empty()) {
        return;
    }
    if (fAtLineStart) {
        for (int i = 0; i < fIndentation; i++) {
            fOut->writeText("    ");
        }
    }
    fOut->writeText(std::string(s).c_str());
    fAtLineStart = false;
}

void WGSLCodeGenerator::writeLine(std::string_view s) {
    this->write(s);
    fOut->writeText("\n");
    fAtLineStart = true;
}

void WGSLCodeGenerator::finishLine() {
    if (!fAtLineStart) {
        this->writeLine();
    }
}

void WGSLCodeGenerator::writeName(std::string_view name) {
    // Add underscore before name to avoid conflict with reserved words.
    if (fReservedWords.contains(name)) {
        this->write("_");
    }
    this->write(name);
}

void WGSLCodeGenerator::writeUserDefinedVariableDecl(const Type& type,
                                                     std::string_view name,
                                                     int location) {
    this->write("@location(" + std::to_string(location) + ") ");
    this->write(name);
    this->write(": " + toWGSLType(type));
    this->writeLine(";");
}

void WGSLCodeGenerator::writeBuiltinVariableDecl(const Type& type,
                                                 std::string_view name,
                                                 Builtin kind) {
    this->write("@builtin(" + toWGSLBuiltinName(kind) + ") ");
    this->write(name);
    this->write(": " + toWGSLType(type));
    this->writeLine(";");
}

void WGSLCodeGenerator::writeFunction(const FunctionDefinition& f) {
    this->writeFunctionDeclaration(f.declaration());
    this->write(" ");
    this->writeBlock(f.body()->as<Block>());

    if (f.declaration().isMain()) {
        // We just emitted the user-defined main function. We next generate a program entry point
        // that calls the user-defined main.
        this->writeEntryPoint(f);
    }
}

void WGSLCodeGenerator::writeFunctionDeclaration(const FunctionDeclaration& f) {
    this->write("fn ");
    this->write(f.mangledName());
    this->write("(");
    const char* separator = "";
    FunctionDependencies* deps = fRequirements.dependencies.find(&f);
    if (deps) {
        std::string structNamePrefix = pipelineStructPrefix(fProgram.fConfig->fKind);
        if (structNamePrefix.length() != 0) {
            if ((*deps & FunctionDependencies::kPipelineInputs) != FunctionDependencies::kNone) {
                separator = ", ";
                this->write("_stageIn: ");
                this->write(structNamePrefix);
                this->write("In");
            }
            if ((*deps & FunctionDependencies::kPipelineOutputs) != FunctionDependencies::kNone) {
                this->write(separator);
                separator = ", ";
                this->write("_stageOut: ptr<function, ");
                this->write(structNamePrefix);
                this->write("Out>");
            }
        }
    }
    for (const auto& param : f.parameters()) {
        this->write(separator);
        separator = ", ";
        this->writeName(param->name());
        this->write(": ");

        // Declare an "out" function parameter as a pointer.
        if (param->modifiers().fFlags & Modifiers::kOut_Flag) {
            this->write(toPtrType(param->type()));
        } else {
            this->write(toWGSLType(param->type()));
        }
    }
    this->write(")");
    if (f.returnType().typeKind() != Type::TypeKind::kVoid) {
        this->write(" -> ");
        this->write(toWGSLType(f.returnType()));
    }
}

void WGSLCodeGenerator::writeEntryPoint(const FunctionDefinition& main) {
    SkASSERT(main.declaration().isMain());

    // The input and output parameters for a vertex/fragment stage entry point function have the
    // FSIn/FSOut/VSIn/VSOut struct types that have been synthesized in generateCode(). An entry
    // point always has the same signature and acts as a trampoline to the user-defined main
    // function.
    std::string outputType;
    if (fProgram.fConfig->fKind == ProgramKind::kVertex) {
        this->writeLine("@stage(vertex) fn vertexMain(_stageIn: VSIn) -> VSOut {");
        outputType = "VSOut";
    } else if (fProgram.fConfig->fKind == ProgramKind::kFragment) {
        this->writeLine("@stage(fragment) fn fragmentMain(_stageIn: FSIn) -> FSOut {");
        outputType = "FSOut";
    } else {
        fContext.fErrors->error(Position(), "program kind not supported");
        return;
    }

    // Declare the stage output struct.
    fIndentation++;
    this->write("var _stageOut: ");
    this->write(outputType);
    this->writeLine(";");

    // Generate assignment to sk_FragColor built-in if the user-defined main returns a color.
    if (fProgram.fConfig->fKind == ProgramKind::kFragment) {
        auto symbolTable = topLevelSymbolTable(main);
        const Symbol* symbol = (*symbolTable)["sk_FragColor"];
        SkASSERT(symbol);
        if (main.declaration().returnType().matches(symbol->type())) {
            this->write("_stageOut.sk_FragColor = ");
        }
    }

    // Generate the function call to the user-defined main:
    this->write(main.declaration().mangledName());
    this->write("(");
    const char* separator = "";
    FunctionDependencies* deps = fRequirements.dependencies.find(&main.declaration());
    if (deps) {
        if ((*deps & FunctionDependencies::kPipelineInputs) != FunctionDependencies::kNone) {
            separator = ", ";
            this->write("_stageIn");
        }
        if ((*deps & FunctionDependencies::kPipelineOutputs) != FunctionDependencies::kNone) {
            this->write(separator);
            separator = ", ";
            this->write("&_stageOut");
        }
    }
    // TODO(armansito): Handle arbitrary parameters.
    if (main.declaration().parameters().size() != 0) {
        const Variable* v = main.declaration().parameters()[0];
        const Type& type = v->type();
        if (v->modifiers().fLayout.fBuiltin == SK_MAIN_COORDS_BUILTIN) {
            if (!type.matches(*fContext.fTypes.fFloat2)) {
                fContext.fErrors->error(
                        main.fPosition,
                        "main function has unsupported parameter: " + type.description());
                return;
            }

            this->write(separator);
            separator = ", ";
            this->write("_stageIn.sk_FragCoord.xy");
        }
    }
    this->writeLine(");");
    this->writeLine("return _stageOut;");

    fIndentation--;
    this->writeLine("}");
}

void WGSLCodeGenerator::writeStatement(const Statement& s) {
    switch (s.kind()) {
        case Statement::Kind::kBlock:
            this->writeBlock(s.as<Block>());
            break;
        case Statement::Kind::kExpression:
            this->writeExpressionStatement(s.as<ExpressionStatement>());
            break;
        case Statement::Kind::kReturn:
            this->writeReturnStatement(s.as<ReturnStatement>());
            break;
        case Statement::Kind::kVarDeclaration:
            this->writeVarDeclaration(s.as<VarDeclaration>());
            break;
        default:
            SkDEBUGFAILF("unsupported statement (kind: %d) %s", s.kind(), s.description().c_str());
            break;
    }
}

void WGSLCodeGenerator::writeStatements(const StatementArray& statements) {
    for (const auto& s : statements) {
        if (!s->isEmpty()) {
            this->writeStatement(*s);
            this->finishLine();
        }
    }
}

void WGSLCodeGenerator::writeBlock(const Block& b) {
    // Write scope markers if this block is a scope, or if the block is empty (since we need to emit
    // something here to make the code valid).
    bool isScope = b.isScope() || b.isEmpty();
    if (isScope) {
        this->writeLine("{");
        fIndentation++;
    }
    this->writeStatements(b.children());
    if (isScope) {
        fIndentation--;
        this->writeLine("}");
    }
}

void WGSLCodeGenerator::writeExpressionStatement(const ExpressionStatement& s) {
    if (s.expression()->hasSideEffects()) {
        this->writeExpression(*s.expression(), Precedence::kTopLevel);
        this->write(";");
    }
}

void WGSLCodeGenerator::writeReturnStatement(const ReturnStatement& s) {
    this->write("return");
    if (s.expression()) {
        this->write(" ");
        this->writeExpression(*s.expression(), Precedence::kTopLevel);
    }
    this->write(";");
}

void WGSLCodeGenerator::writeVarDeclaration(const VarDeclaration& varDecl) {
    bool isConst = varDecl.var().modifiers().fFlags & Modifiers::kConst_Flag;
    if (isConst) {
        this->write("let ");
    } else {
        this->write("var ");
    }
    this->writeName(varDecl.var().name());
    this->write(": ");
    this->write(toWGSLType(varDecl.var().type()));

    if (varDecl.value()) {
        this->write(" = ");
        this->writeExpression(*varDecl.value(), Precedence::kTopLevel);
    } else if (isConst) {
        fContext.fErrors->error(varDecl.fPosition, "A let-declared constant must specify a value");
    }

    this->write(";");
}

void WGSLCodeGenerator::writeExpression(const Expression& e, Precedence parentPrecedence) {
    switch (e.kind()) {
        case Expression::Kind::kBinary:
            this->writeBinaryExpression(e.as<BinaryExpression>(), parentPrecedence);
            break;
        case Expression::Kind::kConstructorCompound:
            this->writeConstructorCompound(e.as<ConstructorCompound>(), parentPrecedence);
            break;
        case Expression::Kind::kLiteral:
            this->writeLiteral(e.as<Literal>());
            break;
        default:
            SkDEBUGFAILF("unsupported expression (kind: %d) %s", e.kind(), e.description().c_str());
            break;
    }
}

void WGSLCodeGenerator::writeBinaryExpression(const BinaryExpression& b,
                                              Precedence parentPrecedence) {
    // TODO(skia:13092): implement
}

void WGSLCodeGenerator::writeLiteral(const Literal& l) {
    const Type& type = l.type();
    if (type.isFloat()) {
        this->write(skstd::to_string(l.floatValue()));
        return;
    }
    if (type.isBoolean()) {
        this->write(l.boolValue() ? "true" : "false");
        return;
    }
    SkASSERT(type.isInteger());
    if (type.matches(*fContext.fTypes.fUInt)) {
        this->write(std::to_string(l.intValue() & 0xffffffff));
        this->write("u");
    } else if (type.matches(*fContext.fTypes.fUShort)) {
        this->write(std::to_string(l.intValue() & 0xffff));
        this->write("u");
    } else {
        this->write(std::to_string(l.intValue()));
    }
}

void WGSLCodeGenerator::writeAnyConstructor(const AnyConstructor& c, Precedence parentPrecedence) {
    this->write(toWGSLType(c.type()));
    this->write("(");
    const char* separator = "";
    for (const auto& e : c.argumentSpan()) {
        this->write(separator);
        separator = ", ";
        this->writeExpression(*e, Precedence::kSequence);
    }
    this->write(")");
}

void WGSLCodeGenerator::writeConstructorCompound(const ConstructorCompound& c,
                                                 Precedence parentPrecedence) {
    // TODO(skia:13092): Support matrix constructors
    if (c.type().isVector()) {
        this->writeConstructorCompoundVector(c, parentPrecedence);
    } else {
        fContext.fErrors->error(c.fPosition, "unsupported compound constructor");
    }
}

void WGSLCodeGenerator::writeConstructorCompoundVector(const ConstructorCompound& c,
                                                       Precedence parentPrecedence) {
    // TODO(skia:13092): WGSL supports constructing vectors from a mix of scalars and vectors but
    // not matrices. SkSL supports vec4(mat2x2) which we need to handle here
    // (see https://www.w3.org/TR/WGSL/#type-constructor-expr).
    this->writeAnyConstructor(c, parentPrecedence);
}

void WGSLCodeGenerator::writeProgramElement(const ProgramElement& e) {
    switch (e.kind()) {
        case ProgramElement::Kind::kExtension:
            // TODO(skia:13092): WGSL supports extensions via the "enable" directive
            // (https://www.w3.org/TR/WGSL/#language-extensions). While we could easily emit this
            // directive, we should first ensure that all possible SkSL extension names are
            // converted to their appropriate WGSL extension. Currently there are no known supported
            // WGSL extensions aside from the hypotheticals listed in the spec.
            break;
        case ProgramElement::Kind::kGlobalVar:
            // All global declarations are handled explicitly as the "program header" in
            // generateCode().
            break;
        case ProgramElement::Kind::kInterfaceBlock:
            // All interface block declarations are handled explicitly as the "program header" in
            // generateCode().
            break;
        case ProgramElement::Kind::kStructDefinition:
            // All struct type declarations are handled explicitly as the "program header" in
            // generateCode().
            break;
        case ProgramElement::Kind::kFunctionPrototype:
            // A WGSL function declaration must contain its body and the function name is in scope
            // for the entire program (see https://www.w3.org/TR/WGSL/#function-declaration and
            // https://www.w3.org/TR/WGSL/#declaration-and-scope).
            //
            // As such, we don't emit function prototypes.
            break;
        case ProgramElement::Kind::kFunction:
            this->writeFunction(e.as<FunctionDefinition>());
            break;
        default:
            SkDEBUGFAILF("unsupported program element: %s\n", e.description().c_str());
            break;
    }
}

void WGSLCodeGenerator::writeStageInputStruct() {
    std::string structNamePrefix = pipelineStructPrefix(fProgram.fConfig->fKind);
    if (structNamePrefix.length() == 0) {
        // There's no need to declare pipeline stage outputs.
        return;
    }

    this->write("struct ");
    this->write(structNamePrefix);
    this->writeLine("In {");
    fIndentation++;

    bool declaredFragCoordsBuiltin = false;
    auto declareInVar = [this, &declaredFragCoordsBuiltin](
                                Modifiers modifiers, const Type& type, std::string_view name) {
        // If a `layout(location=...)` has been specified, then treat this as a user-defined
        // variable, even if it's a built-in.
        int location = modifiers.fLayout.fLocation;
        if (location >= 0) {
            this->writeUserDefinedVariableDecl(type, name, location);
        } else if (modifiers.fLayout.fBuiltin >= 0) {
            auto builtin = builtinFromSkSLName(modifiers.fLayout.fBuiltin);
            if (builtin.has_value()) {
                this->writeBuiltinVariableDecl(type, name, *builtin);
                if (modifiers.fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN) {
                    declaredFragCoordsBuiltin = true;
                }
            }
        }
    };

    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const Variable& v =
                    e->as<GlobalVarDeclaration>().declaration()->as<VarDeclaration>().var();
            if (v.modifiers().fFlags & Modifiers::kIn_Flag) {
                declareInVar(v.modifiers(), v.type(), v.name());
            }
        } else if (e->is<InterfaceBlock>()) {
            const Variable& v = e->as<InterfaceBlock>().variable();
            // Merge all the members of `out` interface blocks to the output struct, which are
            // specified as either "builtin" or with a "layout(location=".
            //
            // TODO(armansito): Is it legal to have an interface block without a storage qualifier
            // but with members that have individual storage qualifiers?
            if (v.modifiers().fFlags & Modifiers::kIn_Flag) {
                for (const auto& f : v.type().fields()) {
                    declareInVar(f.fModifiers, *f.fType, f.fName);
                }
            }
        }
    }

    if (fProgram.fConfig->fKind == ProgramKind::kFragment &&
        fRequirements.mainNeedsCoordsArgument && !declaredFragCoordsBuiltin) {
        this->writeLine("@builtin(position) sk_FragCoord: vec4<f32>;");
    }

    fIndentation--;
    this->writeLine("};");
}

void WGSLCodeGenerator::writeStageOutputStruct() {
    std::string structNamePrefix;
    if (fProgram.fConfig->fKind == ProgramKind::kVertex) {
        structNamePrefix = "VS";
    } else if (fProgram.fConfig->fKind == ProgramKind::kFragment) {
        structNamePrefix = "FS";
    } else {
        // There's no need to declare pipeline stage outputs.
        return;
    }

    this->write("struct ");
    this->write(structNamePrefix);
    this->writeLine("Out {");
    fIndentation++;

    auto declareOutVar = [this](Modifiers modifiers, const Type& type, std::string_view name) {
        // If a `layout(location=...)` has been specified, then treat this as a user-defined
        // variable, even if it's a built-in.
        //
        // For example, sk_FragColor is a built-in that specifies a location. In WGSL there is no
        // built-in variable for a fragment stage color output as it relies on the user-defined
        // location of a render target.
        int location = modifiers.fLayout.fLocation;
        if (location >= 0) {
            this->writeUserDefinedVariableDecl(type, name, location);
        } else if (modifiers.fLayout.fBuiltin >= 0) {
            auto builtin = builtinFromSkSLName(modifiers.fLayout.fBuiltin);
            if (builtin.has_value()) {
                this->writeBuiltinVariableDecl(type, name, *builtin);
            }
        }
    };

    // TODO(skia:13092): Remember all variables that are added to the output struct here so they
    // can be referenced correctly when handling variable references.
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            const Variable& v =
                    e->as<GlobalVarDeclaration>().declaration()->as<VarDeclaration>().var();
            if (v.modifiers().fFlags & Modifiers::kOut_Flag) {
                declareOutVar(v.modifiers(), v.type(), v.name());
            }
        } else if (e->is<InterfaceBlock>()) {
            const Variable& v = e->as<InterfaceBlock>().variable();
            // Merge all the members of `out` interface blocks to the output struct, which are
            // specified as either "builtin" or with a "layout(location=".
            //
            // TODO(armansito): Is it legal to have an interface block without a storage qualifier
            // but with members that have individual storage qualifiers?
            if (v.modifiers().fFlags & Modifiers::kOut_Flag) {
                for (const auto& f : v.type().fields()) {
                    declareOutVar(f.fModifiers, *f.fType, f.fName);
                }
            }
        }
    }

    fIndentation--;
    this->writeLine("};");
}

}  // namespace SkSL
