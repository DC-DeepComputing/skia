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

namespace skgpu::graphite {

GraphicsPipeline::GraphicsPipeline(const SharedContext* sharedContext,
                                   const PipelineInfo& pipelineInfo)
        : Resource(sharedContext,
                   Ownership::kOwned,
                   skgpu::Budgeted::kYes,
                   /*gpuMemorySize=*/0)
        , fPipelineInfo(pipelineInfo) {}

GraphicsPipeline::~GraphicsPipeline() {
#if defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
    static const char* kNames[2] = { "DeletionN", "DeletionP" };
    TRACE_EVENT_INSTANT2("skia.gpu",
                         TRACE_STR_STATIC(kNames[this->fromPrecompile1()]),
                         TRACE_EVENT_SCOPE_THREAD,
                         "key", this->getPipelineInfo().fHash1,
                         "pipelineID", this->getPipelineInfo().fID1);
#endif
}

GraphicsPipeline::PipelineInfo::PipelineInfo(
            const ShaderInfo& shaderInfo,
            SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags
#if defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
            , uint32_t hash,
            uint32_t futureID
#endif
            )
        : fDstReadReq(shaderInfo.dstReadRequirement())
        , fNumFragTexturesAndSamplers(shaderInfo.numFragmentTexturesAndSamplers())
        , fHasPaintUniforms(shaderInfo.hasPaintUniforms())
        , fHasStepUniforms(shaderInfo.hasStepUniforms())
        , fHasGradientBuffer(shaderInfo.hasGradientBuffer())
#if defined(SK_VERBOSE_PIPELINE_LIFETIME_LOGGING)
        , fHash1(hash)
        , fID1(futureID)
#endif
        {
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
