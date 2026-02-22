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
    fOrderCounter = CompressedPaintersOrder::First();
}

// Draws affected by depth only draws we call "clipped draws."
//
// Clipped draws must come *after* all depth only draws that affect them, and they must come *after*
// any preceding draws from the same renderstep. To accomodate this:
//      1) When recording the depth only draws, a pointer marking the latest layer inserted into is
//         passed between each draw. If a later draws inserts after an earlier draw, the pointer is
//         overwritten. This ensures that the pointer is always the *latest* layer.
//      2) How the pointer is used depends on the property of clipped draw:
//         - Clipped draws which do not dependOnDst use this as the start of the traversal, then
//           proceed FORWARDS until finding a suitable layer.
//         - Clipped draws which do dependOnDst must stop when encountering any shading intersecting
//           draw. Thus, forwards traversal becomes impractical because the draw must exhaustively
//           search layers to the tail to ensure that there are no intersections. Instead,
//           these draws must take the normal BACKWARDS traversal.
//      3) Each clipped draw updates the starting layer to the layer that it inserted into. Because
//         the stopLayer is treated exclusively, a sucessor renderstep stops its traversal before
//         the stopLayer, thus preserving the relative ordering between the draws. (Note: will be
//         changed in future CL, so kind of stub comment)
template <bool kIsDepthOnly>
void DrawListLayer::recordBackwards(int stepIndex,
                                    bool isStencil,
                                    bool dependsOnDst,
                                    bool requiresBarrier,
                                    const RenderStep* step,
                                    const UniformDataCache::Index& uniformIndex,
                                    const LayerKey& key,
                                    const DrawParams* drawParams,
                                    const Insertion& stop,
                                    Insertion* capture) {
    uint8_t bestMatchMask = 0;
    Layer* current = nullptr;
    Layer* targetLayer = nullptr;
    BindingList* targetMatch = nullptr;
    BindingList* forwardMerge = nullptr;
    // If we're an easy draw (!kIsStencil and !dependsOnDst), try the head first.
    if (!isStencil && !dependsOnDst) {
        // A valid stopLayer will never be null, because the depth draw will always return the layer
        // it drew into.
        targetLayer = stop.layer ? stop.layer : fLayers.head();
        if (targetLayer) {
            targetMatch = targetLayer->searchBinding(key, stop.list);
        }
    } else {
        current = fLayers.tail();
        auto processLayer = [&](BindingList* boundary) -> bool {
            auto [overlapType, match, matchMask] =
                    isStencil ? current->test<true, kIsDepthOnly, false>(
                                        drawParams->drawBounds(), key, requiresBarrier, boundary)
                              : current->test<false, kIsDepthOnly, false>(
                                        drawParams->drawBounds(), key, requiresBarrier, boundary);

            if (overlapType == BoundsTest::kIncompatibleOverlap) {
                // If we need to read the dst, we cannot go earlier than this layer.
                if (dependsOnDst) {
                    // Cannot forward merge depth only draws due to risk of pulling a depth draw
                    // after an already inserted clipped draw.
                    if constexpr (!kIsDepthOnly) {
                        if (LayerKey::isExactMatch(matchMask) &&
                            current->fBindings.head() != current->fBindings.tail()) {
                            if (!isStencil ||
                                !match->fBounds.intersects(drawParams->drawBounds())) {
                                forwardMerge = match;
                                targetMatch = forwardMerge;
                            }
                        }
                    }
                    return true;
                } else {
                    // If !dependsOnDst, we just keep searching backwards.
                    return false;
                }
            } else {
                // Found a valid layer (Compatible or Disjoint)
                if (matchMask > bestMatchMask) {
                    targetLayer = current;
                    targetMatch = match;
                    bestMatchMask = matchMask;
                }

                // If it was compatible, we expect a match. If disjoint, match is nullptr.
                return overlapType == BoundsTest::kCompatibleOverlap;
            }
            SkUNREACHABLE;
        };

        // Check current here for safety?
        for (uint32_t limit = 0; limit < kMaxSearchLimit && current != stop.layer; ++limit) {
#if defined(__GNUC__) || defined(__clang__)
            __builtin_prefetch(current->fPrev);
#endif
            if (processLayer(nullptr)) {
                break;
            }
            current = current->fPrev;
        }
        if (current && current == stop.layer) {
            processLayer(stop.list);
        }
    }

    if (!targetLayer) {
        fOrderCounter = fOrderCounter.next();
        targetLayer = fStorage.make<Layer>(fOrderCounter);
        if (forwardMerge) {
            SkASSERT(current);
            current->fBindings.remove(forwardMerge);
            targetLayer->fBindings.addToHead(forwardMerge);
            forwardMerge->fOrder = CompressedPaintersOrder::First();
            fLayers.addAfter(targetLayer, current);
        } else {
            fLayers.addToTail(targetLayer);
        }
    }

    SkASSERT(targetLayer);
    Draw* draw = fStorage.make<Draw>(drawParams, uniformIndex);
    bool notStopLayer = targetLayer != stop.layer;
    BindingList* inserted = targetLayer->add<kIsDepthOnly>(
            &fStorage, targetMatch, key, draw, step, !dependsOnDst && notStopLayer, isStencil, bestMatchMask);

    SkASSERT(inserted);
    if (stepIndex > 0) {
        if (targetLayer->fOrder > capture->layer->fOrder) {
            capture->layer = targetLayer;
            capture->list = inserted;
        } else if (targetLayer->fOrder == capture->layer->fOrder) {
            if (inserted->fOrder > capture->list->fOrder) {
                capture->layer = targetLayer;
                capture->list = inserted;
            }
        }
    } else {
        capture->layer = targetLayer;
        capture->list = inserted;
    }
}

