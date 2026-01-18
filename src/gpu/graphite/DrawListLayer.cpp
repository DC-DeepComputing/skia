/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawListLayer.h"

#include "include/core/SkTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/geom/Geometry.h"

namespace skgpu::graphite {

void DrawListLayer::reset(LoadOp loadOp, SkColor4f color) {
    DrawListBase::reset(loadOp, color);

    fStorage.reset();
    fLayers.reset();
    fDrawCount = 0;
}

void DrawListLayer::recordStep(int stepIndex, // Awkward, test utils only
                               const RenderStep* step,
                               const LayerKey& key,
                               const DrawParams& drawParams,
                               const SkEnumBitMask<DstUsage>& dstUsage,
                               const BarrierType& barrierBeforeDraws) {
    SkTInternalLList<Layer>::Iter iter;
    Layer* current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
    Layer* targetLayer = nullptr;
    BindingList* targetMatch = nullptr;

    bool dependsOnDst = (dstUsage & DstUsage::kDependsOnDst) != DstUsage::kNone;
    bool disjointStencil =
            (step->depthStencilFlags() & DepthStencilFlags::kStencil) != DepthStencilFlags::kNone;

    // If we're an easy draw, try the head first.
    if (!dependsOnDst && !disjointStencil) {
        targetLayer = fLayers.head();
        current = nullptr;

        if (targetLayer) {
            auto result = targetLayer->test(drawParams.drawBounds(), key, disjointStencil);
            if (result != BoundsTest::kIncompatibleOverlap) {
                targetMatch = targetLayer->searchBinding(key);
            } else {
                targetLayer = nullptr;
                current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
            }
        }
    }

    int limit = kMaxSearchLimit;
    BindingList* forwardMerge = nullptr;
    while (current && limit > 0) {
        BoundsTest result = current->test(drawParams.drawBounds(), key, disjointStencil);
        if (result == BoundsTest::kIncompatibleOverlap) {
            // If we need to read the dst, we cannot go earlier than this layer.
            if (dependsOnDst) {
                if (current->fNext) {
                    targetLayer = current->fNext;

                    // We found our target layer, grab the binding if it exists
                    targetMatch = targetLayer->searchBinding(key);
                    break;
                } else {
                    // We are at the end (Tail), so we need a new layer on top.

                    // Check if we can "forward merge" an existing batch from this layer
                    // to the new layer to prevent stranding.
                    // BindingList* match = current->searchBinding(key);

                    // bool safeToMerge =
                    //         match && (!disjointStencil ||
                    //                   !match->fBounds.intersects(drawParams.drawBounds()));
                    // if (safeToMerge) {
                    //     // Don't leave an empty layer behind!
                    //     if (current->fBindings.head() != match ||
                    //         current->fBindings.tail() != match) {
                    //         forwardMerge = match;
                    //     }
                    // }
                    break;
                }
            }
            // If !dependsOnDst, we just keep searching backwards.
        } else {
            // Found a valid layer (Compatible or Disjoint)
            targetLayer = current;

            // If it was compatible, we expect a match. If disjoint, match is nullptr.
            if (result == BoundsTest::kCompatibleOverlap) {
                targetMatch = current->searchBinding(key);
                break;
            }
            // If Disjoint, we can theoretically stay here, but we keep searching backwards
            // to see if there is a 'Compatible' layer further back to batch with.
        }

        current = iter.prev();
        limit--;
    }

    if (!targetLayer) {
        targetLayer = fStorage.make<Layer>();
        fLayers.addToTail(targetLayer);

        if (forwardMerge) {
            SkASSERT(current);
            current->fBindings.remove(forwardMerge);
            targetLayer->fBindings.addToHead(forwardMerge);
            targetMatch = forwardMerge;
        }
    } else {
        SkASSERT(!forwardMerge);
    }

    SkASSERT(targetLayer);
    RecordedDraw* draw = fStorage.make<RecordedDraw>(step, drawParams, barrierBeforeDraws);
    targetLayer->add(&fStorage, targetMatch, key, draw, dependsOnDst);

#ifdef GPU_TEST_UTILS
    this->fLastLayer = targetLayer;
    if (fDrawObserver) {
        fDrawObserver(stepIndex, key, targetLayer->fLastDraw);
    }
#endif
}

void DrawListLayer::recordDepthOnly(
        int stepIndex,
        const RenderStep* step,
        const LayerKey& key,
        const DrawParams& drawParams,
        const BarrierType& barrierBeforeDraws,
        std::optional<std::reference_wrapper<DrawParams*>> captureParams,
        std::optional<std::reference_wrapper<Layer*>> captureLayer) {
    // Child depth only fast path to do their parent
    if (stepIndex > 0) {
        DeferredDraw* deferredDraw = fStorage.make<DeferredDraw>(key, step, drawParams,
                                                                 barrierBeforeDraws, captureParams);
        SkASSERT(fLastDeferredDraw && !fLastDeferredDraw->fChildDraw);
        fLastDeferredDraw->fChildDraw = deferredDraw;
        fLastDeferredDraw = deferredDraw;
        return;
    }
    SkTInternalLList<Layer>::Iter iter;
    Layer* current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
    Layer* targetLayer = nullptr;

    bool disjointStencil =
            (step->depthStencilFlags() & DepthStencilFlags::kStencil) != DepthStencilFlags::kNone;

    int limit = kMaxSearchLimit;
    bool shouldOverwriteLayer = true;
    while (current && limit > 0) {
        BoundsTest result = current->depthOnlyTest(drawParams.drawBounds(), key, disjointStencil);
        if (captureLayer->get() == current) {
            shouldOverwriteLayer = false;
        }
        if (result == BoundsTest::kIncompatibleOverlap) {
            if (current->fNext) {
                targetLayer = current->fNext;
            } else {
                // BindingList* match = current->searchBinding(key);
                // bool safeToMerge =
                //         match && (!disjointStencil ||
                //                     !match->fBounds.intersects(drawParams.drawBounds()));
                // if (safeToMerge) {
                //     // Don't leave an empty layer behind!
                //     if (current->fBindings.head() != match ||
                //         current->fBindings.tail() != match) {
                //         forwardMerge = match;
                //     }
                // }
                // disable forwardMerging
            }
            break;
        } else { // compatible or disjoint or something
            // Found a valid layer (Compatible or Disjoint)
            targetLayer = current;

            // If it was compatible, we expect a match. If disjoint, match is nullptr.
            if (result == BoundsTest::kCompatibleOverlap) {
                break;
            }
            // If Disjoint, we can theoretically stay here, but we keep searching backwards
            // to see if there is a 'Compatible' layer further back to batch with.
        }

        current = iter.prev();
        limit--;
    }

    if (!targetLayer) {
        targetLayer = fStorage.make<Layer>();
        fLayers.addToTail(targetLayer);
        shouldOverwriteLayer = true;
    }
    SkASSERT(targetLayer);

    DeferredDraw* deferredDraw = fStorage.make<DeferredDraw>(key, step, drawParams,
                                                             barrierBeforeDraws, captureParams);
    targetLayer->addDepthOnlyDraw(&fStorage, deferredDraw, disjointStencil);
    fLastDeferredDraw = deferredDraw;
    if (shouldOverwriteLayer) {
        captureLayer->get() = targetLayer;
    }
}

// Clipped draws start at the layer containing their depth only draw, then proceed FORWARDS.
void DrawListLayer::recordClipped(
    int stepIndex,
        const RenderStep* step,
        const LayerKey& key,
        const DrawParams& drawParams,
        const SkEnumBitMask<DstUsage>& dstUsage,
        const BarrierType& barrierBeforeDraws,
        std::optional<std::reference_wrapper<Layer*>> startLayer) {
    Layer* current = startLayer->get();
    Layer* targetLayer = nullptr;
    BindingList* targetMatch = nullptr;

    bool dependsOnDst = (dstUsage & DstUsage::kDependsOnDst) != DstUsage::kNone;
    bool disjointStencil =
            (step->depthStencilFlags() & DepthStencilFlags::kStencil) != DepthStencilFlags::kNone;

    int limit = kMaxSearchLimit;
    while (current && limit > 0) {
        BoundsTest result = current->test(drawParams.drawBounds(), key, disjointStencil);
        if (result != BoundsTest::kIncompatibleOverlap) {
             // Found a valid layer (Compatible or Disjoint)
            targetLayer = current;
            targetMatch = current->searchBinding(key);
            break;
        }

        current = current->fNext;
        limit--;
    }

    if (!targetLayer) {
        targetLayer = fStorage.make<Layer>();
        fLayers.addToTail(targetLayer);
    }

    SkASSERT(targetLayer);
    RecordedDraw* draw = fStorage.make<RecordedDraw>(step, drawParams, barrierBeforeDraws);
    targetLayer->add(&fStorage, targetMatch, key, draw, dependsOnDst);

#ifdef GPU_TEST_UTILS
    this->fLastLayer = targetLayer;
    if (fDrawObserver) {
        fDrawObserver(stepIndex, key, targetLayer->fLastDraw);
    }
#endif
}

// Layer has dual purpose here:
//  1) (Producer) If recording a depth only draw, the pointer is set to the *latest* layer inserted.
//  2) (Consumer) If recording a clipped draw, the pointer is used as the starting point for a
//     forward search, and is the earliest possible layer the draw can be inserted into.
void DrawListLayer::recordDraw(const Renderer* renderer,
                               const Transform& localToDevice,
                               const Geometry& geometry,
                               const Clip& clip,
                               DrawOrder ordering,
                               UniquePaintParamsID paintID,
                               SkEnumBitMask<DstUsage> dstUsage,
                               BarrierType barrierBeforeDraws,
                               PipelineDataGatherer* gatherer,
                               const StrokeStyle* stroke,
                               std::optional<std::reference_wrapper<DrawParams*>> captureParams,
                               std::optional<std::reference_wrapper<Layer*>> layer) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!geometry.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());
    SkASSERT(!(renderer->depthStencilFlags() & DepthStencilFlags::kStencil) ||
             ordering.stencilIndex() != DrawOrder::kUnassigned || captureParams.has_value());

    DrawParams drawParams(this->deduplicateTransform(localToDevice), geometry, clip, ordering,
                          stroke);

    fRenderStepCount += renderer->numRenderSteps();
    // Create a sort key for every render step in this draw
    for (int stepIndex = 0; stepIndex < renderer->numRenderSteps(); ++stepIndex) {
        const RenderStep* const step = renderer->steps()[stepIndex];
        // if (captureParams.has_value()) {
        //     auto bounds = drawParams.drawBounds();
        //     printf("DepthOnly %s %f %f %f %f\n", step->name(), bounds.left(), bounds.right(), bounds.top(), bounds.bot());
        // }

        gatherer->markOffsetAndAlign(step->performsShading(), step->uniformAlignment());

        GraphicsPipelineCache::Index pipelineIndex = fPipelineCache.insert(
                { step->renderStepID(), step->performsShading() ?
                                        paintID : UniquePaintParamsID::Invalid()});

        step->writeUniformsAndTextures(drawParams, gatherer);

        auto [combinedUniforms, combinedTextures] =
                gatherer->endCombinedData(step->performsShading());

        UniformDataCache::Index uniformIndex = combinedUniforms ?
                fUniformDataCache.insert(combinedUniforms) : UniformDataCache::kInvalidIndex;
        TextureDataCache::Index textureBindingIndex = combinedTextures ?
                fTextureDataCache.insert(combinedTextures) : TextureDataCache::kInvalidIndex;

        if (captureParams.has_value()) {
            this->recordDepthOnly(stepIndex, step, LayerKey{pipelineIndex, uniformIndex, textureBindingIndex},
                                  drawParams, barrierBeforeDraws, captureParams, layer);
        } else {
            if (layer.has_value() && layer->get()) { //Awkward, better signal here. Currently can pass nullptr when not a clipped draw.
                SkASSERT(layer->get());
                this->recordClipped(stepIndex, step,
                                    LayerKey{pipelineIndex, uniformIndex, textureBindingIndex},
                                    drawParams, dstUsage, barrierBeforeDraws, layer);
            } else {
                this->recordStep(stepIndex, step,
                                 LayerKey{pipelineIndex, uniformIndex, textureBindingIndex},
                                 drawParams, dstUsage, barrierBeforeDraws);
            }
        }
        gatherer->rewindForRenderStep();
    }

    fDrawCount++;
    fPassBounds.join(clip.drawBounds());
    fRequiresMSAA |= renderer->requiresMSAA();
    fDepthStencilFlags |= renderer->depthStencilFlags();
    if (dstUsage & DstUsage::kDstReadRequired) {
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

std::unique_ptr<DrawPass> DrawListLayer::snapDrawPass(Recorder* recorder,
                                                      sk_sp<TextureProxy> target,
                                                      const SkImageInfo& targetInfo,
                                                      const DstReadStrategy dstReadStrategy) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "draw count", fDrawCount);

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
    UniformTracker uniformTracker(useStorageBuffers);

    const bool rebindTexturesOnPipelineChange = dstReadStrategy == DstReadStrategy::kTextureCopy;
    CompressedPaintersOrder priorDrawPaintOrder {};

    // Accumulate rough pixel area touched by each pipeline
    drawPass->fPipelineDrawAreas.push_back_n(fPipelineCache.count(), 0.f);

    TextureTracker textureBindingTracker(&fTextureDataCache);

    // -----------------------------------------------------------------------------------------
    // Deduplicated Logic: "recordDraw"
    // -----------------------------------------------------------------------------------------
    // bindingsAreInvariant: If true, we assume Pipeline, Uniforms, and Textures match the
    //                       current state (optimization for BindingList tails).
    auto recordDraw = [&](const LayerKey& key,
                          const RenderStep* renderStep,
                          const DrawParams& drawParams,
                          BarrierType barrierBeforeDraws,
                          bool bindingsAreInvariant) -> bool {
        SkASSERT(renderStep);

        // 1. Calculate Binding Deltas (skipped if invariant)
        bool pipelineChange = false;
        bool uniformBindingChange = false;
        bool textureBindingsChange = false;

        if (!bindingsAreInvariant) {
            pipelineChange = key.fPipelineIndex != lastPipeline;

            uniformBindingChange = uniformTracker.writeUniforms(
                    fUniformDataCache, bufferMgr, key.fUniformIndex);

            textureBindingsChange =
                    textureBindingTracker.setCurrentTextureBindings(key.fTextureIndex) ||
                    (rebindTexturesOnPipelineChange && pipelineChange &&
                     key.fTextureIndex != TextureDataCache::kInvalidIndex);
        }

        drawPass->fPipelineDrawAreas[key.fPipelineIndex] += drawParams.drawBounds().area();

        std::optional<SkIRect> newScissor =
                renderStep->getScissor(drawParams, lastScissor, targetBounds);

        if (pipelineChange) {
            drawWriter.newPipelineState(renderStep->primitiveType(),
                                        renderStep->staticDataStride(),
                                        renderStep->appendDataStride(),
                                        renderStep->getRenderStateFlags(),
                                        barrierBeforeDraws);
        } else if (uniformBindingChange || textureBindingsChange || newScissor.has_value()) {
            drawWriter.newDynamicState();
        } else if (barrierBeforeDraws != BarrierType::kNone &&
                   priorDrawPaintOrder != drawParams.order().paintOrder()) {
            drawWriter.flush();
        }

        if (pipelineChange) {
            drawPass->fCommandList.bindGraphicsPipeline(key.fPipelineIndex);
            lastPipeline = key.fPipelineIndex;
        }
        if (uniformBindingChange) {
            uniformTracker.bindUniforms(UniformSlot::kCombinedUniforms, &drawPass->fCommandList);
        }
        if (textureBindingsChange) {
            textureBindingTracker.bindTextures(&drawPass->fCommandList);
        }
        if (newScissor.has_value()) {
            drawPass->fCommandList.setScissor(*newScissor);
            lastScissor = *newScissor;
        }

        uint32_t uniformSsboIndex = useStorageBuffers ? uniformTracker.ssboIndex() : 0;
        renderStep->writeVertices(&drawWriter, drawParams, uniformSsboIndex);

        if (bufferMgr->hasMappingFailed()) {
            SKGPU_LOG_W("Failed to write necessary vertex/instance data for DrawPass, dropping!");
            return false;
        }

        priorDrawPaintOrder = drawParams.order().paintOrder();
        return true;
    };

    for (const auto& layer : fLayers) {
        if (layer->fDepthOnlyDraws) {
            for (DeferredDraw* headDraw : *layer->fDepthOnlyDraws) {
                DeferredDraw* current = headDraw;
                while (current) {
                    if (!recordDraw(current->fKey, current->fStep, current->fDrawParams,
                                    current->fBarrierBeforeDraws, /*bindingsAreInvariant=*/false)) {
                        return nullptr;
                    }
                    current = current->fChildDraw;
                }
            }
        }

        // Non Depth only draws.
        for (const auto& bindingList : layer->fBindings) {
            const RecordedDraw* draw = bindingList->fDraws.head();
            SkASSERT(draw);
            if (!recordDraw(bindingList->fKey, draw->fStep, draw->fDrawParams,
                            draw->fBarrierBeforeDraws, /*bindingsAreInvariant=*/false)) {
                return nullptr;
            }

            draw = draw->fNext;
            while (draw) {
                if (!recordDraw(bindingList->fKey, draw->fStep, draw->fDrawParams,
                                draw->fBarrierBeforeDraws, /*bindingsAreInvariant=*/true)) {
                    return nullptr;
                }
                draw = draw->fNext;
            }
        }
    }

    drawWriter.flush();

    drawPass->fBounds = fPassBounds.roundOut().asSkIRect();
    drawPass->fPipelineDescs = fPipelineCache.detach();
    drawPass->fSampledTextures = fTextureDataCache.detachTextures();

    TRACE_COUNTER1("skia.gpu", "# pipelines", drawPass->fPipelineDescs.size());
    TRACE_COUNTER1("skia.gpu", "# textures", drawPass->fSampledTextures.size());
    TRACE_COUNTER1("skia.gpu", "# commands", drawPass->fCommandList.count());

    this->reset(LoadOp::kLoad);

    return drawPass;
}

}  // namespace skgpu::graphite
