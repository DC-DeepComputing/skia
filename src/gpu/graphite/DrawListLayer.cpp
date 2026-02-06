/*
 * Copyright 2026 Google LLC
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

void DrawListLayer::recordBackwards(int stepIndex,
                                    bool isStencil,
                                    bool dependsOnDst,
                                    const RenderStep* step,
                                    const UniformDataCache::Index& uniformIndex,
                                    const LayerKey& key,
                                    const DrawParams* drawParams,
                                    const Layer* stopLayer) {
    // Child stencils get a fast path to their parent
    if (isStencil) {
        if (stepIndex > 0) {
            ChainedDraw* draw = fStorage.make<ChainedDraw>(key, step, drawParams, uniformIndex);
            SkASSERT(fLastRecordedDraw && !fLastRecordedDraw->fChildDraw);
            fLastRecordedDraw->fChildDraw = draw;
            fLastRecordedDraw = draw;
            return;
        }
    }

    SkTInternalLList<Layer>::Iter iter;
    Layer* targetLayer = nullptr;
    BindingWrapper* targetMatch = nullptr;

    // If we're an easy draw (!kIsStencil and !dependsOnDst), try the head first.
    Layer* current;
    if (!isStencil && !dependsOnDst) {
        targetLayer = fLayers.head();
        if (targetLayer) {
            targetMatch = targetLayer->searchBinding(key);
        }
        current = const_cast<Layer*>(stopLayer);
    } else {
        current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
    }

    int limit = kMaxSearchLimit;
    // When stopLayer == nullptr this is effectively while(current)
    while (current != stopLayer && limit > 0) {
#if defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(current->fPrev);
#endif
        BoundsTest result = current->test(drawParams->drawBounds(), key, isStencil);

        if (result == BoundsTest::kIncompatibleOverlap) {
            // If we need to read the dst, we cannot go earlier than this layer.
            if (dependsOnDst) {
                if (current->fNext) {
                    targetLayer = current->fNext;

                    // We found our target layer, grab the binding if it exists
                    targetMatch = targetLayer->searchBinding(key);
                    break;
                } else {
                    // TODO (thomsmit): Test performance of forward merging
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
    }
    SkASSERT(targetLayer);

    if (isStencil) {
        ChainedDraw* draw = fStorage.make<ChainedDraw>(key, step, drawParams, uniformIndex);
        targetLayer->add(&fStorage, targetMatch, key, draw, step, dependsOnDst, true);
        fLastRecordedDraw = draw;
    } else {
        SingleDraw* draw = fStorage.make<SingleDraw>(drawParams, uniformIndex);
        targetLayer->add(&fStorage, targetMatch, key, draw, step, dependsOnDst, false);
    }
}

void DrawListLayer::recordDepthOnly(
        int stepIndex,
        bool isStencil,
        bool dependsOnDst,
        const RenderStep* step,
        const UniformDataCache::Index& uniformIndex,
        const LayerKey& key,
        const DrawParams* drawParams,
        Layer** captureLayer) {
    if (stepIndex > 0) {
        ChainedDraw* deferredDraw = fStorage.make<ChainedDraw>(key, step, drawParams, uniformIndex);
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
#if defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(current->fPrev);
#endif
        BoundsTest result = current->depthOnlyTest(drawParams->drawBounds(), key, isStencil);
        if (*captureLayer == current) {
            shouldOverwriteLayer = false;
        }
        if (result == BoundsTest::kIncompatibleOverlap) {
            if (current->fNext) {
                targetLayer = current->fNext;
            } else {
                // TODO (thomsmit): Test performance of forward merging
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

    ChainedDraw* deferredDraw = fStorage.make<ChainedDraw>(key, step, drawParams, uniformIndex);
    targetLayer->addDepthOnlyDraw(&fStorage, deferredDraw, isStencil);
    fLastDeferredDraw = deferredDraw;
    if (shouldOverwriteLayer) {
        *captureLayer = targetLayer;
    }
}

// Clipped draws must come *after* the depth only draw, and they must come *after* any preceding
// draws from the same renderstep. To accomodate this:
//      1) Depth only draws store a pointer to the layer they inserted into.
//      2) Normal draws that are afffected by the clip---"clipped draws"---use the layer pointer
//         as the start of the traversal, then proceed FORWARDS until finding a suitable layer.
//      3) Each clipped draw updates the starting layer *after* the layer that *it* inserted into,
//         thus preserving the relative ordering between the draws. This is necessary because
//         bindinglists within a layer have no ordering guarantees, and it is possible to insert
//         into a matching bindinglist that will be drawn PRIOR to a preceding clipped draw.
void DrawListLayer::recordClipped(int stepIndex,
                                  bool isStencil,
                                  bool dependsOnDst,
                                  const RenderStep* step,
                                  const UniformDataCache::Index& uniformIndex,
                                  const LayerKey& key,
                                  const DrawParams* drawParams,
                                  Layer** startLayer) {
    // Child stencils get a fast path to their parent
    if (isStencil && stepIndex > 0) {
        ChainedDraw* draw = fStorage.make<ChainedDraw>(key, step, drawParams, uniformIndex);
        SkASSERT(fLastRecordedDraw && !fLastRecordedDraw->fChildDraw);
        fLastRecordedDraw->fChildDraw = draw;
        fLastRecordedDraw = draw;
        return;
    }

    Layer* current = *startLayer;
    Layer* targetLayer = nullptr;
    BindingWrapper* targetMatch = nullptr;

    int limit = kMaxSearchLimit;
    while (current && limit > 0) {
#if defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(current->fNext);
#endif
        BoundsTest result = current->test(drawParams->drawBounds(), key, isStencil);
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
    if (isStencil) {
        ChainedDraw* draw = fStorage.make<ChainedDraw>(key, step, drawParams, uniformIndex);
        targetLayer->add(&fStorage, targetMatch, key, draw, step, dependsOnDst, isStencil);
        fLastRecordedDraw = draw;
    } else {
        SingleDraw* draw = fStorage.make<SingleDraw>(drawParams, uniformIndex);
        targetLayer->add(&fStorage, targetMatch, key, draw, step, dependsOnDst, isStencil);
    }
    *startLayer = targetLayer; //->fNext is strictly necessary for correctness. Currently does not break anything
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
                               DrawParams** captureParams,
                               Layer** captureLayer) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!geometry.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());
    // Remove now that the stencil interactions are disabled in device
    // SkASSERT(!(renderer->depthStencilFlags() & DepthStencilFlags::kStencil) ||
    //          ordering.stencilIndex() != DrawOrder::kUnassigned || captureParams);

    auto startRecordDraw = std::chrono::high_resolution_clock::now();

    // RENDERER not STEP because ATOMIC
    bool isStencil = (renderer->depthStencilFlags() & DepthStencilFlags::kStencil) !=
                     DepthStencilFlags::kNone;
    bool dependsOnDst = (dstUsage & DstUsage::kDependsOnDst) != DstUsage::kNone;

    // In this iteration, the draw params are created once per record draw call, and the pointer is
    // passed to each draw call. This is storage effecient but will still introduce some pointer
    // chasing, because the params will likely no longer be on the same cache line for successor
    // render steps. We should test whether it is faster for each step to hold a copy of the params
    // except in the case of clipped draws (which must share a copy because they are mutated later).
    DrawParams* drawParams = fStorage.make<DrawParams>(this->deduplicateTransform(localToDevice),
                                                       geometry, clip, ordering, stroke, barrierBeforeDraws);


    fRenderStepCount += renderer->numRenderSteps();
    for (int stepIndex = 0; stepIndex < renderer->numRenderSteps(); ++stepIndex) {
        const RenderStep* const step = renderer->steps()[stepIndex];

        gatherer->markOffsetAndAlign(step->performsShading(), step->uniformAlignment());

        GraphicsPipelineCache::Index pipelineIndex = fPipelineCache.insert(
                {step->renderStepID(),
                 step->performsShading() ? paintID : UniquePaintParamsID::Invalid()});

        step->writeUniformsAndTextures(*drawParams, gatherer);

        auto [combinedUniforms, combinedTextures] =
                gatherer->endCombinedData(step->performsShading());

        UniformDataCache::Index uniformIndex = combinedUniforms
                                                       ? fUniformDataCache.insert(combinedUniforms)
                                                       : UniformDataCache::kInvalidIndex;
        TextureDataCache::Index textureBindingIndex =
                combinedTextures ? fTextureDataCache.insert(combinedTextures)
                                 : TextureDataCache::kInvalidIndex;

        if (captureParams) {
            this->recordDepthOnly(stepIndex,
                                  isStencil,
                                  dependsOnDst,
                                  step,
                                  uniformIndex,
                                  LayerKey{pipelineIndex, textureBindingIndex},
                                  drawParams,
                                  captureLayer);
            *captureParams = drawParams;
        } else {
            if (captureLayer) {
                // Note it is possible to have a nullptr at this point.
                if (!dependsOnDst) {
                    this->recordClipped(stepIndex,
                                        isStencil,
                                        false,
                                        step,
                                        uniformIndex,
                                        LayerKey{pipelineIndex, textureBindingIndex},
                                        drawParams,
                                        captureLayer);
                } else {
                    this->recordBackwards(stepIndex,
                                          isStencil,
                                          dependsOnDst,
                                          step,
                                          uniformIndex,
                                          LayerKey{pipelineIndex, textureBindingIndex},
                                          drawParams,
                                          *captureLayer);
                }
            } else {
                this->recordBackwards(stepIndex,
                                      isStencil,
                                      dependsOnDst,
                                      step,
                                      uniformIndex,
                                      LayerKey{pipelineIndex, textureBindingIndex},
                                      drawParams,
                                      nullptr);
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

    auto endRecordDraw = std::chrono::high_resolution_clock::now();
    fTotalRecordDrawTime += std::chrono::duration<double, std::milli>(endRecordDraw - startRecordDraw).count();
}

std::unique_ptr<DrawPass> DrawListLayer::snapDrawPass(Recorder* recorder,
                                                      sk_sp<TextureProxy> target,
                                                      const SkImageInfo& targetInfo,
                                                      const DstReadStrategy dstReadStrategy) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "draw count", fDrawCount);

    auto startSnapPass = std::chrono::high_resolution_clock::now();

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

    auto recordDraw = [&](const LayerKey& key,
                          const UniformDataCache::Index uniformIndex,
                          const RenderStep* renderStep,
                          const DrawParams& drawParams,
                          bool bindingsAreInvariant) -> bool {
        SkASSERT(renderStep);

        bool pipelineChange = false;
        bool uniformBindingChange = false;
        bool textureBindingsChange = false;

        if (!bindingsAreInvariant) {
            pipelineChange = key.fPipelineIndex != lastPipeline;

            uniformBindingChange =
                    uniformTracker.writeUniforms(fUniformDataCache, bufferMgr, uniformIndex);

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
                                        drawParams.barrierBeforeDraws());
            fPipelineChanges++;
        } else if (uniformBindingChange || textureBindingsChange || newScissor.has_value()) {
            drawWriter.newDynamicState();
        } else if (drawParams.barrierBeforeDraws() != BarrierType::kNone &&
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

    for (Layer* layer : fLayers) {
        if (DepthDrawInfo* depthInfo = layer->fDepthInfo) {
            for (ChainedDraw* headDraw : *depthInfo->fDraws) {
                ChainedDraw* current = headDraw;
                do {
                    if (!recordDraw(current->fKey,
                                    current->fUniformIndex,
                                    current->fStep,
                                    *current->fDrawParams,
                                    false)) {
                        return nullptr;
                    }
                    current = current->fChildDraw;
                } while (SK_UNLIKELY(current));
            }
        }

        for (const BindingWrapper* binding : layer->fBindings) {
            if (binding->fType == BindingListType::kSingle) {
                const auto* singleList = static_cast<const BindingList<SingleDraw>*>(binding);
                for (const SingleDraw* draw : singleList->fDraws) {
                    if (!recordDraw(binding->fKey,
                                    draw->fUniformIndex,
                                    binding->fStep,
                                    *draw->fDrawParams,
                                    false)) {
                        return nullptr;
                    }
                }
            } else {
                fStencilLists++;
                // TODO Test the performance of transposed traversal of stencil draws!
                const auto* chainedList = static_cast<const BindingList<ChainedDraw>*>(binding);
                for (const ChainedDraw* headDraw : chainedList->fDraws) {
                    const ChainedDraw* current = headDraw;
                    while (current) {
                        if (!recordDraw(current->fKey,
                                        current->fUniformIndex,
                                        current->fStep,
                                        *current->fDrawParams,
                                        false)) {
                            return nullptr;
                        }
                        current = current->fChildDraw;
                        fStencilDraws++;
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

    auto endSnapPass = std::chrono::high_resolution_clock::now();
    fTotalSnapDrawPassTime += std::chrono::duration<double, std::milli>(endSnapPass - startSnapPass).count();

    return drawPass;
}

}  // namespace skgpu::graphite
