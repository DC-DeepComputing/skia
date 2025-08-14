/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawList.h"

#include "include/core/SkTypes.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawCommands.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/geom/Geometry.h"

namespace skgpu::graphite {

namespace {

// Writes uniform data either to uniform buffers or to shared storage buffers, and tracks when
// bindings need to change between draws.
class UniformTracker {
public:
    UniformTracker(bool useStorageBuffers) : fUseStorageBuffers(useStorageBuffers) {}

    bool writeUniforms(UniformDataCache& uniformCache,
                       DrawBufferManager* bufferMgr,
                       UniformDataCache::Index index) {
        if (index >= UniformDataCache::kInvalidIndex) {
            return false;
        }

        if (index == fLastIndex) {
            return false;
        }
        fLastIndex = index;

        UniformDataCache::Entry& uniformData = uniformCache.lookup(index);
        const size_t uniformDataSize = uniformData.fCpuData.size();

        // Upload the uniform data if we haven't already.
        // Alternatively, re-upload the uniform data to avoid a rebind if we're using storage
        // buffers. This will result in more data uploaded, but the tradeoff seems worthwhile.
        if (!uniformData.fBufferBinding.fBuffer ||
            (fUseStorageBuffers && uniformData.fBufferBinding.fBuffer != fLastBinding.fBuffer)) {
            UniformWriter writer;
            std::tie(writer, uniformData.fBufferBinding) =
                    fUseStorageBuffers ? bufferMgr->getAlignedSsboWriter(1, uniformDataSize)
                                       : bufferMgr->getUniformWriter(1, uniformDataSize);

            // Early out if buffer mapping failed.
            if (!writer) {
                return {};
            }

            writer.write(uniformData.fCpuData.data(), uniformDataSize);

            if (fUseStorageBuffers) {
                // When using storage buffers, store the SSBO index in the binding's offset field
                // and always use the entire buffer's size in the size field.
                SkASSERT(uniformData.fBufferBinding.fOffset % uniformDataSize == 0);
                uniformData.fBufferBinding.fOffset /= uniformDataSize;
                uniformData.fBufferBinding.fSize = uniformData.fBufferBinding.fBuffer->size();
            }
        }

        const bool needsRebind =
                uniformData.fBufferBinding.fBuffer != fLastBinding.fBuffer ||
                (!fUseStorageBuffers && uniformData.fBufferBinding.fOffset != fLastBinding.fOffset);

        fLastBinding = uniformData.fBufferBinding;

        return needsRebind;
    }

    void bindUniforms(UniformSlot slot, DrawPassCommands::List* commandList) {
        BindBufferInfo binding = fLastBinding;
        if (fUseStorageBuffers) {
            // Track the SSBO index in fLastBinding, but set offset = 0 in the actual used binding.
            binding.fOffset = 0;
        }
        commandList->bindUniformBuffer(binding, slot);
    }

    uint32_t ssboIndex() const {
        // The SSBO index for the last-bound storage buffer is stored in the binding's offset field.
        return fLastBinding.fOffset;
    }

private:
    // Internally track the last binding returned, so that we know whether new uploads or rebindings
    // are necessary. If we're using SSBOs, this is treated specially -- the fOffset field holds the
    // index in the storage buffer of the last-written uniforms, and the offsets used for actual
    // bindings are always zero.
    BindBufferInfo fLastBinding;

    // This keeps track of the last index used for writing uniforms from a provided uniform cache.
    // If a provided index matches the last index, the uniforms are assumed to already be written
    // and no additional uploading is performed. This assumes a UniformTracker will always be
    // provided with the same uniform cache.
    UniformDataCache::Index fLastIndex = UniformDataCache::kInvalidIndex;

    const bool fUseStorageBuffers;
};

// Tracks when to issue BindTexturesAndSamplers commands to a command list and converts
// TextureDataBlocks to that representation as needed.
class TextureTracker {
public:
    TextureTracker(TextureDataCache* textureCache)
            : fTextureCache(textureCache) {}

    bool setCurrentTextureBindings(TextureDataCache::Index bindingIndex) {
        if (bindingIndex < TextureDataCache::kInvalidIndex && fLastIndex != bindingIndex) {
            fLastIndex = bindingIndex;
            return true;
        }
        // No binding change
        return false;
    }

