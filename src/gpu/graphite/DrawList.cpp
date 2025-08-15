/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/DrawList.h"

#include "include/core/SkTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/geom/Geometry.h"

namespace skgpu::graphite {

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

    fDraws.emplace_back(renderer, this->deduplicateTransform(localToDevice),
                        geometry, clip, ordering, paint, stroke);

    // Accumulate renderer information for each draw added to this list
    fRenderStepCount += renderer->numRenderSteps();
    fRequiresMSAA |= renderer->requiresMSAA();
    fDepthStencilFlags |= renderer->depthStencilFlags();
    if (paint && paint->dstReadRequired()) {
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

        if (draw.fPaintParams.has_value()) {
            shaderID = ExtractPaintData(recorder,
                                        recorder->priv().refFloatStorageManager().get(),
                                        &gatherer,
                                        &builder,
                                        uniformLayout,
                                        draw.fDrawParams.transform(),
                                        draw.fPaintParams.value(),
                                        draw.fDrawParams.geometry(),
                                        targetInfo.colorInfo());

            if (shaderID.isValid()) {
                UniformDataBlock paintUniforms = gatherer.endPaintData();
                if (paintUniforms) {
                    shadingUniformIndex = fShadingUniformDataCache.insert(paintUniforms);
                }
            }
        } // else depth-only, no paint data

        // Create a sort key for every render step in this draw, extracting out any
        // RenderStep-specific data.
        for (int stepIndex = 0; stepIndex < draw.fRenderer->numRenderSteps(); ++stepIndex) {
            gatherer.rewindForRenderStep();

            const RenderStep* const step = draw.fRenderer->steps()[stepIndex];
            const bool performsShading = draw.fPaintParams.has_value() && step->performsShading();

            GraphicsPipelineCache::Index pipelineIndex = fPipelineCache.insert(
                    { step->renderStepID(),
                    performsShading ? shaderID : UniquePaintParamsID::Invalid() });

            step->writeUniformsAndTextures(draw.fDrawParams, &gatherer);
            auto [stepUniforms, combinedTextures] = gatherer.endRenderStepData(performsShading);

            UniformDataCache::Index geomUniformIndex = stepUniforms ?
                    fGeometryUniformDataCache.insert(stepUniforms) : UniformDataCache::kInvalidIndex;
            TextureDataCache::Index textureBindingIndex = combinedTextures ?
                    fTextureDataCache.insert(combinedTextures) : TextureDataCache::kInvalidIndex;

            fSortKeys.push_back({&draw, stepIndex, pipelineIndex,
                                 geomUniformIndex, shadingUniformIndex, textureBindingIndex});
        }

        fPassBounds.join(draw.fDrawParams.clip().drawBounds());
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