void DrawListLayer::recordForwards(int stepIndex,
                                   bool isStencil,
                                   bool dependsOnDst,
                                   bool requiresBarrier,
                                   const RenderStep* step,
                                   const UniformDataCache::Index& uniformIndex,
                                   const LayerKey& key,
                                   const DrawParams* drawParams,
                                   const Insertion& start) {
    Layer* current = const_cast<Layer*>(start.layer);
    uint8_t bestMatchMask = 0;
    Layer* targetLayer = nullptr;
    BindingList* targetMatch = nullptr;
    auto processLayer = [&](BindingList* boundary) -> bool {
        auto [overlapType, match, matchMask] = isStencil ? current->test<true, false, true>(
                                          drawParams->drawBounds(), key, requiresBarrier, boundary)
                                : current->test<false, false, true>(
                                          drawParams->drawBounds(), key, requiresBarrier, boundary);
        if (overlapType != BoundsTest::kIncompatibleOverlap) {
            if (matchMask > bestMatchMask) {
                targetLayer = current;
                targetMatch = match;
                bestMatchMask = matchMask;
            }
            return overlapType == BoundsTest::kCompatibleOverlap;
        } else {
            return isStencil;
        }
        SkUNREACHABLE;
    };

    if (current) {
        if (!processLayer(start.list)) {
            current = current->fNext;
            for (uint32_t limit = 0; limit < kMaxSearchLimit && current; ++limit) {
#if defined(__GNUC__) || defined(__clang__)
                __builtin_prefetch(current->fNext);
#endif
                if (processLayer(nullptr)) {
                    break;
                }
                current = current->fNext;
            }
        }
    }

    if (!targetLayer) {
        fOrderCounter = fOrderCounter.next();
        targetLayer = fStorage.make<Layer>(fOrderCounter);
        if (start.layer) {
            fLayers.addAfter(targetLayer, start.layer);
        } else {
            fLayers.addToTail(targetLayer);
        }
    }

    SkASSERT(targetLayer);
    Draw* draw = fStorage.make<Draw>(drawParams, uniformIndex);
    bool notStartLayer = targetLayer != start.layer;
    targetLayer->add<false>(
            &fStorage, targetMatch, key, draw, step, !dependsOnDst && notStartLayer, isStencil, bestMatchMask);
}

