/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/image/SkImage_Base.h"

#include <optional>
#include <utility>

class GrDirectContext;
enum SkAlphaType : int;

sk_sp<SkData> SkImage::encodeToData(GrDirectContext* context, SkEncodedImageFormat type,
                                    int quality) const {
    SkBitmap bm;
    if (as_IB(this)->getROPixels(context, &bm)) {
        return SkEncodeBitmap(bm, type, quality);
    }
    return nullptr;
}

sk_sp<SkData> SkImage::encodeToData(GrDirectContext* context) const {
    if (auto encoded = this->refEncodedData()) {
        return encoded;
    }

    return this->encodeToData(context, SkEncodedImageFormat::kPNG, 100);
}

#ifndef SK_IMAGE_READ_PIXELS_DISABLE_LEGACY_API
sk_sp<SkData> SkImage::encodeToData(SkEncodedImageFormat type, int quality) const {
    auto dContext = as_IB(this)->directContext();
    return this->encodeToData(dContext, type, quality);
}

sk_sp<SkData> SkImage::encodeToData() const {
    auto dContext = as_IB(this)->directContext();
    return this->encodeToData(dContext);
}
#endif

sk_sp<SkData> SkImage::refEncodedData() const {
    return sk_sp<SkData>(as_IB(this)->onRefEncoded());
}

sk_sp<SkImage> SkImage::MakeFromEncoded(sk_sp<SkData> encoded,
                                        std::optional<SkAlphaType> alphaType) {
    if (nullptr == encoded || 0 == encoded->size()) {
        return nullptr;
    }
    return SkImage::MakeFromGenerator(
            SkImageGenerator::MakeFromEncoded(std::move(encoded), alphaType));
}
