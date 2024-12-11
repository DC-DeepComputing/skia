/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GraphicsPipeline.h"

#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderInfo.h"
#include "src/utils/SkShaderUtils.h"

skgpu::StdSteadyClock::time_point fTimeStart;

int64_t micros_since_0();

namespace skgpu::graphite {

GraphicsPipeline::GraphicsPipeline(const SharedContext* sharedContext,
                                   const PipelineInfo& pipelineInfo)
        : Resource(sharedContext,
                   Ownership::kOwned,
                   skgpu::Budgeted::kYes,
                   /*gpuMemorySize=*/0)
        , fPipelineInfo(pipelineInfo) {
}

GraphicsPipeline::~GraphicsPipeline() {
    static const char* kNames[2] = { "DeletionN", "DeletionP" };
#if 1
    SkDebugf("{ %d, %lld, \"%s\", %u, -1, %u, UINT32_MAX },\n",
             -1,
             micros_since_0(),
             kNames[this->fromPrecompile1()],
             this->getPipelineInfo().fHash1,
             this->id1());
#endif
    TRACE_EVENT_INSTANT2("skia.gpu",
                         TRACE_STR_STATIC(kNames[this->fromPrecompile1()]),
                         TRACE_EVENT_SCOPE_THREAD,
                         "key", 0,
                         "pipelineID", this->id1());
}

GraphicsPipeline::PipelineInfo::PipelineInfo(
            const ShaderInfo& shaderInfo,
            SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
            uint32_t hash,
            uint32_t futureID)
        : fDstReadReq(shaderInfo.dstReadRequirement())
        , fNumFragTexturesAndSamplers(shaderInfo.numFragmentTexturesAndSamplers())
        , fHasPaintUniforms(shaderInfo.hasPaintUniforms())
        , fHasStepUniforms(shaderInfo.hasStepUniforms())
        , fHasGradientBuffer(shaderInfo.hasGradientBuffer())
        , fHash1(hash)
        , fID1(futureID) {
#if defined(GPU_TEST_UTILS)
    fSkSLVertexShader = SkShaderUtils::PrettyPrint(shaderInfo.vertexSkSL());
    fSkSLFragmentShader = SkShaderUtils::PrettyPrint(shaderInfo.fragmentSkSL());
    fLabel = shaderInfo.fsLabel();
#endif
#if SK_HISTOGRAMS_ENABLED || defined(GPU_TEST_UTILS)
    fFromPrecompile1 =
            SkToBool(pipelineCreationFlags & PipelineCreationFlags::kForPrecompilation);
#endif
}

}  // namespace skgpu::graphite
