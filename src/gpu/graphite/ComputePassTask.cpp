/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ComputePassTask.h"

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

sk_sp<ComputePassTask> ComputePassTask::Make(std::vector<ResourceBinding> bindings,
                                             const ComputePipelineDesc& pipelineDesc,
                                             const ComputePassDesc& desc) {
    return sk_sp<ComputePassTask>(new ComputePassTask(std::move(bindings), pipelineDesc, desc));
}

sk_sp<ComputePassTask> ComputePassTask::MakeHackTaskForLocalTesting(ResourceProvider* provider) {
    constexpr uint32_t HACK_PROBLEM_SIZE = 512;

    ComputePipelineDesc pipelineDesc;
    pipelineDesc.setProgram(
            "layout(set=0, binding=0) in float in_data[];"
            "layout(set=0, binding=1) out float out_data[];"
            "void main() {"
            "    out_data[sk_ThreadPosition.x] = in_data[sk_ThreadPosition.x] * 2;"
            "}",
            "HackComputeTaskForTesting");

    std::vector<ResourceBinding> bindings;
    bindings.push_back({
        0,  // index
        {
            provider->findOrCreateBuffer(sizeof(float) * HACK_PROBLEM_SIZE,
                                         BufferType::kXferCpuToGpu,
                                         PrioritizeGpuReads::kNo),
            0  // offset
        }
    });
    bindings.push_back({
        1,  // index
        {
            provider->findOrCreateBuffer(sizeof(float) * HACK_PROBLEM_SIZE,
                                         BufferType::kXferGpuToCpu,
                                         PrioritizeGpuReads::kNo),
            0  // offset
        }
    });

    float* srcData = static_cast<float*>(bindings[0].fResource.fBuffer->map());
    float* dstData = static_cast<float*>(bindings[1].fResource.fBuffer->map());
    for (unsigned int i = 0; i < HACK_PROBLEM_SIZE; ++i) {
        srcData[i] = i;
        dstData[i] = -1;
    }
    bindings[0].fResource.fBuffer->unmap();
    bindings[1].fResource.fBuffer->unmap();

    ComputePassDesc desc;
    desc.fLocalDispatchSize = WorkgroupSize(HACK_PROBLEM_SIZE, 1, 1);

    return ComputePassTask::Make(std::move(bindings), pipelineDesc, desc);
}

ComputePassTask::ComputePassTask(std::vector<ResourceBinding> bindings,
                                 const ComputePipelineDesc& pipelineDesc,
                                 const ComputePassDesc& desc)
        : fPipelineDesc(pipelineDesc)
        , fComputePassDesc(desc)
        , fBindings(std::move(bindings)) {}

bool ComputePassTask::prepareResources(ResourceProvider* provider) {
    fPipeline = provider->findOrCreateComputePipeline(fPipelineDesc);
    return fPipeline != nullptr;
}

bool ComputePassTask::addCommands(ResourceProvider* provider,
                                  CommandBuffer* commandBuffer) {
    SkASSERT(fPipeline);
    commandBuffer->addComputePass(fComputePassDesc, fPipeline, fBindings);
    return true;
}

}  // namespace skgpu::graphite
