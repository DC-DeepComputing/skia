/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/utils/SkShaderUtils.h"

namespace skgpu::graphite {

GraphicsPipeline::GraphicsPipeline(const SharedContext* sharedContext, PipelineInfo* pipelineInfo)
        : Resource(sharedContext, Ownership::kOwned, skgpu::Budgeted::kYes, /*gpuMemorySize=*/0) {
#if defined(GRAPHITE_TEST_UTILS)
<<<<<<< PATCH SET (0e6ef0 Remove SkTime from the public API)
    if (pipelineShaders) {
        fPipelineShaders.fSkSLVertexShader =
                SkShaderUtils::PrettyPrint(pipelineShaders->fSkSLVertexShader);
        fPipelineShaders.fSkSLFragmentShader =
                SkShaderUtils::PrettyPrint(pipelineShaders->fSkSLFragmentShader);
        fPipelineShaders.fNativeVertexShader = std::move(pipelineShaders->fNativeVertexShader);
        fPipelineShaders.fNativeFragmentShader = std::move(pipelineShaders->fNativeFragmentShader);
=======
    if (pipelineInfo) {
        fPipelineInfo.fRenderStepID = pipelineInfo->fRenderStepID;
        fPipelineInfo.fPaintID = pipelineInfo->fPaintID;
        fPipelineInfo.fSkSLVertexShader =
                SkShaderUtils::PrettyPrint(pipelineInfo->fSkSLVertexShader);
        fPipelineInfo.fSkSLFragmentShader =
                SkShaderUtils::PrettyPrint(pipelineInfo->fSkSLFragmentShader);
        fPipelineInfo.fNativeVertexShader = std::move(pipelineInfo->fNativeVertexShader);
        fPipelineInfo.fNativeFragmentShader = std::move(pipelineInfo->fNativeFragmentShader);
>>>>>>> BASE      (f3f6c7 Revert "Roll Dawn from beaf20f90f1b to b9afa50913de (668 rev)
    }
#endif
}

GraphicsPipeline::~GraphicsPipeline() = default;

}  // namespace skgpu::graphite
