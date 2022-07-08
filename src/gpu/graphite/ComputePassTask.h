/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputePassTask_DEFINED
#define skgpu_graphite_ComputePassTask_DEFINED

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/Task.h"

#include <vector>

namespace skgpu::graphite {

class CommandBuffer;
class ComputePass;
class Context;

struct ComputeBufferInfo {
    size_t fSize;
};

class ComputePassTask final : public Task {
public:
    static sk_sp<ComputePassTask> Make(std::vector<std::unique_ptr<ComputePass>> passes,
                                       // I don't think having this here is right. Presumably they
                                       // should be per pass, but I haven't had time to think
                                       // through this fully.
                                       std::vector<ComputeBufferInfo> bufferInfos);

    ~ComputePassTask() override;

    bool prepareResources(ResourceProvider*) override;

    bool addCommands(ResourceProvider*, CommandBuffer*) override;

private:
    ComputePassTask(std::vector<std::unique_ptr<ComputePass>> passes,
                    std::vector<ComputeBufferInfo> bufferInfos);

    std::vector<std::unique_ptr<ComputePass>> fComputePasses;
    std::vector<ComputeBufferInfo> fBufferInfos;
    std::vector<sk_sp<Buffer>> fBuffers;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ComputePassTask_DEFINED
