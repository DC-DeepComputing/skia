/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLLayout.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "include/sksl/SkSLPosition.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <cstdint>
#include <string>
#include <vector>

namespace SkSL {

class IRNode;

namespace RP {

class Generator {
public:
    Generator(const Program& program, SkRasterPipelineCallbacks* callbacks)
            : fProgram(program), fCallbacks(callbacks) {}

    /** Converts the SkSL main() function into a list of RPInstructions. */
    void writeProgram(const FunctionDefinition& function);

    /** Converts an SkSL function into a list of RPInstructions. */
    SlotRange writeFunction(const FunctionDefinition& function);

    /** Creates scratch slots, unassociated with any particular SkSL. */
    SlotRange createSlots(int numSlots);

    /** Creates slots associated with an SkSL variable or return value. */
    SlotRange createSlots(std::string_view name,
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

    /** The Builder stitches our instructions together into Raster Pipeline code. */
    Builder* builder() { return &fBuilder; }

private:
    [[maybe_unused]] const Program& fProgram;
    [[maybe_unused]] SkRasterPipelineCallbacks* fCallbacks;

    Builder fBuilder;

    SkTHashMap<const IRNode*, SlotRange> fSlotMap;
    int fSlotCount;
};

SlotRange Generator::createSlots(int numSlots) {
    SlotRange range = {fSlotCount, numSlots};
    fSlotCount += numSlots;
    return range;
}

SlotRange Generator::createSlots(std::string_view name,
                                 const Type& type,
                                 Position pos,
                                 bool isFunctionReturnValue) {
    // TODO: `name`, `pos` and `isFunctionReturnValue` will be used by the debugger.
    // For now, ignore these and just create the raw slots.
    return this->createSlots(type.slotCount());
}

SlotRange Generator::getSlots(const Variable& v) {
    SlotRange* entry = fSlotMap.find(&v);
    if (entry != nullptr) {
        return *entry;
    }
    SlotRange range = this->createSlots(v.name(),
                                        v.type(),
                                        v.fPosition,
                                        /*isFunctionReturnValue=*/false);
    fSlotMap.set(&v, range);
    return range;
}

SlotRange Generator::getSlots(const FunctionDeclaration& f) {
    SlotRange* entry = fSlotMap.find(&f);
    if (entry != nullptr) {
        return *entry;
    }
    SlotRange range = this->createSlots(f.name(),
                                        f.returnType(),
                                        f.definition()->fPosition,
                                        /*isFunctionReturnValue=*/true);
    fSlotMap.set(&f, range);
    return range;
}

SlotRange Generator::writeFunction(const FunctionDefinition& function) {
    // TODO(johnstiles): implement a code generator
    return this->getSlots(function.declaration());
}

void Generator::writeProgram(const FunctionDefinition& function) {
    // Assign slots to the parameters of main; copy src and dst into those slots as appropriate.
    for (const SkSL::Variable* param : function.declaration().parameters()) {
        switch (param->modifiers().fLayout.fBuiltin) {
            case SK_MAIN_COORDS_BUILTIN: {
                // Coordinates are passed via RG.
                SlotRange fragCoord = this->getSlots(*param);
                SkASSERT(fragCoord.count == 2);
                fBuilder.store_src_rg(fragCoord);
                break;
            }
            case SK_INPUT_COLOR_BUILTIN: {
                // Input colors are passed via RGBA.
                SlotRange srcColor = this->getSlots(*param);
                SkASSERT(srcColor.count == 4);
                fBuilder.store_src(srcColor);
                break;
            }
            case SK_DEST_COLOR_BUILTIN: {
                // Dest colors are passed via dRGBA.
                SlotRange destColor = this->getSlots(*param);
                SkASSERT(destColor.count == 4);
                fBuilder.store_dst(destColor);
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

    // Write the result of main() to RGBA. Allow dRGBA to remain in a trashed state.
    SkASSERT(mainResult.count == 4);
    fBuilder.store_src(mainResult);
}

}  // namespace RP

bool AppendStages(const Program& program,
                  const FunctionDefinition& function,
                  const SkStageRec& rec,
                  SkRasterPipelineCallbacks* callbacks) {
    // TODO(johnstiles): uniforms
    RP::Generator generator(program, callbacks);
    generator.writeProgram(function);
    generator.builder()->finish();
    generator.builder()->appendStages(rec.fPipeline, rec.fAlloc);

    return false;
}

}  // namespace SkSL
