
/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImageFilterUtils.h"

#if 0
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkSpecialImage.h"
#include <utility>

namespace skif {

sk_sp<SkImage> MakeWithFilter_Shared(const skif::Functors& functors,
                                     sk_sp<SkImage> src,
                                     const SkImageFilter* filter,
                                     const SkIRect& subset,
                                     const SkIRect& clipBounds,
                                     SkIRect* outSubset,
                                     SkIPoint* offset) {
    if (!filter || !outSubset || !offset || !src->bounds().contains(subset)) {
        return nullptr;
    }

    sk_sp<SkImageFilterCache> cache(
            SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize));

    static const SkSurfaceProps kDefaultSurfaceProps;

    auto srcSpecialImage = functors.fMakeImageFunctor(subset, src, kDefaultSurfaceProps);
    if (!srcSpecialImage) {
        return nullptr;
    }

    // The filters operate in the local space of the src image, where (0,0) corresponds to the
    // subset's top left corner. But the clip bounds and any crop rects on the filters are in the
    // original coordinate system, so configure the CTM to correct crop rects and explicitly adjust
    // the clip bounds (since it is assumed to already be in image space).
    // TODO: Once all image filters support it, we can just use the subset's top left corner as
    // the source FilterResult's origin.
    skif::ContextInfo ctxInfo = {
            skif::Mapping(SkMatrix::Translate(-subset.x(), -subset.y())),
            skif::LayerSpace<SkIRect>(clipBounds.makeOffset(-subset.topLeft())),
            // TODO: Pass subset.topLeft() as the origin of the source FilterResult
            /* fSource= */skif::FilterResult{std::move(srcSpecialImage)},
            src->imageInfo().colorType(),
            src->imageInfo().colorSpace(),
            kDefaultSurfaceProps,
            cache.get()};
    const skif::Context context(ctxInfo, functors);

    sk_sp<SkSpecialImage> result = as_IFB(filter)->filterImage(context)
                                                  .imageAndOffset(context, offset);
    if (!result) {
        return nullptr;
    }

    // The output image and offset are relative to the subset rectangle, so the offset needs to
    // be shifted to put it in the correct spot with respect to the original coordinate system
    offset->fX += subset.x();
    offset->fY += subset.y();

    // Final clip against the exact clipBounds (the clip provided in the context gets adjusted
    // to account for pixel-moving filters so doesn't always exactly match when finished). The
    // clipBounds are translated into the clippedDstRect coordinate space, including the
    // result->subset() ensures that the result's image pixel origin does not affect results.
    SkIRect dstRect = result->subset();
    SkIRect clippedDstRect = dstRect;
    if (!clippedDstRect.intersect(clipBounds.makeOffset(result->subset().topLeft() - *offset))) {
        return nullptr;
    }

    // Adjust the geometric offset if the top-left corner moved as well
    offset->fX += (clippedDstRect.x() - dstRect.x());
    offset->fY += (clippedDstRect.y() - dstRect.y());
    *outSubset = clippedDstRect;
    return result->asImage();
}

} // namespace skif
#endif
