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

DrawListLayer::BindingList* DrawListLayer::Layer::searchBinding(const LayerKey& key) {
    for (BindingList* list : fBindings) {
        if (list->fKey == key) {
            return list;
        }
    }
    return nullptr;
}

BoundsTest DrawListLayer::Layer::test(const Rect& drawBounds,
                                      const LayerKey& key,
                                      bool disjointStencil) {
    if (!disjointStencil && fBindings.head() &&
        fBindings.head() == fBindings.tail() && fBindings.head()->fKey == key) {
        return BoundsTest::kCompatibleOverlap;
    }

    bool foundMatch = false;

    for (BindingList* list : fBindings) {
        if (list->fKey == key) {
            foundMatch = true;
            // If we aren't disjoint stencil, we can overlap with ourselves freely, so we skip the
            // bounds check for this list.
            if (!disjointStencil) continue;
        }

        if (list->fBounds.intersects(drawBounds)) {
            for (const RecordedDraw* d : list->fDraws) {
                if (d->fDrawParams.drawBounds().intersects(drawBounds)) {
                    return BoundsTest::kIncompatibleOverlap;
                }
            }
        }
    }

    return foundMatch ? BoundsTest::kCompatibleOverlap : BoundsTest::kDisjoint;
}

void DrawListLayer::Layer::add(SkArenaAllocWithReset* alloc,
                               BindingList* match,
                               const LayerKey& key,
                               RecordedDraw* draw,
                               bool dependsOnDst) {
    if (match) {
        if (!dependsOnDst) {
            match->fDraws.addToHead(draw);
        } else {
            match->fDraws.addToTail(draw);
        }
        match->fBounds.join(draw->fDrawParams.drawBounds());
    } else {
        BindingList* list = alloc->make<BindingList>();
        list->fKey = key;
        list->fBounds = draw->fDrawParams.drawBounds();
        list->fDraws.addToHead(draw);
        fBindings.addToTail(list);
    }

#ifdef GPU_TEST_UTILS
    this->fLastDraw = draw;
    this->fLastKey = key;
#endif
}

void DrawListLayer::Layer::addDepthOnlyDraw(SkArenaAllocWithReset* alloc,
                                            DeferredDraw* deferredDraw) {
    if (!fDepthOnlyDraws) {
        fDepthOnlyDraws = alloc->make<skia_private::TArray<DeferredDraw*>>();
    }
    fDepthOnlyDraws->push_back(deferredDraw);
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

    // If we didn't find a match during the search, check the target layer one last time (In case we
    // settled on a 'Disjoint' layer that actually had the binding but we didn't fetch it yet)
    if (!targetMatch && targetLayer->test(drawParams.drawBounds(), key, disjointStencil) ==
                                BoundsTest::kCompatibleOverlap) {
        targetMatch = targetLayer->searchBinding(key);
    }

    RecordedDraw* draw = fStorage.make<RecordedDraw>(step, drawParams, barrierBeforeDraws);
    targetLayer->add(&fStorage, targetMatch, key, draw, dependsOnDst);

#ifdef GPU_TEST_UTILS
    this->fLastLayer = targetLayer;
    if (fDrawObserver) {
        fDrawObserver(stepIndex, key, targetLayer->fLastDraw);
    }
#endif
}

