/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ComputePassTask.h"

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/ComputePass.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

ComputePassTask::~ComputePassTask() {}

sk_sp<ComputePassTask> ComputePassTask::Make(std::vector<std::unique_ptr<ComputePass>> passes,
                                             std::vector<ComputeBufferInfo> bufferInfos) {
    return sk_sp<ComputePassTask>(new ComputePassTask(std::move(passes), std::move(bufferInfos)));
}

ComputePassTask::ComputePassTask(std::vector<std::unique_ptr<ComputePass>> passes,
                                 std::vector<ComputeBufferInfo> bufferInfos)
    : fComputePasses(std::move(passes))
    , fBufferInfos(std::move(bufferInfos)) {}

bool ComputePassTask::prepareResources(ResourceProvider* provider) {
    for (const auto& computePass: fComputePasses) {
        if (!computePass->prepareResources(provider)) {
            return false;
        }
    }
    for (ComputeBufferInfo info : fBufferInfos) {
        fBuffers.push_back(provider->findOrCreateBuffer(info.fSize,
                // FIXME what are the right values here?
                BufferType::kXferCpuToGpu,
                PrioritizeGpuReads::kNo));
    }
    return true;
}

extern float* GLOBAL_DEBUG_BUFFER;

bool ComputePassTask::addCommands(ResourceProvider* resourceProvider,
                                  CommandBuffer* commandBuffer) {
    if (commandBuffer->beginComputePass()) {
        SkASSERT(fBuffers.size() == fBufferInfos.size());
        for (size_t i = 0; i < fBuffers.size(); ++i) {
            void* data = fBuffers[i]->map();
            for (int j = 0; j < 512; ++j) {
                ((float*)data)[j] = i == 0 ? j : -1;
            }
            commandBuffer->bindComputeBuffer(fBuffers[i], 0, i);
            if (i == 1) {
                GLOBAL_DEBUG_BUFFER = (float*)data;
            }
        }
        for (const auto& computePass: fComputePasses) {
            if (!computePass->addCommands(commandBuffer)) {
                commandBuffer->endComputePass();
                return false;
            }
        }

        commandBuffer->endComputePass();
    }

    return true;
}

} // namespace skgpu::graphite
