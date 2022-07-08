/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputePass_DEFINED
#define skgpu_graphite_ComputePass_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/ComputePipeline.h"

#include <memory>
#include <string>

namespace skgpu::graphite {

class CommandBuffer;
class ResourceProvider;

class ComputePass {
public:
    static std::unique_ptr<ComputePass> Make(std::string src, int threadGroupsX, int threadGroupsY,
                                             int threadGroupsZ, int threadsX, int threadsY,
                                             int threadsZ);

    bool prepareResources(ResourceProvider*);

    bool addCommands(CommandBuffer*) const;

private:
    ComputePass(std::string src, int threadGroupsX, int threadGroupsY, int threadGroupsZ,
                int threadsX, int threadsY, int threadsZ);

    sk_sp<ComputePipeline> fComputePipeline;
    std::string fSrc;
    int fThreadGroupsX, fThreadGroupsY, fThreadGroupsZ;
    int fThreadsX, fThreadsY, fThreadsZ;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ComputePass_DEFINED
