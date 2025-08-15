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

std::unique_ptr<DrawPass> DrawPass::Make(Recorder* recorder,
                                         std::unique_ptr<DrawList> draws,
                                         sk_sp<TextureProxy> target,
                                         const SkImageInfo& targetInfo,
                                         std::pair<LoadOp, StoreOp> ops,
                                         std::array<float, 4> clearColor,
                                         const DstReadStrategy dstReadStrategy) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "draw count", draws->fDraws.count());

    // The DrawList is converted directly into the DrawPass' data structures, but once the DrawPass
    // is returned from Make(), it is considered immutable.
    std::unique_ptr<DrawPass> drawPass(new DrawPass(target, ops, clearColor,
                                                    recorder->priv().refFloatStorageManager()));
    if (!draws->prepareSortKeys(recorder, targetInfo)) {
        return nullptr;
    }
    const std::vector<DrawList::SortKey>& keys = draws->fSortKeys;

    DrawBufferManager* bufferMgr = recorder->priv().drawBufferManager();
    DrawWriter drawWriter(&drawPass->fCommandList, bufferMgr);
    GraphicsPipelineCache::Index lastPipeline = GraphicsPipelineCache::kInvalidIndex;
    const SkIRect targetBounds = SkIRect::MakeSize(targetInfo.dimensions());
    SkIRect lastScissor = targetBounds;

    SkASSERT(drawPass->fTarget->isFullyLazy() ||
             SkIRect::MakeSize(drawPass->fTarget->dimensions()).contains(lastScissor));
    drawPass->fCommandList.setScissor(lastScissor);

    const Caps* caps = recorder->priv().caps();
    const bool useStorageBuffers = caps->storageBufferSupport();
    UniformTracker geometryUniformTracker(useStorageBuffers);
    UniformTracker shadingUniformTracker(useStorageBuffers);

    // TODO(b/372953722): Remove this forced binding command behavior once dst copies are always
    // bound separately from the rest of the textures.
    const bool rebindTexturesOnPipelineChange = dstReadStrategy == DstReadStrategy::kTextureCopy;
    // Keep track of the prior draw's PaintOrder. If the current draw requires barriers and there
    // is no pipeline or state change, then we must compare the current and prior draw's PaintOrders
    // to determine if the draws overlap. If they do, we must inject a flush between them such that
    // the barrier addition and draw commands are ordered correctly.
    CompressedPaintersOrder priorDrawPaintOrder {};

    // If a draw uses an advanced blend mode and the device supports this via noncoherent blending,
    // then we must insert the appropriate barrier and ensure that the draws do not overlap.
    const bool advancedBlendsRequireBarrier =
            caps->blendEquationSupport() == Caps::BlendEquationSupport::kAdvancedNoncoherent;

#if defined(SK_TRACE_GRAPHITE_PIPELINE_USE)
    // Accumulate rough pixel area touched by each pipeline as we iterate the SortKeys
    drawPass->fPipelineDrawAreas.push_back_n(pipelineCache.count(), 0.f);