    void bindTextures(DrawPassCommands::List* commandList) {
        SkASSERT(fLastIndex < TextureDataCache::kInvalidIndex);
        TextureDataBlock binding = fTextureCache->lookup(fLastIndex);

        auto [textures, samplers] =
                commandList->bindDeferredTexturesAndSamplers(binding.numTextures());

        for (int i = 0; i < binding.numTextures(); ++i) {
            auto [t, s] = binding.texture(i);
            textures[i] = t.get();
            samplers[i] = s;
        }
    }

private:
    TextureDataCache::Index fLastIndex = TextureDataCache::kInvalidIndex;

    TextureDataCache* const fTextureCache;
};

}  // anonymous namespace

const Transform& DrawList::deduplicateTransform(const Transform& localToDevice) {
    // TODO: This is a pretty simple deduplication strategy and doesn't take advantage of the stack
    // knowledge that Device has.
    if (fTransforms.empty() || fTransforms.back() != localToDevice) {
        fTransforms.push_back(localToDevice);
    }
    return fTransforms.back();
}

void DrawList::reset(LoadOp loadOp, SkColor4f color) {
    fSortKeys.clear();
    fDraws.reset();
    fTransforms.reset();

    fPaintUniformCache.reset();
    fRenderStepUniformCache.reset();
    fTextureCache.reset();
    fPipelineCache.reset();

    fLoadOp = loadOp;
    fClearColor = color.premul().array(); // FIXME: color conversion yet?

    fDrawBounds = Rect::InfiniteInverted();
    fDstReadBounds = Rect::InfiniteInverted();
    fDepthStencilFlags = DepthStencilFlags::kNone;
    fRequiresMSAA = false;

    SkDEBUGCODE(fCoverageMaskShapeDrawCount = 0;)
}

void DrawList::recordDraw(const Renderer* renderer,
                          const Transform& localToDevice,
                          const Geometry& geometry,
                          const Clip& clip,
                          DrawOrder ordering,
                          UniquePaintParamsID paintID,
                          DstReadStrategy dstReadStrategy,
                          bool dependsOnDst,
                          SkBlendMode finalBlendMode,
                          PipelineDataGatherer* gatherer,
                          const StrokeStyle* stroke) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!geometry.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());
    SkASSERT(!(renderer->depthStencilFlags() & DepthStencilFlags::kStencil) ||
             ordering.stencilIndex() != DrawOrder::kUnassigned);

    // TODO: Add validation that the renderer's expected shape type and stroke params match provided
    UniformDataCache::Index paintUniformIndex = UniformDataCache::kInvalidIndex;
    // TextureDataBlock paintTextures;
    if (!paintID.isValid()) {
        UniformDataBlock paintUniforms = gatherer->endPaintData();
        if (!paintUniforms.empty()) {
            paintUniformIndex = fPaintUniformCache.insert(paintUniforms);
        }
    }

    const Draw& draw = fDraws.emplace_back(renderer,
                                           this->deduplicateTransform(localToDevice),
                                           geometry,
                                           clip,
                                           ordering,
                                           stroke,
                                           dstReadStrategy,
                                           dependsOnDst,
                                           finalBlendMode);

    for (int i = 0; i < renderer->numRenderSteps(); ++i) {
        gatherer->rewindForRenderStep();
        const RenderStep& step = renderer->step(i);

        UniquePaintParamsID perStepPaintID = step.performsShading() ? paintID : paintID.Invalid();
        UniformDataCache::Index perStepPaintUniformIndex =
                step.performsShading() ? paintUniformIndex : UniformDataCache::kInvalidIndex;

        GraphicsPipelineCache::Index pipelineIndex = fPipelineCache.insert({step.renderStepID(),
                                                                            perStepPaintID});

        step.writeUniformsAndTextures(draw.fDrawParams, gatherer);

        auto [stepUniforms, combinedTextures] = gatherer->endRenderStepData(step.performsShading());

        UniformDataCache::Index stepUniformIndex = !stepUniforms.empty() ?
                fRenderStepUniformCache.insert(stepUniforms) : UniformDataCache::kInvalidIndex;

        TextureDataCache::Index textureBindingIndex = !combinedTextures.empty() ?
                fTextureCache.insert(combinedTextures) : TextureDataCache::kInvalidIndex;

        fSortKeys.push_back({&draw, i, pipelineIndex, stepUniformIndex, perStepPaintUniformIndex,
                             textureBindingIndex});
    }

    fDepthStencilFlags |= renderer->depthStencilFlags();
    fRequiresMSAA |= renderer->requiresMSAA();
    fDrawBounds.join(clip.drawBounds());
    if (dstReadStrategy == DstReadStrategy::kTextureCopy) {
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
                                                 const SkImageInfo& targetInfo) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "draw count", fDraws.count());

    // The DrawList is converted directly into the DrawPass' data structures, but once the DrawPass
    // is returned from Make(), it is considered immutable.
    std::unique_ptr<DrawPass> drawPass(new DrawPass(target,
                                                    {fLoadOp, StoreOp::kStore},
                                                    fClearColor,
                                                    recorder->priv().refFloatStorageManager()));

    DrawBufferManager* bufferMgr = recorder->priv().drawBufferManager();
    if (bufferMgr->hasMappingFailed()) {
        SKGPU_LOG_W("Buffer mapping has already failed; dropping draw pass!");
        return nullptr;
    }

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

    // gradient lifted

    // Used to record vertex/instance data, buffer binds, and draw calls
    DrawWriter drawWriter(&drawPass->fCommandList, bufferMgr);
    GraphicsPipelineCache::Index lastPipeline = GraphicsPipelineCache::kInvalidIndex;
    const SkIRect targetBounds = SkIRect::MakeSize(targetInfo.dimensions());
    SkIRect lastScissor = targetBounds;

    SkASSERT(drawPass->fTarget->isFullyLazy() ||
             SkIRect::MakeSize(drawPass->fTarget->dimensions()).contains(lastScissor));
    drawPass->fCommandList.setScissor(lastScissor);

    // All large gradients pack their data into a single buffer throughout the draw pass,
    // therefore the gradient buffer only needs to be bound once.
    auto caps = recorder->priv().caps();
    const bool useStorageBuffers = caps->storageBufferSupport();
    UniformTracker geometryUniformTracker(useStorageBuffers);
    UniformTracker shadingUniformTracker(useStorageBuffers);
    TextureTracker textureBindingTracker(&fTextureCache);

    // TODO(b/372953722): Remove this forced binding command behavior once dst copies are always
    // bound separately from the rest of the textures.
    const bool rebindTexturesOnPipelineChange =
            caps->getDstReadStrategy() == DstReadStrategy::kTextureCopy;
    // Keep track of the prior draw's PaintOrder. If the current draw requires barriers and there
    // is no pipeline or state change, then we must compare the current and prior draw's PaintOrders
    // to determine if the draws overlap. If they do, we must inject a flush between them such that
    // the barrier addition and draw commands are ordered correctly.
    CompressedPaintersOrder priorDrawPaintOrder {};

    // If a draw uses an advanced blend mode and the device supports this via noncoherent blending,
    // then we must insert the appropriate barrier and ensure that the draws do not overlap.
    const bool advancedBlendsRequireBarrier =
            caps->blendEquationSupport() == Caps::BlendEquationSupport::kAdvancedNoncoherent;

    for (const SortKey& key : fSortKeys) {
        const Draw& draw = key.draw();
        const RenderStep& renderStep = key.renderStep();

        const bool pipelineChange = key.pipelineIndex() != lastPipeline;

        const bool geomBindingChange =
                geometryUniformTracker.writeUniforms(fRenderStepUniformCache, bufferMgr, key.geometryUniformIndex());
        const bool shadingBindingChange =
                shadingUniformTracker.writeUniforms(fPaintUniformCache, bufferMgr, key.shadingUniformIndex());

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
        bool drawUsesAdvancedBlendMode = (int)draw.fFinalBlendMode > (int)SkBlendMode::kLastCoeffMode;

        std::optional<BarrierType> barrierToAddBeforeDraws = std::nullopt;
        if (caps->getDstReadStrategy() == DstReadStrategy::kReadFromInput && draw.fDependsOnDst) {
            barrierToAddBeforeDraws = BarrierType::kReadDstFromInput;
        }
        if (drawUsesAdvancedBlendMode &&
            caps->supportsHardwareAdvancedBlending() &&
            advancedBlendsRequireBarrier) {
            // A draw should only read from the dst OR use hardware for advanced blend modes.
            SkASSERT(!draw.fDependsOnDst);
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
    }
    // Finish recording draw calls for any collected data at the end of the loop
    drawWriter.flush();

    drawPass->fBounds = fDrawBounds.roundOut().asSkIRect();
    drawPass->fPipelineDescs   = fPipelineCache.detach();
    drawPass->fSampledTextures = fTextureCache.detachTextures();

    this->reset(LoadOp::kLoad);

    TRACE_COUNTER1("skia.gpu", "# pipelines", drawPass->fPipelineDescs.size());
    TRACE_COUNTER1("skia.gpu", "# textures", drawPass->fSampledTextures.size());
    TRACE_COUNTER1("skia.gpu", "# commands", drawPass->fCommandList.count());

    return drawPass;
}

} // namespace skgpu::graphite
