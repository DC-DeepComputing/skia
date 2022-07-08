/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlComputePipeline.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "src/gpu/graphite/mtl/MtlGpu.h"
#include "src/gpu/graphite/mtl/MtlUtils.h"
#include "src/sksl/ir/SkSLProgram.h"

namespace skgpu::graphite {

sk_sp<MtlComputePipeline> MtlComputePipeline::Make(MtlResourceProvider* provider,
                                                   const MtlGpu* gpu,
                                                   const std::string& src) {
    SkSL::Program::Settings settings;
    SkSL::Program::Inputs inputs;
    ShaderErrorHandler* errorHandler = gpu->caps()->shaderErrorHandler();
    std::string msl;
    if (!SkSLToMSL(gpu,
                   src,
                   SkSL::ProgramKind::kCompute,
                   settings,
                   &msl,
                   &inputs,
                   errorHandler)) {
        return nullptr;
    }
    sk_cfp<id<MTLLibrary>> shaderLibrary = MtlCompileShaderLibrary(gpu,
                                                                   msl,
                                                                   errorHandler);
    id<MTLFunction> main = [shaderLibrary.get() newFunctionWithName:@"computeMain"];
    NSError* error;
    sk_cfp<id<MTLComputePipelineState>> pso(
            [gpu->device() newComputePipelineStateWithFunction:main
                                                        error:&error]);
    return sk_sp<MtlComputePipeline>(new MtlComputePipeline(gpu, pso));
}

void MtlComputePipeline::freeGpuData() {
}


} // namespace skgpu::graphite
