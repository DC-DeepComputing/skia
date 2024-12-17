/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GraphicsPipeline.h"

#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderInfo.h"
#include "src/utils/SkShaderUtils.h"

#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/SharedContext.h"

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
    auto gCache = sharedContext->globalCache();

    fLog = gCache->gimme2(this->getPipelineInfo().fUniqueKeyHash);
}

GraphicsPipeline::~GraphicsPipeline() {
    auto event = this->fromPrecompile() ? SkLog::EventType2::kDeletionP
                                        : SkLog::EventType2::kDeletionN;

    fLog->addEvent2(event, this->id1());

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
    static const char* kNames[2] = { "DeletionN", "DeletionP" };
    SkDebugf("{ %d, %lld, \"%s\", %u, %u },\n",
             fLog->fID,
             micros_since_0(),
             kNames[this->fromPrecompile()],
             this->getPipelineInfo().fUniqueKeyHash,
             this->id1());
    TRACE_EVENT_INSTANT2("skia.gpu",
                         TRACE_STR_STATIC(kNames[this->fromPrecompile()]),
                         TRACE_EVENT_SCOPE_THREAD,
                         "key", this->getPipelineInfo().fUniqueKeyHash,
                         "compilationID", this->getPipelineInfo().fCompilationID);
#endif
}

GraphicsPipeline::PipelineInfo::PipelineInfo(
            const ShaderInfo& shaderInfo,
            SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
            uint32_t uniqueKeyHash,
            uint32_t compilationID)
        : fDstReadReq(shaderInfo.dstReadRequirement())
        , fNumFragTexturesAndSamplers(shaderInfo.numFragmentTexturesAndSamplers())
        , fHasPaintUniforms(shaderInfo.hasPaintUniforms())
        , fHasStepUniforms(shaderInfo.hasStepUniforms())
        , fHasGradientBuffer(shaderInfo.hasGradientBuffer())
        , fUniqueKeyHash(uniqueKeyHash)
        , fCompilationID(compilationID)
        , fFromPrecompile(pipelineCreationFlags & PipelineCreationFlags::kForPrecompilation) {
#if defined(GPU_TEST_UTILS)
    fSkSLVertexShader = SkShaderUtils::PrettyPrint(shaderInfo.vertexSkSL());
    fSkSLFragmentShader = SkShaderUtils::PrettyPrint(shaderInfo.fragmentSkSL());
    fLabel = shaderInfo.fsLabel();
#endif
}

}  // namespace skgpu::graphite