#endif

    TextureTracker textureBindingTracker(&draws->fTextureDataCache);
    for (const DrawList::SortKey& key : keys) {
        const DrawList::Draw& draw = key.draw();
        const RenderStep& renderStep = key.renderStep();

        const bool pipelineChange = key.pipelineIndex() != lastPipeline;
#if defined(SK_TRACE_GRAPHITE_PIPELINE_USE)
        drawPass->fPipelineDrawAreas[key.pipelineIndex()] +=
                draw.fDrawParams.clip().drawBounds().area();
#endif

        const bool geomBindingChange = geometryUniformTracker.writeUniforms(
                draws->fGeometryUniformDataCache, bufferMgr, key.geometryUniformIndex());
        const bool shadingBindingChange = shadingUniformTracker.writeUniforms(
                draws->fShadingUniformDataCache, bufferMgr, key.shadingUniformIndex());

        // TODO(b/372953722): The Dawn and Vulkan CommandBuffer implementations currently append any
        // dst copy to the texture bind group/descriptor set automatically when processing a
        // BindTexturesAndSamplers call because they use a single group to contain all textures.
        // However, from the DrawPass POV, we can run into the scenario where two pipelines have the
        // same textures+samplers except one requires a dst-copy and the other does not. In this
        // case we wouldn't necessarily insert a new command when the pipeline changed and then
        // end up with layout validation errors.
        const bool textureBindingsChange = textureBindingTracker.setCurrentTextureBindings(
                key.textureBindingIndex()) ||
                (rebindTexturesOnPipelineChange && pipelineChange &&
                 key.textureBindingIndex() != TextureDataCache::kInvalidIndex);

        std::optional<SkIRect> newScissor =
                renderStep.getScissor(draw.fDrawParams, lastScissor, targetBounds);

        // Determine + analyze draw properties to inform whether we need to issue barriers before
        // issuing draw calls.
        bool drawsOverlap = priorDrawPaintOrder != draw.fDrawParams.order().paintOrder();
        bool drawUsesAdvancedBlendMode = paint_uses_advanced_blend_equation(draw.fPaintParams);

        std::optional<BarrierType> barrierToAddBeforeDraws = std::nullopt;
        if (dstReadStrategy == DstReadStrategy::kReadFromInput && draw.readsFromDst()) {
            barrierToAddBeforeDraws = BarrierType::kReadDstFromInput;
        }
        if (drawUsesAdvancedBlendMode &&
            caps->supportsHardwareAdvancedBlending() &&
            advancedBlendsRequireBarrier) {
            // A draw should only read from the dst OR use hardware for advanced blend modes.
            SkASSERT(!draw.readsFromDst());

            barrierToAddBeforeDraws = BarrierType::kAdvancedNoncoherentBlend;
        }

        const bool stateChange = geomBindingChange ||
                                 shadingBindingChange ||
                                 textureBindingsChange ||
                                 newScissor.has_value();

        // Update DrawWriter *before* we actually change any state so that accumulated draws from
        // the previous state use the proper state.
        if (pipelineChange) {
            drawWriter.newPipelineState(renderStep.primitiveType(),
                                        renderStep.staticDataStride(),
                                        renderStep.appendDataStride(),
                                        renderStep.getRenderStateFlags(),
                                        barrierToAddBeforeDraws);
        } else if (stateChange) {
            drawWriter.newDynamicState();
        } else if (barrierToAddBeforeDraws.has_value() && drawsOverlap) {
            // Even if there is no pipeline or state change, we must consider whether a
            // DrawPassCommand to add barriers must be inserted before any draw commands. If so,
            // then determine if the current and prior draws overlap (ie, their PaintOrders are
            // unequal). If so, perform a flush() to make sure the draw and add barrier commands are
            // appended to the command list in the proper order.
            drawWriter.flush();
        }

        // Make state changes before accumulating new draw data
        if (pipelineChange) {
            drawPass->fCommandList.bindGraphicsPipeline(key.pipelineIndex());
            lastPipeline = key.pipelineIndex();
        }
        if (stateChange) {
            if (geomBindingChange) {
                geometryUniformTracker.bindUniforms(UniformSlot::kRenderStep,
                                                    &drawPass->fCommandList);
            }
            if (shadingBindingChange) {
                shadingUniformTracker.bindUniforms(UniformSlot::kPaint, &drawPass->fCommandList);
            }
            if (textureBindingsChange) {
                textureBindingTracker.bindTextures(&drawPass->fCommandList);
            }
            if (newScissor.has_value()) {
                drawPass->fCommandList.setScissor(*newScissor);
                lastScissor = *newScissor;
            }
        }

        uint32_t geometrySsboIndex = useStorageBuffers ? geometryUniformTracker.ssboIndex() : 0;
        uint32_t shadingSsboIndex = useStorageBuffers ? shadingUniformTracker.ssboIndex() : 0;
        skvx::uint2 ssboIndices = {geometrySsboIndex, shadingSsboIndex};
        renderStep.writeVertices(&drawWriter, draw.fDrawParams, ssboIndices);

        if (bufferMgr->hasMappingFailed()) {
            SKGPU_LOG_W("Failed to write necessary vertex/instance data for DrawPass, dropping!");
            return nullptr;
        }

        // Update priorDrawPaintOrder value before iterating to analyze the next draw.
        priorDrawPaintOrder = draw.fDrawParams.order().paintOrder();
    }
    // Finish recording draw calls for any collected data still pending at end of the loop
    drawWriter.flush();

    drawPass->fBounds = draws->fPassBounds.roundOut().asSkIRect();
    drawPass->fPipelineDescs   = draws->fPipelineCache.detach();
    drawPass->fSampledTextures = draws->fTextureDataCache.detachTextures();

    TRACE_COUNTER1("skia.gpu", "# pipelines", drawPass->fPipelineDescs.size());
    TRACE_COUNTER1("skia.gpu", "# textures", drawPass->fSampledTextures.size());
    TRACE_COUNTER1("skia.gpu", "# commands", drawPass->fCommandList.count());

    return drawPass;
}

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
