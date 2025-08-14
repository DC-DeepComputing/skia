/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawPass.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkImageInfo.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkTBlockList.h"
#include "src/base/SkVx.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/Resource.h"  // IWYU pragma: keep
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/Texture.h"  // IWYU pragma: keep
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniquePaintParamsID.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform.h"

#if defined(SK_TRACE_GRAPHITE_PIPELINE_USE)
#include "src/gpu/graphite/RenderPassDesc.h"
#endif

#include <algorithm>
#include <cstdint>
#include <optional>
#include <tuple>
#include <vector>

using namespace skia_private;

namespace skgpu::graphite {

class ShaderCodeDictionary;

DrawPass::DrawPass(sk_sp<TextureProxy> target,
                   std::pair<LoadOp, StoreOp> ops,
                   std::array<float, 4> clearColor,
                   sk_sp<FloatStorageManager> floatStorageManager)
        : fTarget(std::move(target))
        , fBounds(SkIRect::MakeEmpty())
        , fOps(ops)
        , fClearColor(clearColor)
        , fFloatStorageManager(floatStorageManager) {}

DrawPass::~DrawPass() = default;

namespace {
bool paint_uses_advanced_blend_equation(std::optional<PaintParams> drawPaintParams) {
    if (!drawPaintParams.has_value() || !drawPaintParams.value().asFinalBlendMode().has_value()) {
        return false;
    }

    return (int)drawPaintParams.value().asFinalBlendMode().value() >
           (int)SkBlendMode::kLastCoeffMode;
}
} // anonymous

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

    for (int i = 0; i < fSampledTextures.size(); ++i) {
        // It should not have been possible to draw an Image that has an invalid texture info
        SkASSERT(fSampledTextures[i]->textureInfo().isValid());
        // Tasks should have been ordered to instantiate any scratch textures already, or any
        // client-owned image will have been instantiated at creation. However, if a TextureProxy
        // was cached for reuse across Recordings, it's possible that the initializing Recording
        // failed, leaving the TextureProxy in a bad state (and currently with no way to reconstruct
        // the tasks required to initialize it).
        // TODO(b/409888039): Once TextureProxies track their dependendent tasks to include in all
        // Recordings, this "should" be able to changed to asserts.
        if (!fSampledTextures[i]->isInstantiated() && !fSampledTextures[i]->isLazy()) {
            SKGPU_LOG_W("Cannot sample from an uninstantiated TextureProxy, label %s",
                        fSampledTextures[i]->label());
            return false;
        }
    }

#if defined(SK_TRACE_GRAPHITE_PIPELINE_USE)
    {
        TRACE_EVENT0_ALWAYS("skia.shaders", "GraphitePipelineUse");
        TRACE_EVENT0_ALWAYS("skia.shaders", TRACE_STR_COPY(renderPassDesc.toString().c_str()));
        for (int i = 0 ; i < fFullPipelines.size(); ++i) {
            TRACE_EVENT_INSTANT1_ALWAYS(
                    "skia.shaders",
                    TRACE_STR_COPY(fFullPipelines[i]->getPipelineInfo().fLabel.c_str()),
                    TRACE_EVENT_SCOPE_THREAD,
                    "area", sk_float_saturate2int(fPipelineDrawAreas[i]));
        }
    }
#endif

    return true;
}

void DrawPass::addResourceRefs(CommandBuffer* commandBuffer) const {
    for (int i = 0; i < fFullPipelines.size(); ++i) {
        commandBuffer->trackResource(fFullPipelines[i]);
    }
    for (int i = 0; i < fSampledTextures.size(); ++i) {
        commandBuffer->trackCommandBufferResource(fSampledTextures[i]->refTexture());
    }
}

} // namespace skgpu::graphite
