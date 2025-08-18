/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawList.h"

#include "include/core/SkTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/geom/Geometry.h"

namespace skgpu::graphite {

namespace {

bool paint_uses_advanced_blend_equation(std::optional<PaintParams> drawPaintParams) {
    if (!drawPaintParams.has_value() || !drawPaintParams.value().asFinalBlendMode().has_value()) {
        return false;
    }

    return (int)drawPaintParams.value().asFinalBlendMode().value() >
           (int)SkBlendMode::kLastCoeffMode;
}

} // anonymous

const Transform& DrawList::deduplicateTransform(const Transform& localToDevice) {
    // TODO: This is a pretty simple deduplication strategy and doesn't take advantage of the stack
    // knowledge that Device has.
    if (fTransforms.empty() || fTransforms.back() != localToDevice) {
        fTransforms.push_back(localToDevice);
    }
    return fTransforms.back();
}

void DrawList::recordDraw(const Renderer* renderer,
                          const Transform& localToDevice,
                          const Geometry& geometry,
                          const Clip& clip,
                          DrawOrder ordering,
                          const PaintParams* paint,
                          UniquePaintParamsID paintID,
                          bool dstReadReq,
                          PipelineDataGatherer* gatherer,
                          const StrokeStyle* stroke) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!geometry.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());
    SkASSERT(!(renderer->depthStencilFlags() & DepthStencilFlags::kStencil) ||
             ordering.stencilIndex() != DrawOrder::kUnassigned);

    // TODO: Add validation that the renderer's expected shape type and stroke params match provided

    // Create a sort key for every render step in this draw, extracting out any
    // RenderStep-specific data.
    UniformDataCache::Index shadingUniformIndex = UniformDataCache::kInvalidIndex;
    if (paintID.isValid()) {
        UniformDataBlock paintUniforms = gatherer->endPaintData();
        if (paintUniforms) {
            shadingUniformIndex = fShadingUniformDataCache.insert(paintUniforms);
        }
    }

    const Draw& draw = fDraws.emplace_back(renderer, this->deduplicateTransform(localToDevice),
                                           geometry, clip, ordering, paint, stroke);

    fRenderStepCount += renderer->numRenderSteps();

    for (int stepIndex = 0; stepIndex < draw.fRenderer->numRenderSteps(); ++stepIndex) {
        gatherer->rewindForRenderStep();

        const RenderStep* const step = draw.fRenderer->steps()[stepIndex];
        const bool performsShading = draw.fPaintParams.has_value() && step->performsShading();

        GraphicsPipelineCache::Index pipelineIndex = fPipelineCache.insert(
                { step->renderStepID(),
                  performsShading ? paintID : UniquePaintParamsID::Invalid() });

        step->writeUniformsAndTextures(draw.fDrawParams, gatherer);
        auto [stepUniforms, combinedTextures] = gatherer->endRenderStepData(performsShading);

        UniformDataCache::Index geomUniformIndex = stepUniforms ?
                fGeometryUniformDataCache.insert(stepUniforms) : UniformDataCache::kInvalidIndex;
        TextureDataCache::Index textureBindingIndex = combinedTextures ?
                fTextureDataCache.insert(combinedTextures) : TextureDataCache::kInvalidIndex;

        fSortKeys.push_back({&draw,
                             stepIndex,
                             pipelineIndex,
                             geomUniformIndex,
                             performsShading ? shadingUniformIndex : UniformDataCache::kInvalidIndex,
                             textureBindingIndex});
    }

    fPassBounds.join(clip.drawBounds());
    fRequiresMSAA |= renderer->requiresMSAA();
    fDepthStencilFlags |= renderer->depthStencilFlags();
    if (dstReadReq) {
        // For paints that read from the dst, update the bounds. It may later be determined that the
        // DstReadStrategy does not require them, but they are inexpensive to track.
        fDstReadBounds.join(clip.drawBounds());
    }

#if defined(SK_DEBUG)
    if (geometry.isCoverageMaskShape()) {
        fCoverageMaskShapeDrawCount++;
    }
#endif
}

