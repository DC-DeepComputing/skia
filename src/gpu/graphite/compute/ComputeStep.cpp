/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/ComputeStep.h"

#include "include/private/base/SkAssert.h"

#include <unordered_set>

namespace skgpu::graphite {
namespace {

static uint32_t next_id() {
    static std::atomic<int32_t> nextId{0};
    // Not worried about overflow since a Context isn't expected to have that many ComputeSteps.
    // Even if this it wraps around to 0, that ComputeStep will not be in the same Context as the
    // original 0.
    return nextId.fetch_add(1, std::memory_order_relaxed);
}

}  // namespace

ComputeStep::ComputeStep(std::string_view name, SkSpan<const Resource> resources)
        : fUniqueID(next_id())
        , fFlags(Flags::kNone)
        , fName(name)
        , fResources(resources.begin(), resources.end()) {
#ifdef SK_DEBUG
    std::unordered_set<int> inputSlots, outputSlots;
#endif
    for (const Resource& r : fResources) {
#ifdef SK_DEBUG
        // Validate that slot assignments are unique among inputs and outputs.
        bool isInput = (r.fFlow == DataFlow::kInputSlot);
        bool isOutput = (r.fFlow == DataFlow::kOutputSlot);
        if (isInput || isOutput) {
            SkASSERT(r.fSlot > -1);
            if (isInput) {
                auto [_, inserted] = inputSlots.insert(r.fSlot);
                SkASSERT(inserted);
            } else if (isOutput) {
                auto [_, inserted] = outputSlots.insert(r.fSlot);
                SkASSERT(inserted);
            }
        }
#endif  // SK_DEBUG
        switch (r.fFlow) {
            case DataFlow::kPathInput:
                fFlags |= Flags::kConsumesPath;
                break;
            case DataFlow::kVertexOutput:
            case DataFlow::kIndexOutput:
            case DataFlow::kInstanceOutput:
                SkASSERT(r.fType != ResourceType::kUniformBuffer);
                fFlags |= Flags::kOutputsGeometry;
                break;
            case DataFlow::kIndirectOutput:
                fFlags |= Flags::kDrivesIndirectDraw;
                break;
            default:
                break;
        }
    }
}

void ComputeStep::writePathInput(const DrawParams&, int ssboIndex, void* buffer) {
    SK_ABORT("not implemented");
}

}  // namespace skgpu::graphite