// Depth only draws rely on "magic." Because we need depth only draws to occur *before* the shading
// draw, we record them immediately when visiting the clip stack. However, at this point, we only
// have the conservative draw bounds. The conservatives bounds are sufficient to find the correct
// layer to add to, but we have backpointer so that the tighter bounds can be added later.
//
// The main part of the magical behavior is that, because the depth only draw and the succeeding
// cover draw have the exact same bounding, we can defer updating the bounds when recording the
// depth only draw so that the cover draws ends up in the exact same layer or layer. This relies on
// another magic property which is that identical which differ only by at most dependsOnDst, will
// end in the same layer.
//
// TODO: Try also saving a backpointer to the layer that took the depth only draw, so that layer
// searching can be skipped for the cover draw
void DrawListLayer::recordDepthOnly(
        const RenderStep* step,
        const LayerKey& key,
        const DrawParams& drawParams,
        const BarrierType& barrierBeforeDraws,
        std::optional<std::reference_wrapper<DrawParams*>> captureParams) {
    SkTInternalLList<Layer>::Iter iter;
    Layer* current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
    Layer* targetLayer = nullptr;

    bool disjointStencil =
            (step->depthStencilFlags() & DepthStencilFlags::kStencil) != DepthStencilFlags::kNone;

    int limit = kMaxSearchLimit;
    while (current && limit > 0) {
        BoundsTest result = current->test(drawParams.drawBounds(), key, disjointStencil);
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
        } else {
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
    }
    SkASSERT(targetLayer);

    DeferredDraw* deferredDraw = fStorage.make<DeferredDraw>(key, step, drawParams,
                                                             barrierBeforeDraws, captureParams);
    targetLayer->addDepthOnlyDraw(&fStorage, deferredDraw);
}

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
                               std::optional<std::reference_wrapper<DrawParams*>> captureParams) {
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
            this->recordDepthOnly(step, LayerKey{pipelineIndex, uniformIndex, textureBindingIndex},
                                  drawParams, barrierBeforeDraws, captureParams);
        } else {
            this->recordStep(stepIndex, step,
                             LayerKey{pipelineIndex, uniformIndex, textureBindingIndex},
                             drawParams, dstUsage, barrierBeforeDraws);
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
    UniformTracker uniformTracker(useStorageBuffers);

    // TODO(b/372953722): Remove this forced binding command behavior once dst copies are always
    // bound separately from the rest of the textures.
    const bool rebindTexturesOnPipelineChange = dstReadStrategy == DstReadStrategy::kTextureCopy;
    CompressedPaintersOrder priorDrawPaintOrder {};

    // Accumulate rough pixel area touched by each pipeline as we iterate the SortKeys
    drawPass->fPipelineDrawAreas.push_back_n(fPipelineCache.count(), 0.f);

    TextureTracker textureBindingTracker(&fTextureDataCache);
    for (const auto& layer : fLayers) {
        // If the layer has depth only draws, then we must record them!
        if (layer->fDepthOnlyDraws) {
            for (auto& depthDraw : *layer->fDepthOnlyDraws) {
                const bool pipelineChange = depthDraw->fKey.fPipelineIndex != lastPipeline;
                const bool uniformBindingChange = uniformTracker.writeUniforms(
                        fUniformDataCache, bufferMgr, depthDraw->fKey.fUniformIndex);
                const bool textureBindingsChange =
                        textureBindingTracker.setCurrentTextureBindings(
                                depthDraw->fKey.fTextureIndex) ||
                        (rebindTexturesOnPipelineChange && pipelineChange &&
                         depthDraw->fKey.fTextureIndex != TextureDataCache::kInvalidIndex);

                const RenderStep* renderStep = depthDraw->fStep;
                SkASSERT(renderStep);

                drawPass->fPipelineDrawAreas[depthDraw->fKey.fPipelineIndex] +=
                        depthDraw->fDrawParams.drawBounds().area();

                std::optional<SkIRect> newScissor =
                        renderStep->getScissor(depthDraw->fDrawParams, lastScissor, targetBounds);

                if (pipelineChange) {
                    drawWriter.newPipelineState(renderStep->primitiveType(),
                                                renderStep->staticDataStride(),
                                                renderStep->appendDataStride(),
                                                renderStep->getRenderStateFlags(),
                                                depthDraw->fBarrierBeforeDraws);
                } else if (uniformBindingChange || textureBindingsChange ||
                           newScissor.has_value()) {
                    drawWriter.newDynamicState();
                } else if (depthDraw->fBarrierBeforeDraws != BarrierType::kNone &&
                           priorDrawPaintOrder != depthDraw->fDrawParams.order().paintOrder()) {
                    drawWriter.flush();
                }
                if (pipelineChange) {
                    drawPass->fCommandList.bindGraphicsPipeline(depthDraw->fKey.fPipelineIndex);
                    lastPipeline = depthDraw->fKey.fPipelineIndex;
                }
                if (uniformBindingChange) {
                    uniformTracker.bindUniforms(UniformSlot::kCombinedUniforms,
                                                &drawPass->fCommandList);
                }
                if (textureBindingsChange) {
                    textureBindingTracker.bindTextures(&drawPass->fCommandList);
                }
                if (newScissor.has_value()) {
                    drawPass->fCommandList.setScissor(*newScissor);
                    lastScissor = *newScissor;
                }

                uint32_t uniformSsboIndex = useStorageBuffers ? uniformTracker.ssboIndex() : 0;
                renderStep->writeVertices(&drawWriter, depthDraw->fDrawParams, uniformSsboIndex);

                if (bufferMgr->hasMappingFailed()) {
                    SKGPU_LOG_W(
                            "Failed to write necessary vertex/instance data for DrawPass, "
                            "dropping!");
                    return nullptr;
                }

                priorDrawPaintOrder = depthDraw->fDrawParams.order().paintOrder();
            }
        }

        for (const auto& bindingList : layer->fBindings) {
            const bool pipelineChange = bindingList->fKey.fPipelineIndex != lastPipeline;
            const bool uniformBindingChange = uniformTracker.writeUniforms(
                    fUniformDataCache, bufferMgr, bindingList->fKey.fUniformIndex);
            // TODO(b/372953722): The Dawn and Vulkan CommandBuffer implementations currently
            // append any dst copy to the texture bind group/descriptor set automatically when
            // processing a BindTexturesAndSamplers call because they use a single group to
            // contain all textures. However, from the DrawPass POV, we can run into the
            // scenario where two pipelines have the same textures+samplers except one requires
            // a dst-copy and the other does not. In this case we wouldn't necessarily insert a
            // new command when the pipeline changed and then end up with layout validation
            // errors.
            const bool textureBindingsChange =
                    textureBindingTracker.setCurrentTextureBindings(
                            bindingList->fKey.fTextureIndex) ||
                    (rebindTexturesOnPipelineChange && pipelineChange &&
                     bindingList->fKey.fTextureIndex != TextureDataCache::kInvalidIndex);

            // The first draw inside the binding list checks per BindingList state changes
            const DrawListLayer::RecordedDraw* draw = bindingList->fDraws.head();
            {
                const RenderStep* renderStep = draw->fStep;
                SkASSERT(renderStep);

                drawPass->fPipelineDrawAreas[bindingList->fKey.fPipelineIndex] +=
                        draw->fDrawParams.drawBounds().area();

                std::optional<SkIRect> newScissor =
                        renderStep->getScissor(draw->fDrawParams, lastScissor, targetBounds);

                // Update DrawWriter *before* we actually change any state so that accumulated
                // draws from the previous state use the proper state.
                if (pipelineChange) {
                    drawWriter.newPipelineState(renderStep->primitiveType(),
                                                renderStep->staticDataStride(),
                                                renderStep->appendDataStride(),
                                                renderStep->getRenderStateFlags(),
                                                draw->fBarrierBeforeDraws);
                } else if (uniformBindingChange || textureBindingsChange ||
                           newScissor.has_value()) {
                    drawWriter.newDynamicState();
                } else if (draw->fBarrierBeforeDraws != BarrierType::kNone &&
                           priorDrawPaintOrder != draw->fDrawParams.order().paintOrder()) {
                    // TODO can be removed?
                    //
                    // Even if there is no pipeline or state change, we must consider whether a
                    // DrawPassCommand to add barriers must be inserted before any draw
                    // commands. If so, then determine if the current and prior draws overlap
                    // (ie, their PaintOrders are unequal). If so, perform a flush() to make
                    // sure the draw and add barrier commands are appended to the command list
                    // in the proper order.
                    drawWriter.flush();
                }
                if (pipelineChange) {
                    drawPass->fCommandList.bindGraphicsPipeline(bindingList->fKey.fPipelineIndex);
                    lastPipeline = bindingList->fKey.fPipelineIndex;
                }
                if (uniformBindingChange) {
                    uniformTracker.bindUniforms(UniformSlot::kCombinedUniforms,
                                                &drawPass->fCommandList);
                }
                if (textureBindingsChange) {
                    textureBindingTracker.bindTextures(&drawPass->fCommandList);
                }
                if (newScissor.has_value()) {
                    drawPass->fCommandList.setScissor(*newScissor);
                    lastScissor = *newScissor;
                }

                uint32_t uniformSsboIndex = useStorageBuffers ? uniformTracker.ssboIndex() : 0;
                renderStep->writeVertices(&drawWriter, draw->fDrawParams, uniformSsboIndex);

                if (bufferMgr->hasMappingFailed()) {
                    SKGPU_LOG_W(
                            "Failed to write necessary vertex/instance data for DrawPass, "
                            "dropping!");
                    return nullptr;
                }

                priorDrawPaintOrder = draw->fDrawParams.order().paintOrder();
                draw = draw->fNext;
            }

            // Our binding list guarantees that all other attributes are invariant,
            // so only check scissor
            while (draw) {
                const RenderStep* renderStep = draw->fStep;
                SkASSERT(renderStep);

                drawPass->fPipelineDrawAreas[bindingList->fKey.fPipelineIndex] +=
                        draw->fDrawParams.drawBounds().area();

                std::optional<SkIRect> newScissor =
                        renderStep->getScissor(draw->fDrawParams, lastScissor, targetBounds);

                if (newScissor.has_value()) {
                    drawWriter.newDynamicState();
                } else if (draw->fBarrierBeforeDraws != BarrierType::kNone &&
                           priorDrawPaintOrder != draw->fDrawParams.order().paintOrder()) {
                    // TODO can be removed?
                    drawWriter.flush();
                }
                if (newScissor.has_value()) {
                    drawPass->fCommandList.setScissor(*newScissor);
                    lastScissor = *newScissor;
                }

                uint32_t uniformSsboIndex = useStorageBuffers ? uniformTracker.ssboIndex() : 0;
                renderStep->writeVertices(&drawWriter, draw->fDrawParams, uniformSsboIndex);

                if (bufferMgr->hasMappingFailed()) {
                    SKGPU_LOG_W(
                            "Failed to write necessary vertex/instance data for DrawPass, "
                            "dropping!");
                    return nullptr;
                }

                priorDrawPaintOrder = draw->fDrawParams.order().paintOrder();
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
