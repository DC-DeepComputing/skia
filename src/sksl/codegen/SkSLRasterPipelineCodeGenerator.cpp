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
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <cstddef>
#include <vector>

namespace SkSL {

class SkRPGenerator {
public:
    SkRPGenerator(const Program& program,
                  const SkStageRec& rec,
                  SkRasterPipelineCallbacks* callbacks)
            : fProgram(program), fRec(rec), fCallbacks(callbacks) {}

    void writeProgram(const FunctionDefinition& function);

    // TODO(johnstiles): not yet implemented
    void copyRGToSlot(int slotIdx) {}
    void copyRGBAToSlot(int slotIdx) {}
    void copyDRGBAToSlot(int slotIdx) {}

    /** Slots hold our program's data. */
    int createSlot(const std::string& name,
                   const Type& type,
                   Position pos,
                   bool isFunctionReturnValue);
    int getSlot(const Variable& v);

private:
    //
    // Global state for the lifetime of the generator:
    //
    [[maybe_unused]] const Program&             fProgram;
    [[maybe_unused]] const SkStageRec&          fRec;
    [[maybe_unused]] SkRasterPipelineCallbacks* fCallbacks;

    // A single scalar in our program consumes one slot.
    struct Slot {
        void*      fData;               // points to four bytes allocated in fAlloc
        bool       fWrittenTo = false;  // for debugger
    };
    std::vector<Slot> fSlots;

    // [Variable/Function, first slot in fSlots]
    SkTHashMap<const IRNode*, size_t> fSlotMap;
};

void SkRPGenerator::writeProgram(const FunctionDefinition& function) {
    // TODO(johnstiles): not yet implemented
}

int SkRPGenerator::createSlot(const std::string& name,
                              const Type& type,
                              Position pos,
                              bool isFunctionReturnValue) {
    // TODO: `name`, `pos` and `isFunctionReturnValue` will be used by the debugger.
    size_t slot   = fSlots.size(),
           nslots = type.slotCount();

    // Create brand new slots, value-initialized to zero.
    float* slotPtr = fRec.fAlloc->makeArray<float>(nslots * sizeof(float));
    while (nslots--) {
        fSlots.push_back(Slot{slotPtr++});
    }
    return slot;
}

int SkRPGenerator::getSlot(const Variable& v) {
    size_t* entry = fSlotMap.find(&v);
    if (entry != nullptr) {
        return *entry;
    }

    size_t slot = this->createSlot(std::string(v.name()), v.type(), v.fPosition,
                                   /*isFunctionReturnValue=*/false);
    fSlotMap.set(&v, slot);
    return slot;
}

bool AppendStages(const Program& program,
                  const FunctionDefinition& function,
                  const SkStageRec& rec,
                  SkRasterPipelineCallbacks* callbacks) {
    SkRPGenerator generator(program, rec, callbacks);

    int args[8];  // at most 8 arguments (half4 srcColor, half4 dstColor)
    size_t argSlots = 0;
    for (const SkSL::Variable* param : function.declaration().parameters()) {
        switch (param->modifiers().fLayout.fBuiltin) {
            case SK_MAIN_COORDS_BUILTIN: {
                SkASSERT(param->type().slotCount() == 2);
                SkASSERT((argSlots + 2) <= std::size(args));
                int fragCoord = generator.getSlot(*param);
                args[argSlots++] = fragCoord++;
                args[argSlots++] = fragCoord++;
                generator.copyRGToSlot(fragCoord);
                break;
            }
            case SK_INPUT_COLOR_BUILTIN: {
                SkASSERT(param->type().slotCount() == 4);
                SkASSERT((argSlots + 4) <= std::size(args));
                int color = generator.getSlot(*param);
                args[argSlots++] = color++;
                args[argSlots++] = color++;
                args[argSlots++] = color++;
                args[argSlots++] = color++;
                generator.copyRGBAToSlot(color);
                break;
            }
            case SK_DEST_COLOR_BUILTIN: {
                SkASSERT(param->type().slotCount() == 4);
                SkASSERT((argSlots + 4) <= std::size(args));
                int color = generator.getSlot(*param);
                args[argSlots++] = color++;
                args[argSlots++] = color++;
                args[argSlots++] = color++;
                args[argSlots++] = color++;
                generator.copyDRGBAToSlot(color);
                break;
            }
            default:
                SkDEBUGFAIL("Invalid parameter to main()");
                return false;
        }
    }
    SkASSERT(argSlots <= 8);

    generator.writeProgram(function);

    return false;
}

}  // namespace SkSL
