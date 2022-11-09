/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLLayout.h"
#include "include/private/SkSLModifiers.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <cstddef>
#include <vector>

namespace SkSL {
namespace {

// A single scalar in our program consumes one slot.
using Slot = int;

// Scalars, vectors, and matrices can be represented as a range of slot indices.
struct SlotRange {
    Slot index = 0;
    int count = 0;
};

// Represents a single raster-pipeline SkSL instruction.
struct RPInstruction {
    RPInstruction(SkRasterPipeline::Stage op, int a)               : fOp(op), rA(a)               {}
    RPInstruction(SkRasterPipeline::Stage op, int a, int b)        : fOp(op), rA(a), rB(b)        {}
    RPInstruction(SkRasterPipeline::Stage op, int a, int b, int c) : fOp(op), rA(a), rB(b), rC(c) {}

    SkRasterPipeline::Stage fOp;
    int32_t                 rA, rB, rC;
};

class SkRPBuilder {
public:
    void finish(const SkStageRec& rp) {
        // TODO(johnstiles): optimize(?), then emit stages from fProgram into rp
    }

    void store_src_rg(SlotRange slots) {
        SkASSERT(slots.count == 2);
        fProgram.push_back({SkRasterPipeline::store_src_rg, slots.index});
    }

    void store_src(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fProgram.push_back({SkRasterPipeline::store_src, slots.index});
    }

    void store_dst(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fProgram.push_back({SkRasterPipeline::store_dst, slots.index});
    }

    void load_src(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fProgram.push_back({SkRasterPipeline::load_src, slots.index});
    }

    void load_dst(SlotRange slots) {
        SkASSERT(slots.count == 4);
        fProgram.push_back({SkRasterPipeline::load_dst, slots.index});
    }

    SkTArray<RPInstruction> fProgram;
};

class SkRPGenerator {
public:
    SkRPGenerator(const Program& program, SkRasterPipelineCallbacks* callbacks)
            : fProgram(program), fCallbacks(callbacks) {}

    /** Converts the SkSL main() function into a list of RPInstructions. */
    void writeProgram(const FunctionDefinition& function);

    /** Converts an SkSL function into a list of RPInstructions. */
    SlotRange writeFunction(const FunctionDefinition& function);

    /** Creates scratch slots, unassociated with any particular SkSL. */
    SlotRange createSlots(int numSlots);

    /** Creates slots associated with an SkSL variable or return value. */
    SlotRange createSlots(const std::string& name,
                          const Type& type,
                          Position pos,
                          bool isFunctionReturnValue);

    /** Looks up the slots associated with an SkSL variable; creates the slot if necessary. */
    SlotRange getSlots(const Variable& v);

    /**
     * Looks up the slots associated with an SkSL function's return value; creates the slot if
     * necessary. Note that recursion is never supported, so we don't need to maintain return values
     * in a stack; we can just statically allocate one slot per function.
     */
    SlotRange getSlots(const FunctionDeclaration& f);

    /** Allows appending instructions to the RP program. */
    SkRPBuilder* builder() { return &fBuilder; }

private:
    [[maybe_unused]] const Program& fProgram;
    [[maybe_unused]] SkRasterPipelineCallbacks* fCallbacks;

    SkRPBuilder fBuilder;

    SkTHashMap<const IRNode*, SlotRange> fSlotMap;
    int fSlotCount;
};

SlotRange SkRPGenerator::createSlots(int numSlots) {
    SlotRange range = {fSlotCount, numSlots};
    fSlotCount += numSlots;
    return range;
}

SlotRange SkRPGenerator::createSlots(const std::string& name,
                                     const Type& type,
                                     Position pos,
                                     bool isFunctionReturnValue) {
    // TODO: `name`, `pos` and `isFunctionReturnValue` will be used by the debugger.
    // For now, ignore these and just create the raw slots.
    return this->createSlots(type.slotCount());
}

SlotRange SkRPGenerator::getSlots(const Variable& v) {
    SlotRange* entry = fSlotMap.find(&v);
    if (entry != nullptr) {
        return *entry;
    }
    SlotRange range = this->createSlots(std::string(v.name()),
                                        v.type(),
                                        v.fPosition,
                                        /*isFunctionReturnValue=*/false);
    fSlotMap.set(&v, range);
    return range;
}

SlotRange SkRPGenerator::getSlots(const FunctionDeclaration& f) {
    SlotRange* entry = fSlotMap.find(&f);
    if (entry != nullptr) {
        return *entry;
    }
    SlotRange range = this->createSlots(std::string(f.name()),
                                        f.returnType(),
                                        f.definition()->fPosition,
                                        /*isFunctionReturnValue=*/true);
    fSlotMap.set(&f, range);
    return range;
}

SlotRange SkRPGenerator::writeFunction(const FunctionDefinition& function) {
    // TODO(johnstiles): implement a code generator
    return this->getSlots(function.declaration());
}

void SkRPGenerator::writeProgram(const FunctionDefinition& function) {
    // Assign slots to the parameters of main; copy src and dst into those slots as appropriate.
    for (const SkSL::Variable* param : function.declaration().parameters()) {
        switch (param->modifiers().fLayout.fBuiltin) {
            case SK_MAIN_COORDS_BUILTIN: {
                // Coordinates are passed via RG.
                SlotRange fragCoord = this->getSlots(*param);
                SkASSERT(fragCoord.count == 2);
                this->builder()->store_src_rg(fragCoord);
                break;
            }
            case SK_INPUT_COLOR_BUILTIN: {
                // Input colors are passed via RGBA.
                SlotRange srcColor = this->getSlots(*param);
                SkASSERT(srcColor.count == 4);
                this->builder()->store_src(srcColor);
                break;
            }
            case SK_DEST_COLOR_BUILTIN: {
                // Dest colors are passed via dRGBA.
                SlotRange destColor = this->getSlots(*param);
                SkASSERT(destColor.count == 4);
                this->builder()->store_dst(destColor);
                break;
            }
            default: {
                SkDEBUGFAIL("Invalid parameter to main()");
                return;
            }
        }
    }

    // Invoke main().
    SlotRange mainResult = this->writeFunction(function);

    // Write the result of main() to RGBA. Allow dRGBA to be trashed.
    SkASSERT(mainResult.count == 4);
    this->builder()->store_src(mainResult);
}

}  // namespace

bool AppendStages(const Program& program,
                  const FunctionDefinition& function,
                  const SkStageRec& rec,
                  SkRasterPipelineCallbacks* callbacks) {
    // TODO(johnstiles): uniforms
    SkRPGenerator generator(program, callbacks);
    generator.writeProgram(function);

    generator.builder()->finish(rec);

    return false;
}

}  // namespace SkSL