// Layer has dual purpose here:
//  1) (Producer) If recording a depth only draw, the pointer is set to the *latest* layer inserted.
//  2) (Consumer) If recording a clipped draw, the pointer is the latest layer inserted into across
//     *all depth only draws* which affect this draw. Thus, it is the earliest possible layer that
//     the clipped draw could be inserted into, so it is used as the starting point for a *forward*
//     search.
std::pair<DrawParams*, Insertion> DrawListLayer::recordDraw(const Renderer* renderer,
                                                            const Transform& localToDevice,
                                                            const Geometry& geometry,
                                                            const Clip& clip,
                                                            DrawOrder ordering,
                                                            UniquePaintParamsID paintID,
                                                            SkEnumBitMask<DstUsage> dstUsage,
                                                            BarrierType barrierBeforeDraws,
                                                            PipelineDataGatherer* gatherer,
                                                            const StrokeStyle* stroke,
                                                            const Insertion& latestInsertion) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!geometry.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());

    // Stencil-based renderers consist of multiple steps: some modify the stencil buffer, while
    // others shade. However, for the purpose of intersection testing, we must treat all of these
    // steps as stenciling operations.
    //
    // This is because incorrect consumption risks corruption just as much as incorrect
    // modification. Interleaving one stencil sequence into another corrupts the stencil buffer
    // state or the shading results that depend on it. This effectively creates "stencil regions" in
    // the draw list enclosed by the first stencil draw from a renderer and its corresponding last
    // shading draw.
    //
    // Incoming stencil draws must respect these regions and cannot interleave. However, because the
    // initial stencil draws (which must be pilot draws) are not transparent---meaning they trigger
    // an incompatible overlap against other stencils but are not forced to halt traversal---they
    // can successfully bypass an existing stencil region entirely, skipping over it to insert into
    // an earlier layer.
    //
    // Although it is possible to treat stencil rendersteps individually (e.g., allowing stencils
    // renderers with transparent shading steps or depth-only stencils to bypass existing stencil
    // regions to find optimal batches), doing so ends up being a slowdown because the stencil draws
    // never are able to early-exit out of list traversal except on exact matches.
    bool rendererIsStencil = SkToBool(renderer->depthStencilFlags() & DepthStencilFlags::kStencil);
    bool dependsOnDst = SkToBool(dstUsage & DstUsage::kDependsOnDst);
    bool requiresBarrier = barrierBeforeDraws != BarrierType::kNone;

    // Currently, the draw params are created once per record draw call, and the pointer is passed
    // to each draw call. This is storage effecient but will still introduce some pointer chasing,
    // because the params will likely no longer be on the same cache line for successor render
    // steps. We should test whether it is faster for each step to hold a copy of the params except
    // in the case of clipped draws (which must share a copy because they are mutated later).
    DrawParams* drawParams = fStorage.make<DrawParams>(this->deduplicateTransform(localToDevice),
                                                       geometry,
                                                       clip,
                                                       ordering,
                                                       stroke,
                                                       barrierBeforeDraws);

    Insertion stepInsertion = {nullptr, nullptr};
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

        if (paintID == UniquePaintParamsID::Invalid()) {  // Invalid ID implies depth only draw
            this->recordBackwards<true>(stepIndex,
                                        rendererIsStencil,
                                        true,
                                        requiresBarrier,
                                        step,
                                        uniformIndex,
                                        LayerKey{pipelineIndex, textureBindingIndex},
                                        drawParams,
                                        /*stop=*/{},
                                        &stepInsertion);
        } else {
            if (stepIndex == 0) {
                this->recordBackwards<false>(stepIndex,
                                             rendererIsStencil,
                                             dependsOnDst,
                                             requiresBarrier,
                                             step,
                                             uniformIndex,
                                             LayerKey{pipelineIndex, textureBindingIndex},
                                             drawParams,
                                             latestInsertion,
                                             &stepInsertion);
            } else {
                this->recordForwards(stepIndex,
                                     rendererIsStencil,
                                     false,
                                     requiresBarrier,
                                     step,
                                     uniformIndex,
                                     LayerKey{pipelineIndex, textureBindingIndex},
                                     drawParams,
                                     stepInsertion);
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

    return {drawParams, stepInsertion};
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
                          const UniformDataCache::Index uniformIndex,
                          const RenderStep* renderStep,
                          const DrawParams& drawParams,
                          bool bindingsAreInvariant) -> bool {
        SkASSERT(renderStep);

        bool pipelineChange = false;
        bool textureBindingsChange = false;

        if (!bindingsAreInvariant) {
            pipelineChange = key.fPipelineIndex != lastPipeline;

            textureBindingsChange =
                    textureBindingTracker.setCurrentTextureBindings(key.fTextureIndex) ||
                    (rebindTexturesOnPipelineChange && pipelineChange &&
                     key.fTextureIndex != TextureDataCache::kInvalidIndex);
        }

        bool uniformBindingChange =
                uniformTracker.writeUniforms(fUniformDataCache, bufferMgr, uniformIndex);

        drawPass->fPipelineDrawAreas[key.fPipelineIndex] += drawParams.drawBounds().area();

        std::optional<SkIRect> newScissor =
                renderStep->getScissor(drawParams, lastScissor, targetBounds);

        if (pipelineChange) {
            drawWriter.newPipelineState(renderStep->primitiveType(),
                                        renderStep->staticDataStride(),
                                        renderStep->appendDataStride(),
                                        renderStep->getRenderStateFlags(),
                                        drawParams.barrierBeforeDraws());
        } else if (uniformBindingChange || textureBindingsChange || newScissor.has_value()) {
            drawWriter.newDynamicState();
        } else if (drawParams.barrierBeforeDraws() != BarrierType::kNone) {
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
        for (const BindingList* list : layer->fBindings) {
            SkASSERT(!list->fDraws.isEmpty());
            const Draw* current = list->fDraws.head();

            if (!recordDraw(list->fKey,
                            current->fUniformIndex,
                            list->fStep,
                            *current->fDrawParams,
                            false)) {
                return nullptr;
            }
            current = current->fNext;

            while (current) {
                if (!recordDraw(list->fKey,
                                current->fUniformIndex,
                                list->fStep,
                                *current->fDrawParams,
                                true)) {
                    return nullptr;
                }
                current = current->fNext;
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
