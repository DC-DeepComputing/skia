/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawList.h"

#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/geom/Shape.h"

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

}  // anonymous namespace

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
                          const StrokeStyle* stroke) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!geometry.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());
    SkASSERT(!(renderer->depthStencilFlags() & DepthStencilFlags::kStencil) ||
             ordering.stencilIndex() != DrawOrder::kUnassigned);

    // TODO: Add validation that the renderer's expected shape type and stroke params match provided

    UniquePaintParamsID paintID = UniquePaintParamsID::InvalidID();
    UniformDataCache::Index paintUniformIndex = UniformDataCache::kInvalidIndex;
    TextureDataBlock paintTextures;
    if (paint) {
        // Convert the PaintParams to a PaintParamsKey and collect its uniforms and textures once
        // (for reuse in case the Renderer has multiple steps that perform shading).
        fDataGatherer.reset();
        KeyContext keyContext{recorder, // FIXME need recorder too
                              localToDevice,
                              targetColorInfo, /*FIXME need this here*/
                              geometry.isShape() || geometry.isEdgeAAQuad()
                                    ? KeyContext::OptimizeSampling::kYes
                                    : KeyContext::OptimizeSampling::kNo};
        paint->toKey(keyContext, fKeyBuilder, &fDataGatherer);
        paintID = recorder->priv().shaderCodeDictionary()->findOrCreate(fKeyBuilder);
        if (!paintID.isValid()) {
            return; // Don't record the draw FIXME log?
        }
        if (fDataGatherer.hasUniforms()) {
            paintUniformIndex = fPaintUniformCache.insert(fDataGatherer.finishUniformDataBlock());
        }
        if (fDataGatherer.hasTextures()) {
            paintTextures = fTextureCache.insert(fDataGatherer.textureDataBlock());
        }
    }

    const Draw& draw = fDraws.emplace_back(DrawParams{this->deduplicateTransform(localToDevice),
                                                      geometry,
                                                      clip,
                                                      ordering,
                                                      stroke},
                                           renderer);

    for (int i = 0; i < renderer->numRenderSteps(); ++i) {
        const RenderStep& step = renderer->step(i);
        GraphicsPipelineCache::Index pipelineIndex = fPipelineCache.insert(
                {step, step.performsShading() ? paintID : UniquePaintParamsID::InvalidID()}
        );

        fDataGatherer.reset();
        step.writeUniformsAndTextures(draw.first, &fDataGatherer);

        UniformDataCache::Index stepUniformIndex = fDataGatherer.hasUniforms() ?
                fRenderStepUniformCache.insert(fDataGatherer.finishUniformDataBlock()) : UniformDataCache::kInvalidIndex;
        TextureDataBlock stepTextures = fDataGatherer.hasTextures() ?
                fTextureCache.insert(fDataGatherer.textureDataBlock()) : TextureDataBlock();

        fSortKeys.push_back({&draw, i, pipelineIndex, stepUniformIndex, step.performsShading() ? paintUniformIndex : UniformDataCache::kInvalidIndex, textureBindingIndex});
    }

    fDepthStencilFlags |= renderer->depthStencilFlags();
    fRequiresMSAA |= renderer->requiresMSAA();
    fDrawBounds.join(clip.drawBounds());
    if (paint && paint->dstReadRequirement() == DstReadRequirement::kTextureCopy) {
        fDstCopyBounds.join(clip.drawBounds());
    }

#if defined(SK_DEBUG)
    if (geometry.isCoverageMaskShape()) {
        fCoverageMaskShapeDrawCount++;
    }
#endif
}