std::unique_ptr<DrawPass> DrawList::snapDrawPass(Recorder* recorder,
                                                 sk_sp<TextureProxy> target,
                                                 const SkImageInfo& targetInfo,
                                                 const DstReadStrategy dstReadStrategy) {
    // NOTE: This assert is here to ensure SortKey is as tightly packed as possible. Any change to
    // its size should be done with care and good reason. The performance of sorting the keys is
    // heavily tied to the total size.
    //
    // At 24 bytes (current), sorting is about 30% slower than if SortKey could be packed into just
    // 16 bytes. There are several ways this could be done if necessary:
    //  - Restricting the max draw count to 16k (14-bits) and only using a single index to refer to
    //    the uniform data => 8 bytes of key, 8 bytes of pointer.
    //  - Restrict the max draw count to 32k (15-bits), use a single uniform index, and steal the
    //    4 low bits from the Draw* pointer since it's 16 byte aligned.
    //  - Compact the Draw* to an index into the original collection, although that has extra
    //    indirection and does not work as well with SkTBlockList.
    // In pseudo tests, manipulating the pointer or having to mask out indices was about 15% slower
    // than an 8 byte key and unmodified pointer.
    static_assert(sizeof(SortKey) == SkAlignTo(16 + sizeof(void*), alignof(SortKey)));

    // TODO: Explore sorting algorithms; in all likelihood this will be mostly sorted already, so
    // algorithms that approach O(n) in that condition may be favorable. Alternatively, could
    // explore radix sort that is always O(n). Brief testing suggested std::sort was faster than
    // std::stable_sort and SkTQSort on my [ml]'s Windows desktop. Also worth considering in-place
    // vs. algorithms that require an extra O(n) storage.
    // TODO: It's not strictly necessary, but would a stable sort be useful or just end up hiding
    // bugs in the DrawOrder determination code?
    std::sort(fSortKeys.begin(), fSortKeys.end());

    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "draw count", fDraws.count());

    // The DrawList is converted directly into the DrawPass' data structures, but once the DrawPass
    // is returned from Make(), it is considered immutable.
    std::unique_ptr<DrawPass> drawPass(new DrawPass(target, {fLoadOp, StoreOp::kStore}, fClearColor,
                                                    recorder->priv().refFloatStorageManager()));

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

    TextureTracker textureBindingTracker(&fTextureDataCache);
    for (const DrawList::SortKey& key : fSortKeys) {
        const DrawList::Draw& draw = key.draw();
        const RenderStep& renderStep = key.renderStep();

        const bool pipelineChange = key.pipelineIndex() != lastPipeline;
#if defined(SK_TRACE_GRAPHITE_PIPELINE_USE)
        drawPass->fPipelineDrawAreas[key.pipelineIndex()] +=
                draw.fDrawParams.clip().drawBounds().area();
#endif

        const bool geomBindingChange = geometryUniformTracker.writeUniforms(
                fGeometryUniformDataCache, bufferMgr, key.geometryUniformIndex());
        const bool shadingBindingChange = shadingUniformTracker.writeUniforms(
                fShadingUniformDataCache, bufferMgr, key.shadingUniformIndex());

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

    drawPass->fBounds = fPassBounds.roundOut().asSkIRect();
    drawPass->fPipelineDescs   = fPipelineCache.detach();
    drawPass->fSampledTextures = fTextureDataCache.detachTextures();

    TRACE_COUNTER1("skia.gpu", "# pipelines", drawPass->fPipelineDescs.size());
    TRACE_COUNTER1("skia.gpu", "# textures", drawPass->fSampledTextures.size());
    TRACE_COUNTER1("skia.gpu", "# commands", drawPass->fCommandList.count());

    this->reset(LoadOp::kLoad);

    return drawPass;
}

void DrawList::reset(LoadOp loadOp, SkColor4f color) {
    fLoadOp = loadOp;
    fClearColor = color.premul().array();

    fSortKeys.clear();
    fDraws.reset();
    fTransforms.reset();

    // Accumulate renderer information for each draw added to this list
    fRenderStepCount = 0;
    fRequiresMSAA = false;
    fDepthStencilFlags = DepthStencilFlags::kNone;
    SkDEBUGCODE(fCoverageMaskShapeDrawCount = 0);

    fDstReadBounds = Rect::InfiniteInverted();
    fPassBounds = Rect::InfiniteInverted();

    fGeometryUniformDataCache.reset();
    fShadingUniformDataCache.reset();
    fTextureDataCache.reset();
    fPipelineCache.reset();
}

} // namespace skgpu::graphite
