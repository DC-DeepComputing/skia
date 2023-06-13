/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/SlugImpl.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkDevice.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/GlyphRun.h"
#include "src/text/gpu/SDFTControl.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"

#include <memory>
#include <utility>

class SkMaskFilter;
class SkStrikeClient;

namespace sktext {

SlugImpl::SlugImpl(SubRunAllocator&& alloc,
                   gpu::SubRunContainerOwner subRuns,
                   SkRect sourceBounds,
                   const SkPaint& paint,
                   SkPoint origin)
        : fAlloc {std::move(alloc)}
        , fSubRuns(std::move(subRuns))
        , fSourceBounds{sourceBounds}
        , fInitialPaint{paint}
        , fOrigin{origin} {}

#if defined(SK_GANESH)
void SlugImpl::surfaceDraw(SkCanvas* canvas,
                           const GrClip* clip,
                           const SkMatrixProvider& viewMatrix,
                           const SkPaint& drawingPaint,
                           skgpu::ganesh::SurfaceDrawContext* sdc) const {
    fSubRuns->draw(canvas, clip, viewMatrix, fOrigin, drawingPaint, this, sdc);
}
#endif

#if defined(SK_GRAPHITE)
void SlugImpl::surfaceDraw(SkCanvas* canvas,
                           const SkPaint& paint,
                           skgpu::graphite::Device* device) const {
    fSubRuns->draw(canvas, fOrigin, paint, this, device);
}
#endif

void SlugImpl::doFlatten(SkWriteBuffer& buffer) const {
    buffer.writeRect(fSourceBounds);
    SkPaintPriv::Flatten(fInitialPaint, buffer);
    buffer.writePoint(fOrigin);
    fSubRuns->flattenAllocSizeHint(buffer);
    fSubRuns->flattenRuns(buffer);
}

sk_sp<Slug> SlugImpl::MakeFromBuffer(SkReadBuffer& buffer, const SkStrikeClient* client) {
    SkRect sourceBounds = buffer.readRect();
    SkASSERT(!sourceBounds.isEmpty());
    if (!buffer.validate(!sourceBounds.isEmpty())) { return nullptr; }
    SkPaint paint = buffer.readPaint();
    SkPoint origin = buffer.readPoint();
    int allocSizeHint = gpu::SubRunContainer::AllocSizeHintFromBuffer(buffer);

    auto [initializer, _, alloc] =
            SubRunAllocator::AllocateClassMemoryAndArena<SlugImpl>(allocSizeHint);

    gpu::SubRunContainerOwner container = gpu::SubRunContainer::MakeFromBufferInAlloc(buffer, client, &alloc);

    // Something went wrong while reading.
    SkASSERT(buffer.isValid());
    if (!buffer.isValid()) { return nullptr;}

    return sk_sp<SlugImpl>(initializer.initialize(
            std::move(alloc), std::move(container), sourceBounds, paint, origin));
}

SkMatrix position_matrix(const SkMatrix& drawMatrix, SkPoint drawOrigin) {
    SkMatrix position_matrix = drawMatrix;
    return position_matrix.preTranslate(drawOrigin.x(), drawOrigin.y());
}

sk_sp<SlugImpl> SlugImpl::Make(const SkMatrixProvider& viewMatrix,
                               const sktext::GlyphRunList& glyphRunList,
                               const SkPaint& initialPaint,
                               const SkPaint& drawingPaint,
                               SkStrikeDeviceInfo strikeDeviceInfo,
                               sktext::StrikeForGPUCacheInterface* strikeCache) {
    size_t subRunSizeHint = gpu::SubRunContainer::EstimateAllocSize(glyphRunList);
    auto [initializer, _, alloc] =
            SubRunAllocator::AllocateClassMemoryAndArena<SlugImpl>(subRunSizeHint);

    const SkMatrix positionMatrix =
            position_matrix(viewMatrix.localToDevice(), glyphRunList.origin());

    auto subRuns = gpu::SubRunContainer::MakeInAlloc(glyphRunList,
                                                positionMatrix,
                                                drawingPaint,
                                                strikeDeviceInfo,
                                                strikeCache,
                                                &alloc,
                                                gpu::SubRunContainer::kAddSubRuns,
                                                "Make Slug");

    sk_sp<SlugImpl> slug = sk_sp<SlugImpl>(initializer.initialize(
            std::move(alloc),
            std::move(subRuns),
            glyphRunList.sourceBounds(),
            initialPaint,
            glyphRunList.origin()));

    // There is nothing to draw here. This is particularly a problem with RSX form blobs where a
    // single space becomes a run with no glyphs.
    if (slug->fSubRuns->isEmpty()) { return nullptr; }

    return slug;
}

}  // namespace sktext