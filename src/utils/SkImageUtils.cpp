/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/utils/SkImageUtils.h"

namespace SkImageUtils {

sk_sp<SkImage> ScaleImage(const SkImage* image, const SkImageInfo& info,
                          const SkSamplingOptions& sampling) {
    // nothing changing, so don't need to make a copy
    if (image->imageInfo() == info) {
        return sk_ref_sp(image);
    }

    auto surf = image->makeSurface(info);
    if (!surf) {
        return nullptr;
    }

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    surf->getCanvas()->drawImageRect(image,
                                     SkRect::MakeIWH(info.width(), info.height()),
                                     sampling,
                                     &paint);
    return surf->makeImageSnapshot();
}

} // namespace
