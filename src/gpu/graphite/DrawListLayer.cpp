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

template <bool kIsStencil>
void DrawListLayer::recordBackwards(int stepIndex,
                                        bool dependsOnDst,
                                        const RenderStep* step,
                                        const LayerKey& key,
                                        const DrawParams* drawParams,
                                        const BarrierType& barrierBeforeDraws) {
    // Child stencils get a fast path to their parent
    if constexpr (kIsStencil) {
        if (stepIndex > 0) {
            ChainedDraw* draw = fStorage.make<ChainedDraw>(
                key, step, drawParams, barrierBeforeDraws);
            SkASSERT(fLastRecordedDraw && !fLastRecordedDraw->fChildDraw);
            fLastRecordedDraw->fChildDraw = draw;
            fLastRecordedDraw = draw;
            return;
        }
    }

    Layer* current = nullptr;
    Layer* targetLayer = nullptr;
    BindingWrapper* targetMatch = nullptr;
    SkTInternalLList<Layer>::Iter iter;

    // If we're an easy draw (!kIsStencil and !dependsOnDst), try the head first.
    if constexpr (!kIsStencil) {
        if (!dependsOnDst) {
            targetLayer = fLayers.head();
            if (targetLayer) {
                targetMatch = targetLayer->searchBinding(key);
            }
        } else {
            current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
        }
    }

    int limit = kMaxSearchLimit;
    while (current && limit > 0) {
#if defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(current->fPrev);
#endif
        BoundsTest result = current->test(drawParams->drawBounds(), key, kIsStencil);

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

    if constexpr (kIsStencil) {
        ChainedDraw* draw = fStorage.make<ChainedDraw>(
                key, step, drawParams, barrierBeforeDraws);
        targetLayer->add(&fStorage, targetMatch, key, draw, dependsOnDst, true);
        fLastRecordedDraw = draw;
    } else {
        SingleDraw* draw = fStorage.make<SingleDraw>(step, drawParams, barrierBeforeDraws);
        targetLayer->add(&fStorage, targetMatch, key, draw, dependsOnDst, false);
    }
}

void DrawListLayer::recordStep(int stepIndex,
                               bool dependsOnDst,
                               const RenderStep* step,
                               const LayerKey& key,
                               const DrawParams* drawParams,
                               const BarrierType& barrierBeforeDraws) {
    this->recordBackwards</*kIsStencil=*/false>(stepIndex, dependsOnDst, step, key, drawParams,
                                                barrierBeforeDraws);
}

// TODO (thomsmit):
// Currently, stencils are recorded atomically. However, we should try an alternative scheme similar
// to recording clipped draws. The first step in a stencil draw stores the layer in which it inserts
// into. It returns incompatible for other stencil layers, but is notably able to bypass "stencil
// regions" i.e. regions which are contained between the first and last step of a stencil draw. Once
// the first step inserts, it records the layer which it inserted into. The child steps begin their
// search at that layer and progress FORWARDS, they either: 1) Find a layer which can take them 2)
// traverse until they hit the search limit 3) hit another stencil (which signals the start of
// another stencil region). If 2 or 3 occur (no match found), we insert a layer immediately after
// the first step. The layer pointer is updated to the next layer.
//
// This obviates the need for chained draws, at the cost of additional layer insertions, and
// disjointed layer ordering. (i.e. layers will no longer be exclusively inserted at the tail of the
// list).
void DrawListLayer::recordStencil(int stepIndex,
                                  bool dependsOnDst,
                                  const RenderStep* step,
                                  const LayerKey& key,
                                  const DrawParams* drawParams,
                                  const BarrierType& barrierBeforeDraws) {
    this->recordBackwards</*kIsStencil=*/true>(stepIndex, dependsOnDst, step, key, drawParams,
                                               barrierBeforeDraws);
}

void DrawListLayer::recordDepthOnly(
        int stepIndex,
        bool isStencil,
        bool dependsOnDst,
        const RenderStep* step,
        const LayerKey& key,
        const DrawParams* drawParams,
        const BarrierType& barrierBeforeDraws,
        std::optional<std::reference_wrapper<Layer*>> captureLayer) {
    if (stepIndex > 0) {
        ChainedDraw* deferredDraw =
                fStorage.make<ChainedDraw>(key, step, drawParams, barrierBeforeDraws);
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
        if (captureLayer->get() == current) {
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

    ChainedDraw* deferredDraw =
            fStorage.make<ChainedDraw>(key, step, drawParams, barrierBeforeDraws);
    targetLayer->addDepthOnlyDraw(&fStorage, deferredDraw, isStencil);
    fLastDeferredDraw = deferredDraw;
    if (shouldOverwriteLayer) {
        captureLayer->get() = targetLayer;
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
                                  const LayerKey& key,
                                  const DrawParams* drawParams,
                                  const BarrierType& barrierBeforeDraws,
                                  std::optional<std::reference_wrapper<Layer*>> startLayer) {
    // Child stencils get a fast path to their parent
    if (isStencil && stepIndex > 0) {
        ChainedDraw* draw = fStorage.make<ChainedDraw>(key, step, drawParams, barrierBeforeDraws);
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
        ChainedDraw* draw = fStorage.make<ChainedDraw>(key, step, drawParams, barrierBeforeDraws);
        targetLayer->add(&fStorage, targetMatch, key, draw, dependsOnDst, isStencil);
        fLastRecordedDraw = draw;
    } else {
        SingleDraw* draw = fStorage.make<SingleDraw>(step, drawParams, barrierBeforeDraws);
        targetLayer->add(&fStorage, targetMatch, key, draw, true, isStencil); //See comment 4 above!
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
    bool isStencil = (renderer->depthStencilFlags() & DepthStencilFlags::kStencil) !=
                     DepthStencilFlags::kNone;
    bool dependsOnDst = (dstUsage & DstUsage::kDependsOnDst) != DstUsage::kNone;

    // In this iteration, the draw params are created once per record draw call, and the pointer is
    // passed to each draw call. This is storage effecient but will still introduce some pointer
    // chasing, because the params will likely no longer be on the same cache line for successor
    // render steps. We should test whether it is faster for each step to hold a copy of the params
    // except in the case of clipped draws (which must share a copy because they are mutated later).
    DrawParams* drawParams = fStorage.make<DrawParams>(this->deduplicateTransform(localToDevice),
                                                       geometry, clip, ordering, stroke);


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

        if (captureParams.has_value()) {
            this->recordDepthOnly(stepIndex,
                                  isStencil,
                                  dependsOnDst,
                                  step,
                                  LayerKey{pipelineIndex, uniformIndex, textureBindingIndex},
                                  drawParams,
                                  barrierBeforeDraws,
                                  layer);
            captureParams->get() = drawParams;
        } else {
            if (layer.has_value()) {
                // Note it is possible to have a nullptr at this point.
                if (!dependsOnDst) {
                    this->recordClipped(stepIndex,
                                    isStencil,
                                    dependsOnDst,
                                    step,
                                    LayerKey{pipelineIndex, uniformIndex, textureBindingIndex},
                                    drawParams,
                                    barrierBeforeDraws,
                                    layer);
                } else {
                    // TODO use layer as no BypassMarker
                    if (isStencil) {
                        this->recordStencil(stepIndex,
                                            dependsOnDst,
                                            step,
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
            } else {
                if (isStencil) {
                    this->recordStencil(stepIndex,
                                        dependsOnDst,
                                        step,
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

    for (Layer* layer : fLayers) {
        if (layer->fDepthOnlyDraws) {
            for (ChainedDraw* headDraw : *layer->fDepthOnlyDraws) {
                ChainedDraw* current = headDraw;
                while (current) {
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
                    if (!recordDraw(binding->fKey,
                                    draw->fStep,
                                    *draw->fDrawParams,
                                    draw->fBarrierBeforeDraws,
                                    false)) {
                        return nullptr;
                    }
                }
            } else {
                // TODO Test the performance of transposed traversal of stencil draws!
                const auto* chainedList = static_cast<const BindingList<ChainedDraw>*>(binding);
                for (const ChainedDraw* headDraw : chainedList->fDraws) {
                    const ChainedDraw* current = headDraw;
                    while (current) {
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
