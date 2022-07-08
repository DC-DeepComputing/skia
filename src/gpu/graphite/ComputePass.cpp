/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ComputePass.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ComputePass.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

ComputePass::ComputePass(std::string src, int threadGroupsX, int threadGroupsY, int threadGroupsZ,
                         int threadsX, int threadsY, int threadsZ)
    : fSrc(src)
    , fThreadGroupsX(threadGroupsX)
    , fThreadGroupsY(threadGroupsY)
    , fThreadGroupsZ(threadGroupsZ)
    , fThreadsX(threadsX)
    , fThreadsY(threadsY)
    , fThreadsZ(threadsZ) {}

std::unique_ptr<ComputePass> ComputePass::Make(std::string src, int threadGroupsX,
                                               int threadGroupsY, int threadGroupsZ,
                                               int threadsX, int threadsY, int threadsZ) {
    return std::unique_ptr<ComputePass>(new ComputePass(std::move(src), threadGroupsX,
                                                        threadGroupsY, threadGroupsZ, threadsX,
                                                        threadsY, threadsZ));
}

bool ComputePass::prepareResources(ResourceProvider* provider) {
    fComputePipeline = provider->findOrCreateComputePipeline(fSrc);
    return fComputePipeline != nullptr;
}

bool ComputePass::addCommands(CommandBuffer* commandBuffer) const {
    commandBuffer->bindComputePipeline(fComputePipeline);
    commandBuffer->execute(fThreadGroupsX, fThreadGroupsY, fThreadGroupsZ,
                           fThreadsX, fThreadsY, fThreadsZ);
    return true;
}


} // namespace skgpu::graphite
