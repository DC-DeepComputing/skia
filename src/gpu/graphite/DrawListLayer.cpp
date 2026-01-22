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

void DrawListLayer::recordStep(int stepIndex,  // Awkward, test utils only
                               bool dependsOnDst,
                               const RenderStep* step,
                               const LayerKey& key,
                               const DrawParams& drawParams,
                               const BarrierType& barrierBeforeDraws) {
    SkTInternalLList<Layer>::Iter iter;
    Layer* current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
    Layer* targetLayer = nullptr;
    BindingWrapper* targetMatch = nullptr;

    // If we're an easy draw, try the head first.
    if (!dependsOnDst) {
        targetLayer = fLayers.head();
        current = nullptr;

        if (targetLayer) {
            auto result = targetLayer->test(drawParams.drawBounds(), key, false);
            if (result != BoundsTest::kIncompatibleOverlap) {
                targetMatch = targetLayer->searchBinding(key);
            } else {
                targetLayer = nullptr;
                current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
            }
        }
    }

    int limit = kMaxSearchLimit;
    BindingWrapper* forwardMerge = nullptr;
    while (current && limit > 0) {
        BoundsTest result = current->test(drawParams.drawBounds(), key, false);

        if (result == BoundsTest::kIncompatibleOverlap) {
            // If we need to read the dst, we cannot go earlier than this layer.
            if (dependsOnDst) {
                if (current->fNext) {
                    targetLayer = current->fNext;

                    // We found our target layer, grab the binding if it exists
                    targetMatch = targetLayer->searchBinding(key);
                    break;
                } else {
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
    DrawParams* recordParams = fStorage.make<DrawParams>(drawParams);
    SingleDraw* draw = fStorage.make<SingleDraw>(step, recordParams, barrierBeforeDraws);
    targetLayer->add(&fStorage, targetMatch, key, draw, dependsOnDst, false);

#ifdef GPU_TEST_UTILS
    this->fLastLayer = targetLayer;
    if (fDrawObserver) {
        fDrawObserver(stepIndex, key, targetLayer->fLastDraw);
    }
#endif
}

void DrawListLayer::recordStencil(int stepIndex,
                                  bool dependsOnDst,
                                  const Renderer* renderer,
                                  const LayerKey& key,
                                  const DrawParams& drawParams,
                                  const BarrierType& barrierBeforeDraws) {
    // Child stencils get a fast path to their parent
    if (stepIndex > 0) {
        DrawParams* params = fStorage.make<DrawParams>(drawParams);
        ChainedDraw* draw = fStorage.make<ChainedDraw>(
            key, renderer->steps()[stepIndex], params, barrierBeforeDraws);
        SkASSERT(fLastRecordedDraw && !fLastRecordedDraw->fChildDraw);
        fLastRecordedDraw->fChildDraw = draw;
        fLastRecordedDraw = draw;
        return;
    }

    SkTInternalLList<Layer>::Iter iter;
    Layer* current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
    Layer* targetLayer = nullptr;
    BindingWrapper* targetMatch = nullptr;

    int limit = kMaxSearchLimit;
    BindingWrapper* forwardMerge = nullptr;
    while (current && limit > 0) {
        BoundsTest result = current->test(drawParams.drawBounds(), key, true);

        if (result == BoundsTest::kIncompatibleOverlap) {
            if (dependsOnDst) {
                if (current->fNext) {
                    targetLayer = current->fNext;
                    targetMatch = targetLayer->searchBinding(key);
                    break;
                } else {
                    break;
                }
            }
            // If !dependsOnDst, we just keep searching backwards.
        } else {
            // Only disjoints are allowed
            if (result == BoundsTest::kDisjoint) {
                targetLayer = current;
                // no match
                // targetMatch = current->searchBinding(key);
                break;
            }
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
    DrawParams* params = fStorage.make<DrawParams>(drawParams);
    ChainedDraw* draw = fStorage.make<ChainedDraw>(
            key, renderer->steps()[stepIndex], params, barrierBeforeDraws);
    targetLayer->add(&fStorage, targetMatch, key, draw, dependsOnDst, true);
    fLastRecordedDraw = draw;
}

void DrawListLayer::recordDepthOnly(
        int stepIndex,
        bool isStencil,
        bool dependsOnDst,
        const RenderStep* step,
        const LayerKey& key,
        const DrawParams& drawParams,
        const BarrierType& barrierBeforeDraws,
        std::optional<std::reference_wrapper<DrawParams*>> captureParams,
        std::optional<std::reference_wrapper<Layer*>> captureLayer) {
    if (stepIndex > 0) {
        SkASSERT(captureParams.has_value());
        ChainedDraw* deferredDraw =
                fStorage.make<ChainedDraw>(key, step, captureParams->get(), barrierBeforeDraws);
        SkASSERT(fLastDeferredDraw && !fLastDeferredDraw->fChildDraw);
        fLastDeferredDraw->fChildDraw = deferredDraw;
        fLastDeferredDraw = deferredDraw;
        return;
    }
    SkTInternalLList<Layer>::Iter iter;
    Layer* current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
    Layer* targetLayer = nullptr;

    int limit = kMaxSearchLimit;
    bool shouldOverwriteLayer = true;
    while (current && limit > 0) {
        BoundsTest result = current->depthOnlyTest(drawParams.drawBounds(), key, isStencil);
        if (captureLayer->get() == current) {
            shouldOverwriteLayer = false;
        }
        if (result == BoundsTest::kIncompatibleOverlap) {
            if (current->fNext) {
                targetLayer = current->fNext;
            } else {
                // disable forward merging
            }
            break;
        } else {
            targetLayer = current;
            if (result == BoundsTest::kCompatibleOverlap) {
                break;
            }
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

    DrawParams* deferredParams = fStorage.make<DrawParams>(drawParams);
    ChainedDraw* deferredDraw =
            fStorage.make<ChainedDraw>(key, step, deferredParams, barrierBeforeDraws);
    targetLayer->addDepthOnlyDraw(&fStorage, deferredDraw, isStencil);
    fLastDeferredDraw = deferredDraw;
    captureParams->get() = deferredParams;
    if (shouldOverwriteLayer) {
        captureLayer->get() = targetLayer;
    }
}

// Clipped draws start at the layer containing their depth only draw, then proceed FORWARDS.
void DrawListLayer::recordClipped(int stepIndex,
                                  bool isStencil,
                                  bool dependsOnDst,
                                  const RenderStep* step,
                                  const LayerKey& key,
                                  const DrawParams& drawParams,
                                  const BarrierType& barrierBeforeDraws,
                                  std::optional<std::reference_wrapper<Layer*>> startLayer) {
    // Child stencils get a fast path to their parent
    if (isStencil && stepIndex > 0) {
        DrawParams* params = fStorage.make<DrawParams>(drawParams);
        ChainedDraw* draw = fStorage.make<ChainedDraw>(key, step, params, barrierBeforeDraws);
        SkASSERT(fLastRecordedDraw && !fLastRecordedDraw->fChildDraw);
        fLastRecordedDraw->fChildDraw = draw;
        fLastRecordedDraw = draw;
        return;
    }

    Layer* current = startLayer->get();
    Layer* targetLayer = nullptr;
    BindingWrapper* targetMatch = nullptr;

    int limit = kMaxSearchLimit;
    while (current && limit > 0) {
        BoundsTest result = current->test(drawParams.drawBounds(), key, isStencil);
        if (result != BoundsTest::kIncompatibleOverlap) {
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
    DrawParams* recordParams = fStorage.make<DrawParams>(drawParams);
    if (isStencil) {
        ChainedDraw* draw = fStorage.make<ChainedDraw>(key, step, recordParams, barrierBeforeDraws);
        targetLayer->add(&fStorage, targetMatch, key, draw, dependsOnDst, isStencil);
        fLastRecordedDraw = draw;
    } else {
        SingleDraw* draw = fStorage.make<SingleDraw>(step, recordParams, barrierBeforeDraws);
        targetLayer->add(&fStorage, targetMatch, key, draw, dependsOnDst, isStencil);
    }
    startLayer->get() = targetLayer;
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

    // RENDERER not STEP because ATOMIC
    bool isStencil = renderer->depthStencilFlags() != DepthStencilFlags::kNone;
    bool dependsOnDst = dstUsage != DstUsage::kNone;

    DrawParams drawParams(
            this->deduplicateTransform(localToDevice), geometry, clip, ordering, stroke);

    fRenderStepCount += renderer->numRenderSteps();
    // Create a sort key for every render step in this draw
    for (int stepIndex = 0; stepIndex < renderer->numRenderSteps(); ++stepIndex) {
        const RenderStep* const step = renderer->steps()[stepIndex];

        gatherer->markOffsetAndAlign(step->performsShading(), step->uniformAlignment());

        GraphicsPipelineCache::Index pipelineIndex = fPipelineCache.insert(
                {step->renderStepID(),
                 step->performsShading() ? paintID : UniquePaintParamsID::Invalid()});

        step->writeUniformsAndTextures(drawParams, gatherer);

        auto [combinedUniforms, combinedTextures] =
                gatherer->endCombinedData(step->performsShading());

        UniformDataCache::Index uniformIndex = combinedUniforms
                                                       ? fUniformDataCache.insert(combinedUniforms)
                                                       : UniformDataCache::kInvalidIndex;
        TextureDataCache::Index textureBindingIndex =
                combinedTextures ? fTextureDataCache.insert(combinedTextures)
                                 : TextureDataCache::kInvalidIndex;

        if (captureParams.has_value()) {
            // TODO depthStencil?
            this->recordDepthOnly(stepIndex,
                                  isStencil,
                                  dependsOnDst,
                                  step,
                                  LayerKey{pipelineIndex, uniformIndex, textureBindingIndex},
                                  drawParams,
                                  barrierBeforeDraws,
                                  captureParams,
                                  layer);
        } else {
            if (layer.has_value() && layer->get()) {  // Awkward, better signal here. Currently can
                                                      // pass nullptr when not a clipped draw.
                SkASSERT(layer->get());
                this->recordClipped(stepIndex,
                                    isStencil,
                                    dependsOnDst,
                                    step,
                                    LayerKey{pipelineIndex, uniformIndex, textureBindingIndex},
                                    drawParams,
                                    barrierBeforeDraws,
                                    layer);
            } else {
                if (isStencil) {
                    this->recordStencil(stepIndex,
                                        dependsOnDst,
                                        renderer,
                                        LayerKey{pipelineIndex, uniformIndex, textureBindingIndex},
                                        drawParams,
                                        barrierBeforeDraws);
                } else {
                    this->recordStep(stepIndex,
                                     dependsOnDst,
                                     step,
                                     LayerKey{pipelineIndex, uniformIndex, textureBindingIndex},
                                     drawParams,
                                     barrierBeforeDraws);
                }
            }
        }

        // printf("\n\n\nSTEP %d %s\n", stepIndex, isStencil ? "yes" : "no");
        // for (const auto& l : fLayers) {
        //     // printf("New Layer\n");
        //     if (l->fDepthOnlyDraws) {
        //         printf("\n");
        //         for (ChainedDraw* headDraw : *l->fDepthOnlyDraws) {
        //             ChainedDraw* current = headDraw;
        //             while (current) {
        //                 printf("Depth Only: ");
        //                 Rect bounds = current->fDrawParams->drawBounds();
        //                 printf("%u %s: %f %f %f %f\n",
        //                        current->fDrawParams->order().depth().bits(),
        //                        current->fStep->name(),
        //                        bounds.left(),
        //                        bounds.right(),
        //                        bounds.top(),
        //                        bounds.bot());
        //                 current = current->fChildDraw;
        //             }
        //         }
        //     }

        //     for (const BindingWrapper* wrapper : l->fBindings) {
        //         if (wrapper->fType == BindingListType::kSingle) {
        //             const auto* list = static_cast<const BindingList<SingleDraw>*>(wrapper);
        //             const SingleDraw* draw = list->fDraws.head();

        //             if (draw) {
        //                 printf("Start : ");
        //                 Rect b = draw->fDrawParams->drawBounds();  // segfauulting here
        //                 printf("%u %s: %f %f %f %f\n",
        //                        draw->fDrawParams->order().depth().bits(),
        //                        draw->fStep->name(),
        //                        b.left(),
        //                        b.right(),
        //                        b.top(),
        //                        b.bot());
        //                 draw = draw->fNext;
        //             }

        //             while (draw) {
        //                 printf("Middle: ");
        //                 Rect bb = draw->fDrawParams->drawBounds();
        //                 printf("%u %s: %f %f %f %f\n",
        //                        draw->fDrawParams->order().depth().bits(),
        //                        draw->fStep->name(),
        //                        bb.left(),
        //                        bb.right(),
        //                        bb.top(),
        //                        bb.bot());
        //                 draw = draw->fNext;
        //             }
        //         } else {
        //             const auto* list = static_cast<const BindingList<ChainedDraw>*>(wrapper);
        //             for (const ChainedDraw* head : list->fDraws) {
        //                 const ChainedDraw* current = head;
        //                 while (current) {
        //                     printf("Chained: ");
        //                     Rect b = current->fDrawParams->drawBounds();
        //                     printf("%u %s: %f %f %f %f\n",
        //                            current->fDrawParams->order().depth().bits(),
        //                            current->fStep->name(),
        //                            b.left(),
        //                            b.right(),
        //                            b.top(),
        //                            b.bot());
        //                     current = current->fChildDraw;
        //                 }
        //             }
        //         }
        //     }
        // }
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

    std::unique_ptr<DrawPass> drawPass(new DrawPass(target,
                                                    {fLoadOp, StoreOp::kStore},
                                                    fClearColor,
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
    CompressedPaintersOrder priorDrawPaintOrder{};

    // Accumulate rough pixel area touched by each pipeline
    drawPass->fPipelineDrawAreas.push_back_n(fPipelineCache.count(), 0.f);

    TextureTracker textureBindingTracker(&fTextureDataCache);

    // printf("\n\n\nSNAPPING\n\n\n");

    auto recordDraw = [&](const LayerKey& key,
                          const RenderStep* renderStep,
                          const DrawParams& drawParams,
                          BarrierType barrierBeforeDraws,
                          bool bindingsAreInvariant) -> bool {
        SkASSERT(renderStep);

        bool pipelineChange = false;
        bool uniformBindingChange = false;
        bool textureBindingsChange = false;

        if (!bindingsAreInvariant) {
            pipelineChange = key.fPipelineIndex != lastPipeline;

            uniformBindingChange =
                    uniformTracker.writeUniforms(fUniformDataCache, bufferMgr, key.fUniformIndex);

            textureBindingsChange =
                    textureBindingTracker.setCurrentTextureBindings(key.fTextureIndex) ||
                    (rebindTexturesOnPipelineChange && pipelineChange &&
                     key.fTextureIndex != TextureDataCache::kInvalidIndex);
        }

        drawPass->fPipelineDrawAreas[key.fPipelineIndex] += drawParams.drawBounds().area();

        std::optional<SkIRect> newScissor =
                renderStep->getScissor(drawParams, lastScissor, targetBounds);

        // auto bounds = drawParams.drawBounds();
        // printf("%u %s %s: %f %f %f %f\n",
        //        drawParams.order().depth().bits(),
        //        renderStep->name(),
        //        newScissor.has_value() ? "Scissor" : "No Scissor",
        //        bounds.left(),
        //        bounds.right(),
        //        bounds.top(),
        //        bounds.bot());
        // if (newScissor.has_value()) {
        //     printf("   -%d %d %d %d\n",
        //            newScissor.value().left(),
        //            newScissor.value().right(),
        //            newScissor.value().top(),
        //            newScissor.value().bottom());
        // }

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
            // printf("\n");
            for (ChainedDraw* headDraw : *layer->fDepthOnlyDraws) {
                ChainedDraw* current = headDraw;
                while (current) {
                    // printf("Depth Only: ");
                    if (!recordDraw(current->fKey,
                                    current->fStep,
                                    *current->fDrawParams,
                                    current->fBarrierBeforeDraws,
                                    false)) {
                        return nullptr;
                    }
                    current = current->fChildDraw;
                }
            }
        }

        for (const BindingWrapper* binding : layer->fBindings) {
            if (binding->fType == BindingListType::kSingle) {
                const auto* singleList = static_cast<const BindingList<SingleDraw>*>(binding);
                for (const SingleDraw* draw : singleList->fDraws) {
                    // printf("Single: ");
                    if (!recordDraw(binding->fKey,
                                    draw->fStep,
                                    *draw->fDrawParams,
                                    draw->fBarrierBeforeDraws,
                                    false)) {
                        return nullptr;
                    }
                }
            } else {
                const auto* chainedList = static_cast<const BindingList<ChainedDraw>*>(binding);
                for (const ChainedDraw* headDraw : chainedList->fDraws) {
                    const ChainedDraw* current = headDraw;
                    while (current) {
                        // printf("Chained: ");
                        if (!recordDraw(current->fKey,
                                        current->fStep,
                                        *current->fDrawParams,
                                        current->fBarrierBeforeDraws,
                                        false)) {
                            return nullptr;
                        }
                        current = current->fChildDraw;
                    }
                }
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
