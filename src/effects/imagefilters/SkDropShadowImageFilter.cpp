/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/imagefilters/SkCropImageFilter.h"

#include <utility>

namespace {

static sk_sp<SkImageFilter> make_drop_shadow_graph(SkVector offset,
                                                   SkSize sigma,
                                                   SkColor color,
                                                   bool shadowOnly,
                                                   sk_sp<SkImageFilter> input,
                                                   const SkRect* crop) {
    // A drop shadow blurs the input, filters it to be the solid color + blurred
    // alpha, and then offsets it. If it's not shadow-only, the input is then
    // src-over blended on top. Finally it's cropped to the optional 'crop'.
    sk_sp<SkImageFilter> filter = input;
    filter = SkImageFilters::Blur(sigma.fWidth, sigma.fHeight, std::move(filter));
    filter = SkImageFilters::ColorFilter(
            SkColorFilters::Blend(color, SkBlendMode::kSrcIn),
            std::move(filter));
    filter = SkImageFilters::Offset(offset.fX, offset.fY, std::move(filter));
    if (!shadowOnly) {
        filter = SkImageFilters::Blend(
                SkBlendMode::kSrcOver, std::move(filter), std::move(input));
    }
    if (crop) {
        filter = SkMakeCropImageFilter(*crop, std::move(filter));
    }
    return filter;
}

sk_sp<SkFlattenable> legacy_drop_shadow_create_proc(SkReadBuffer& buffer) {
    auto [child, cropRect] = SkImageFilter_Base::Unflatten(buffer);

    SkScalar dx = buffer.readScalar();
    SkScalar dy = buffer.readScalar();
    SkScalar sigmaX = buffer.readScalar();
    SkScalar sigmaY = buffer.readScalar();
    SkColor color = buffer.readColor();

    // For backwards compatibility, the shadow mode had been saved as an enum cast to a 32LE int,
    // where shadow-and-foreground was 0 and shadow-only was 1. Other than the number of bits, this
    // is equivalent to the bool that SkDropShadowImageFilter now uses.
    bool shadowOnly = SkToBool(buffer.read32LE(1));
    return make_drop_shadow_graph({dx, dy}, {sigmaX, sigmaY}, color, shadowOnly,
                                  std::move(child), cropRect ? &*cropRect : nullptr);
}

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::DropShadow(
        SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor color,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_drop_shadow_graph({dx, dy}, {sigmaX, sigmaY}, color, /*shadowOnly=*/false,
                                  std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::DropShadowOnly(
        SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor color,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_drop_shadow_graph({dx, dy}, {sigmaX, sigmaY}, color, /*shadowOnly=*/true,
                                  std::move(input), cropRect);
}

// TODO (michaelludwig) - Remove after grace period for SKPs to stop using old create proc
void SkRegisterLegacyDropShadowImageFilterFlattenable() {
    SkFlattenable::Register("SkDropShadowImageFilter", legacy_drop_shadow_create_proc);
    SkFlattenable::Register("SkDropShadowImageFilterImpl", legacy_drop_shadow_create_proc);
}
