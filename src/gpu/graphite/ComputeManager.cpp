/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ComputeManager.h"

#include "include/gpu/graphite/Recorder.h"

namespace skgpu::graphite {

ComputeManager::ComputeManager(Recorder* recorder)
        : fRecorder(recorder)
        , fBuilder(recorder) {}

ComputeManager::~ComputeManager() = default;

bool ComputeManager::addStep(const ComputeStep* step, std::optional<WorkgroupSize> globalSize) {
    if (fBuilder.appendStep(step, globalSize)) {
        fHasDispatches = true;
        return true;
    }
    return false;
}

bool ComputeManager::addStepIndirect(const ComputeStep* step, BindBufferInfo indirectBuffer) {
    if (fBuilder.appendStepIndirect(step, indirectBuffer)) {
        fHasDispatches = true;
        return true;
    }
    return false;
}

void ComputeManager::snapDispatchGroups(ComputeTask::DispatchGroupList* outComputeDispatches) {
    if (fHasDispatches) {
        if (std::unique_ptr<DispatchGroup> dg = fBuilder.finalize()) {
            outComputeDispatches->emplace_back(std::move(dg));
        }
        fHasDispatches = false;
    }
}

} // namespace skgpu::graphite
