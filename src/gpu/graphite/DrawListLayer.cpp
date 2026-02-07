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

template <bool kIsDepthOnly>
void DrawListLayer::recordBackwards(int stepIndex,
                                    bool isStencil,
                                    bool dependsOnDst,
                                    const RenderStep* step,
                                    const UniformDataCache::Index& uniformIndex,
                                    const LayerKey& key,
                                    const DrawParams* drawParams,
                                    const Layer* stopLayer,
                                    Layer** captureLayer) {
    // Child stencils get a fast path to their parent
    if (isStencil) {
        if (stepIndex > 0) {
            SkASSERT(fStencilLayer);
            SkASSERT(fStencilList);
            SkASSERT(fStencilWrapper);
            SingleDraw* draw = fStorage.make<SingleDraw>(drawParams, uniformIndex);
            fStencilLayer->addStencil<kIsDepthOnly>(
                    &fStorage, fStencilWrapper, key, draw, step, &fStencilList);
            return;
        } else {
            fStencilList = nullptr;
        }
    }

    SkTInternalLList<Layer>::Iter iter;
    Layer* targetLayer = nullptr;
    BindingWrapper* targetMatch = nullptr;

    // If we're an easy draw (!kIsStencil and !dependsOnDst), try the head first.
    Layer* current;
    if constexpr (!kIsDepthOnly) {
        if (!isStencil && !dependsOnDst && fLayers.head() && fLayers.head() != stopLayer) {
            targetMatch = fLayers.head()->searchBinding(key, isStencil);
            current = const_cast<Layer*>(stopLayer);
        } else {
            current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
        }
    } else {
        current = iter.init(fLayers, SkTInternalLList<Layer>::Iter::kTail_IterStart);
    }

    bool shouldOverwriteLayer = true;
    int limit = kMaxSearchLimit;
    // When stopLayer == nullptr this is effectively while(current)
    while (current != stopLayer && limit > 0) {
#if defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(current->fPrev);
#endif
        BoundsTest result = isStencil ? current->test<true>(drawParams->drawBounds(), key)
                                      : current->test<false>(drawParams->drawBounds(), key);
        if constexpr (kIsDepthOnly) {
            if (*captureLayer == current) {
                shouldOverwriteLayer = false;
            }
        }

        if (result == BoundsTest::kIncompatibleOverlap) {
            // If we need to read the dst, we cannot go earlier than this layer.
            if (dependsOnDst) {
                if (current->fNext) {
                    targetLayer = current->fNext;

                    // We found our target layer, grab the binding if it exists
                    targetMatch = targetLayer->searchBinding(key, isStencil);
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
                targetMatch = current->searchBinding(key, isStencil);
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
        if constexpr (kIsDepthOnly) {
            shouldOverwriteLayer = true;
        }
    }

    SkASSERT(targetLayer);
    SingleDraw* draw = fStorage.make<SingleDraw>(drawParams, uniformIndex);
    if (isStencil) {
        targetLayer->addStencil<kIsDepthOnly>(
                &fStorage, targetMatch, key, draw, step, &fStencilList);
        fStencilLayer = targetLayer;
        fStencilWrapper = targetMatch;
    } else {
        targetLayer->add<kIsDepthOnly>(&fStorage, targetMatch, key, draw, step, dependsOnDst);
    }

    if constexpr (kIsDepthOnly) {
        fParentDepthLayer = targetLayer;
        if (shouldOverwriteLayer) {
            *captureLayer = targetLayer;
        }
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
    if (isStencil) {
        if (stepIndex > 0) {
            SkASSERT(fStencilLayer);
            SkASSERT(fStencilList);
            SkASSERT(fStencilWrapper);
            SingleDraw* draw = fStorage.make<SingleDraw>(drawParams, uniformIndex);
            fStencilLayer->addStencil<false>(
                    &fStorage, fStencilWrapper, key, draw, step, &fStencilList);
            return;
        } else {
            fStencilList = nullptr;
        }
    }

    Layer* current = *startLayer;
    Layer* targetLayer = nullptr;
    BindingWrapper* targetMatch = nullptr;

    int limit = kMaxSearchLimit;
    while (current && limit > 0) {
#if defined(__GNUC__) || defined(__clang__)
        __builtin_prefetch(current->fNext);
#endif
        BoundsTest result = isStencil ? current->test<true>(drawParams->drawBounds(), key)
                                      : current->test<false>(drawParams->drawBounds(), key);
        if (result != BoundsTest::kIncompatibleOverlap) {
            targetLayer = current;
            targetMatch = current->searchBinding(key, isStencil);
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
    SingleDraw* draw = fStorage.make<SingleDraw>(drawParams, uniformIndex);
    if (isStencil) {
        targetLayer->addStencil<false>(&fStorage, targetMatch, key, draw, step, &fStencilList);
        fStencilLayer = targetLayer;
        fStencilWrapper = targetMatch;
    } else {
        targetLayer->add<false>(&fStorage, targetMatch, key, draw, step, dependsOnDst);
    }
    *startLayer = targetLayer;
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
                                                       geometry,
                                                       clip,
                                                       ordering,
                                                       stroke,
                                                       barrierBeforeDraws);

    fParentDepthLayer = nullptr;
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
            this->recordBackwards<true>(stepIndex,
                                        isStencil,
                                        true,
                                        step,
                                        uniformIndex,
                                        LayerKey{pipelineIndex, textureBindingIndex},
                                        drawParams,
                                        fParentDepthLayer,  // TODO bad!
                                        captureLayer);
            *captureParams = drawParams;
        } else {
            if (captureLayer && !dependsOnDst) {
                this->recordClipped(stepIndex,
                                    isStencil,
                                    false,
                                    step,
                                    uniformIndex,
                                    LayerKey{pipelineIndex, textureBindingIndex},
                                    drawParams,
                                    captureLayer);
            } else {
                this->recordBackwards<false>(stepIndex,
                                             isStencil,
                                             dependsOnDst,
                                             step,
                                             uniformIndex,
                                             LayerKey{pipelineIndex, textureBindingIndex},
                                             drawParams,
                                             captureLayer ? *captureLayer : nullptr,
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
    fTotalRecordDrawTime +=
            std::chrono::duration<double, std::milli>(endRecordDraw - startRecordDraw).count();
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
        for (const BindingWrapper* binding : layer->fBindings) {
            if (binding->fType == BindingListType::kSingle) {
                const auto* singleList = static_cast<const SingleDrawList*>(binding);
                SkASSERT(!singleList->fDraws.isEmpty());

                const SingleDraw* current = singleList->fDraws.head();

                // First draw in the list initiates the pipeline bind (last arg = false)
                if (!recordDraw(singleList->fKey,
                                current->fUniformIndex,
                                singleList->fStep,
                                *current->fDrawParams,
                                false)) {
                    return nullptr;
                }

                current = current->fNext;
                while (current) {
                    // Subsequent draws are merged/instanced (last arg = true)
                    if (!recordDraw(singleList->fKey,
                                    current->fUniformIndex,
                                    singleList->fStep,
                                    *current->fDrawParams,
                                    true)) {
                        return nullptr;
                    }
                    current = current->fNext;
                }
            } else {
                fStencilLists++;
                const auto* stencilList = static_cast<const StencilDrawList*>(binding);
                for (const StencilDraws* sd : stencilList->fStencilDraws) {
                    if (!sd || sd->fDraws.isEmpty()) {
                        continue;
                    }

                    const SingleDraw* first = sd->fDraws.head();
                    if (!recordDraw(sd->fKey,
                                    first->fUniformIndex,
                                    sd->fStep,
                                    *first->fDrawParams,
                                    false)) {
                        return nullptr;
                    }
                    fStencilDraws++;

                    for (const SingleDraw* current = first->fNext; current;
                         current = current->fNext) {
                        if (!recordDraw(sd->fKey,
                                        current->fUniformIndex,
                                        sd->fStep,
                                        *current->fDrawParams,
                                        true)) {
                            return nullptr;
                        }
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
    fTotalSnapDrawPassTime +=
            std::chrono::duration<double, std::milli>(endSnapPass - startSnapPass).count();

    return drawPass;
}

}  // namespace skgpu::graphite
