/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawPass.h"

#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAlign.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/geom/BoundsManager.h"

#include "src/base/SkMathPriv.h"
#include "src/base/SkTBlockList.h"

#include <algorithm>

using namespace skia_private;

namespace skgpu::graphite {

namespace {



// class GradientBufferTracker {
// public:
//     bool writeData(SkSpan<const float> gradData, DrawBufferManager* bufferMgr) {
//         if (gradData.empty()) {
//             return true;
//         }

//         auto [writer, bufferInfo] = bufferMgr->getSsboWriter(gradData.size(), sizeof(float));

//         if (!writer) {
//             return false;
//         }

//         writer.write(gradData.data(), gradData.size_bytes());
//         fBufferInfo = bufferInfo;
//         fHasData = true;

//         return true;
//     }

//     void bindIfNeeded(DrawPassCommands::List* commandList) const {
//         if (fHasData) {
//             commandList->bindUniformBuffer(fBufferInfo, UniformSlot::kGradient);
//         }
//     }

// private:
//     BindBufferInfo fBufferInfo;
//     bool fHasData = false;
// };

} // namespace

///////////////////////////////////////////////////////////////////////////////////////////////////

DrawPass::DrawPass(sk_sp<TextureProxy> target,
                   std::pair<LoadOp, StoreOp> ops,
                   std::array<float, 4> clearColor)
        : fTarget(std::move(target))
        , fBounds(SkIRect::MakeEmpty())
        , fOps(ops)
        , fClearColor(clearColor) {}

DrawPass::~DrawPass() = default;

bool DrawPass::prepareResources(ResourceProvider* resourceProvider,
                                const RuntimeEffectDictionary* runtimeDict,
                                const RenderPassDesc& renderPassDesc) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    fFullPipelines.reserve(fPipelineDescs.size());
    for (const GraphicsPipelineDesc& pipelineDesc : fPipelineDescs) {
        auto pipeline = resourceProvider->findOrCreateGraphicsPipeline(runtimeDict,
                                                                       pipelineDesc,
                                                                       renderPassDesc);
        if (!pipeline) {
            SKGPU_LOG_W("Failed to create GraphicsPipeline for draw in RenderPass. Dropping pass!");
            return false;
        }
        fFullPipelines.push_back(std::move(pipeline));
    }
    // The DrawPass may be long lived on a Recording and we no longer need the GraphicPipelineDescs
    // once we've created pipelines, so we drop the storage for them here.
    fPipelineDescs.clear();

#if defined(SK_DEBUG)
    for (int i = 0; i < fSampledTextures.size(); ++i) {
        // It should not have been possible to draw an Image that has an invalid texture info
        SkASSERT(fSampledTextures[i]->textureInfo().isValid());
        // Tasks should have been ordered to instantiate any scratch textures already, or any
        // client-owned image will have been instantiated at creation.
        SkASSERTF(fSampledTextures[i]->isInstantiated() ||
                  fSampledTextures[i]->isLazy(),
                  "proxy label = %s", fSampledTextures[i]->label());
    }
#endif

    // fSamplers.reserve(fSamplerDescs.size());
    // for (int i = 0; i < fSamplerDescs.size(); ++i) {
    //     sk_sp<Sampler> sampler = resourceProvider->findOrCreateCompatibleSampler(fSamplerDescs[i]);
    //     if (!sampler) {
    //         SKGPU_LOG_W("Failed to create sampler. Will not create renderpass!");
    //         return false;
    //     }
    //     fSamplers.push_back(std::move(sampler));
    // }
    // The DrawPass may be long lived on a Recording and we no longer need the SamplerDescs
    // once we've created Samplers, so we drop the storage for them here.
    // fSamplerDescs.clear();

    return true;
}

void DrawPass::addResourceRefs(CommandBuffer* commandBuffer) const {
    for (int i = 0; i < fFullPipelines.size(); ++i) {
        commandBuffer->trackResource(fFullPipelines[i]);
    }
    for (int i = 0; i < fSampledTextures.size(); ++i) {
        commandBuffer->trackCommandBufferResource(fSampledTextures[i]->refTexture());
    }
    // for (int i = 0; i < fSamplers.size(); ++i) {
    //     commandBuffer->trackResource(fSamplers[i]);
    // }
}

} // namespace skgpu::graphite
