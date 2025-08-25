/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawList.h"

#include "include/core/SkTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/KeyContext.h"
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

} // namespace

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
                          const StrokeStyle* stroke,
                          bool dependsOnDst,
                          bool dstReadReq) {
    SkASSERT(localToDevice.valid());
    SkASSERT(!geometry.isEmpty() && !clip.drawBounds().isEmptyNegativeOrNaN());
    SkASSERT(!(renderer->depthStencilFlags() & DepthStencilFlags::kStencil) ||
             ordering.stencilIndex() != DrawOrder::kUnassigned);

    // TODO: Add validation that the renderer's expected shape type and stroke params match provided

    fDraws.emplace_back(renderer, this->deduplicateTransform(localToDevice), geometry, clip,
                        ordering, paint, stroke, dependsOnDst, dstReadReq);

    // Accumulate renderer information for each draw added to this list
    fRenderStepCount += renderer->numRenderSteps();
    fRequiresMSAA |= renderer->requiresMSAA();
    fDepthStencilFlags |= renderer->depthStencilFlags();
    if (paint && dstReadReq) {
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


bool DrawList::prepareSortKeys(Recorder* recorder,
                               const SkImageInfo& targetInfo) {
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

    // Geometry uniforms are currently always UBO-backed.
    const Caps* caps = recorder->priv().caps();
    const bool useStorageBuffers = caps->storageBufferSupport();
    const ResourceBindingRequirements& bindingReqs = caps->resourceBindingRequirements();
    Layout uniformLayout =
            useStorageBuffers ? bindingReqs.fStorageBufferLayout : bindingReqs.fUniformBufferLayout;

    ShaderCodeDictionary* dict = recorder->priv().shaderCodeDictionary();
    PaintParamsKeyBuilder builder(dict);

    // The initial layout we pass here is not important as it will be re-assigned when writing
    // shading and geometry uniforms below.
    PipelineDataGatherer gatherer(uniformLayout);
    fSortKeys.reserve(fRenderStepCount);

    for (const DrawList::Draw& draw : fDraws.items()) {
        gatherer.resetForDraw();

        UniquePaintParamsID shaderID = UniquePaintParamsID::Invalid();
        UniformDataCache::Index shadingUniformIndex = UniformDataCache::kInvalidIndex;

        if (draw.paintParams().has_value()) {
            SkDEBUGCODE(builder.checkReset());
            SkDEBUGCODE(gatherer.checkReset());

            auto& geometry = draw.drawParams().geometry();
            KeyContext keyContext(recorder,
                                  recorder->priv().refFloatStorageManager().get(),
                                  &builder,
                                  &gatherer,
                                  draw.drawParams().transform(),
                                  targetInfo.colorInfo(),
                                  geometry.isShape() || geometry.isEdgeAAQuad()
                                    ? KeyGenFlags::kDefault
                                    : KeyGenFlags::kDisableSamplingOptimization,
                                  draw.paintParams().value().color());
#if defined(SK_DEBUG)
            auto result = draw.paintParams().value().toKey(keyContext);
            auto [dependsOnDst, dstReadReq, usesAdvancedBlend] = *result;
#else
            draw.paintParams().value().toKey(keyContext);
#endif
            SkASSERT(dependsOnDst == draw.dependsOnDst());
            SkASSERT(dstReadReq == draw.dstReadReq());
            SkASSERT(usesAdvancedBlend == paint_uses_advanced_blend_equation(draw.paintParams()));

            shaderID = recorder->priv().shaderCodeDictionary()->findOrCreate(&builder);
            if (shaderID.isValid()) {
                UniformDataBlock paintUniforms = gatherer.endPaintData();
                if (paintUniforms) {
                    shadingUniformIndex = fShadingUniformDataCache.insert(paintUniforms);
                }
            }
        } // else depth-only, no paint data

        // Create a sort key for every render step in this draw, extracting out any
        // RenderStep-specific data.
        for (int stepIndex = 0; stepIndex < draw.renderer()->numRenderSteps(); ++stepIndex) {
            gatherer.rewindForRenderStep();

            const RenderStep* const step = draw.renderer()->steps()[stepIndex];
            const bool performsShading = draw.paintParams().has_value() && step->performsShading();

            GraphicsPipelineCache::Index pipelineIndex = fPipelineCache.insert(
                    { step->renderStepID(),
                    performsShading ? shaderID : UniquePaintParamsID::Invalid() });

            step->writeUniformsAndTextures(draw.drawParams(), &gatherer);
            auto [stepUniforms, combinedTextures] = gatherer.endRenderStepData(performsShading);

            UniformDataCache::Index geomUniformIndex = stepUniforms ?
                    fGeometryUniformDataCache.insert(stepUniforms) : UniformDataCache::kInvalidIndex;
            TextureDataCache::Index textureBindingIndex = combinedTextures ?
                    fTextureDataCache.insert(combinedTextures) : TextureDataCache::kInvalidIndex;

            fSortKeys.push_back({&draw, stepIndex, pipelineIndex,
                                 geomUniformIndex, shadingUniformIndex, textureBindingIndex});
        }

        fPassBounds.join(draw.drawParams().clip().drawBounds());
    }

    // TODO: Explore sorting algorithms; in all likelihood this will be mostly sorted already, so
    // algorithms that approach O(n) in that condition may be favorable. Alternatively, could
    // explore radix sort that is always O(n). Brief testing suggested std::sort was faster than
    // std::stable_sort and SkTQSort on my [ml]'s Windows desktop. Also worth considering in-place
    // vs. algorithms that require an extra O(n) storage.
    // TODO: It's not strictly necessary, but would a stable sort be useful or just end up hiding
    // bugs in the DrawOrder determination code?
    std::sort(fSortKeys.begin(), fSortKeys.end());
    return true;
}

} // namespace skgpu::graphite