std::unique_ptr<DrawPass> DrawList::snapDrawPass() {
TRACE_EVENT1("skia.gpu", TRACE_FUNC, "draw count", draws->fDraws.count());

    // The DrawList is converted directly into the DrawPass' data structures, but once the DrawPass
    // is returned from Make(), it is considered immutable.
    std::unique_ptr<DrawPass> drawPass(new DrawPass(target, ops, clearColor));

    DrawBufferManager* bufferMgr = recorder->priv().drawBufferManager();
    if (bufferMgr->hasMappingFailed()) {
        SKGPU_LOG_W("Buffer mapping has already failed; dropping draw pass!");
        return nullptr;
    }

    // Geometry uniforms are currently always UBO-backed.
    const bool useStorageBuffers = recorder->priv().caps()->storageBufferSupport();
    const ResourceBindingRequirements& bindingReqs =
            recorder->priv().caps()->resourceBindingRequirements();
    Layout uniformLayout =
            useStorageBuffers ? bindingReqs.fStorageBufferLayout : bindingReqs.fUniformBufferLayout;

    GradientBufferTracker gradientBufferTracker;

    ShaderCodeDictionary* dict = recorder->priv().shaderCodeDictionary();
    // PaintParamsKeyBuilder builder(dict);

    // The initial layout we pass here is not important as it will be re-assigned when writing
    // shading and geometry uniforms below.
    // PipelineDataGatherer gatherer(uniformLayout);

    // std::vector<SortKey> keys;
    // keys.reserve(draws->renderStepCount());

    // for (const DrawList::Draw& draw : draws->fDraws.items()) {
    //     // If we have two different descriptors, such that the uniforms f rom the PaintParams can be
    //     // bound independently of those used by the rest of the RenderStep, then we can upload now
    //     // and remember the location for re-use on any RenderStep that does shading.
    //     UniquePaintParamsID shaderID;
    //     UniformDataCache::Index shadingUniformIndex = UniformDataCache::kInvalidIndex;
    //     TextureDataBlock paintTextures;

    //     if (draw.fPaintParams.has_value()) {
    //         shaderID = ExtractPaintData(recorder,
    //                                     &gatherer,
    //                                     &builder,
    //                                     uniformLayout,
    //                                     draw.fDrawParams.transform(),
    //                                     draw.fPaintParams.value(),
    //                                     draw.fDrawParams.geometry(),
    //                                     targetInfo.colorInfo());

    //         if (shaderID.isValid()) {
    //             if (gatherer.hasUniforms()) {
    //                 shadingUniformIndex =
    //                         shadingUniformDataCache.insert(gatherer.finishUniformDataBlock());
    //             }
    //             if (gatherer.hasTextures()) {
    //                 paintTextures = textureDataCache->insert(gatherer.textureDataBlock());
    //             }
    //         }
    //     } // else depth-only

    //     // Create a sort key for every render step in this draw, extracting out any
    //     // RenderStep-specific data.
    //     for (int stepIndex = 0; stepIndex < draw.fRenderer->numRenderSteps(); ++stepIndex) {
    //         const RenderStep* const step = draw.fRenderer->steps()[stepIndex];
    //         const bool performsShading = draw.fPaintParams.has_value() && step->performsShading();

    //         GraphicsPipelineCache::Index pipelineIndex = pipelineCache.insert(
    //                 {step, performsShading ? shaderID : UniquePaintParamsID::InvalidID()});

    //         gatherer.resetWithNewLayout(uniformLayout);
    //         step->writeUniformsAndTextures(draw.fDrawParams, &gatherer);

    //         UniformDataCache::Index geomUniformIndex =
    //                 gatherer.hasUniforms()
    //                         ? geometryUniformDataCache.insert(gatherer.finishUniformDataBlock())
    //                         : UniformDataCache::kInvalidIndex;

    //         TextureDataBlock stepTextures =
    //                 gatherer.hasTextures() ? textureDataCache->insert(gatherer.textureDataBlock())
    //                                        : TextureDataBlock();
    //         TextureBindingCache::Index textureIndex = textureBindingTracker.trackTextures(
    //                 performsShading ? paintTextures : TextureDataBlock(), stepTextures);

    //         keys.push_back({&draw, stepIndex, pipelineIndex,
    //                         geomUniformIndex, shadingUniformIndex, textureIndex});
    //     }
    // }

    if (!gradientBufferTracker.writeData(gatherer.gradientBufferData(), bufferMgr)) {
        // The necessary uniform data couldn't be written to the GPU, so the DrawPass is invalid.
        // Early out now since the next Recording snap will fail.
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
    static_assert(sizeof(DrawList::SortKey) ==
                  SkAlignTo(16 + sizeof(void*), alignof(DrawList::SortKey)));
    // TODO: Explore sorting algorithms; in all likelihood this will be mostly sorted already, so
    // algorithms that approach O(n) in that condition may be favorable. Alternatively, could
    // explore radix sort that is always O(n). Brief testing suggested std::sort was faster than
    // std::stable_sort and SkTQSort on my [ml]'s Windows desktop. Also worth considering in-place
    // vs. algorithms that require an extra O(n) storage.
    // TODO: It's not strictly necessary, but would a stable sort be useful or just end up hiding
    // bugs in the DrawOrder determination code?
    std::sort(draws->fSortKeys.begin(), draws->fSortKeys.end());

    // Used to record vertex/instance data, buffer binds, and draw calls
    DrawWriter drawWriter(&drawPass->fCommandList, bufferMgr);
    GraphicsPipelineCache::Index lastPipeline = GraphicsPipelineCache::kInvalidIndex;
    SkIRect lastScissor = SkIRect::MakeSize(targetInfo.dimensions());

    SkASSERT(drawPass->fTarget->isFullyLazy() ||
             SkIRect::MakeSize(drawPass->fTarget->dimensions()).contains(lastScissor));
    drawPass->fCommandList.setScissor(lastScissor);

    // All large gradients pack their data into a single buffer throughout the draw pass,
    // therefore the gradient buffer only needs to be bound once.
    gradientBufferTracker.bindIfNeeded(&drawPass->fCommandList);

    UniformTracker geometryUniformTracker(useStorageBuffers);
    UniformTracker shadingUniformTracker(useStorageBuffers);

    for (const DrawList::SortKey& key : keys) {
        const DrawList::Draw& draw = key.draw();
        const RenderStep& renderStep = key.renderStep();

        const bool pipelineChange = key.pipelineIndex() != lastPipeline;

        const bool geomBindingChange = geometryUniformTracker.writeUniforms(
                geometryUniformDataCache, bufferMgr, key.geometryUniformIndex());
        const bool shadingBindingChange = shadingUniformTracker.writeUniforms(
                shadingUniformDataCache, bufferMgr, key.shadingUniformIndex());

        const bool textureBindingsChange = textureBindingTracker.setCurrentTextureBindings(
                key.textureBindingIndex());
        const SkIRect* newScissor        = draw.fDrawParams.clip().scissor() != lastScissor ?
                &draw.fDrawParams.clip().scissor() : nullptr;

        const bool stateChange = geomBindingChange ||
                                 shadingBindingChange ||
                                 textureBindingsChange ||
                                 SkToBool(newScissor);

        // Update DrawWriter *before* we actually change any state so that accumulated draws from
        // the previous state use the proper state.
        if (pipelineChange) {
            drawWriter.newPipelineState(renderStep.primitiveType(),
                                        renderStep.vertexStride(),
                                        renderStep.instanceStride());
        } else if (stateChange) {
            drawWriter.newDynamicState();
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
            if (newScissor) {
                drawPass->fCommandList.setScissor(*newScissor);
                lastScissor = *newScissor;
            }
        }

        uint32_t geometrySsboIndex = useStorageBuffers ? geometryUniformTracker.ssboIndex() : 0;
        uint32_t shadingSsboIndex = useStorageBuffers ? shadingUniformTracker.ssboIndex() : 0;
        skvx::uint2 ssboIndices = {geometrySsboIndex, shadingSsboIndex};
        renderStep.writeVertices(&drawWriter, draw.first, ssboIndices);

        if (bufferMgr->hasMappingFailed()) {
            SKGPU_LOG_W("Failed to write necessary vertex/instance data for DrawPass, dropping!");
            return nullptr;
        }
    }
    // Finish recording draw calls for any collected data at the end of the loop
    drawWriter.flush();

    drawPass->fBounds = draws->fDrawBounds.roundOut().asSkIRect();

    drawPass->fPipelineDescs   = draws->fPipelineCache.detach();
    drawPass->fSamplerDescs    = draws->fTextureBindingCache.detachSamplers();
    drawPass->fSampledTextures = draws->fTextureBindingCache.detachTextures();

    TRACE_COUNTER1("skia.gpu", "# pipelines", drawPass->fPipelineDescs.size());
    TRACE_COUNTER1("skia.gpu", "# textures", drawPass->fSampledTextures.size());
    TRACE_COUNTER1("skia.gpu", "# commands", drawPass->fCommandList.count());

    return drawPass;
}

} // namespace skgpu::graphite
