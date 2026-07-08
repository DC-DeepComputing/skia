/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputeManager_DEFINED
#define skgpu_graphite_ComputeManager_DEFINED

#include "src/gpu/graphite/task/ComputeTask.h"

#include <memory>
#include <optional>

namespace skgpu::graphite {

class ComputeStep;
class Recorder;
struct WorkgroupSize;
struct BindBufferInfo;

class ComputeManager {
public:
    ComputeManager(Recorder* recorder);
    ~ComputeManager();

    // Expose APIs for RenderSteps to record compute steps
    bool addStep(const ComputeStep*, std::optional<WorkgroupSize> globalSize = std::nullopt);
    bool addStepIndirect(const ComputeStep*, BindBufferInfo indirectBuffer);

    // Finalize the dispatches and return them to the caller
    void snapDispatchGroups(ComputeTask::DispatchGroupList* outComputeDispatches);

private:
    Recorder* fRecorder;
    DispatchGroup::Builder fBuilder;
    bool fHasDispatches = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ComputeManager_DEFINED
