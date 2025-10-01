/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/encode/SkPngRustEncoder.h"

#include <memory>

#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkEncoder.h"
#include "src/encode/SkPngRustEncoderImpl.h"

namespace SkPngRustEncoder {

bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options) {
    std::unique_ptr<SkEncoder> encoder = Make(dst, src, options);
    return encoder && encoder->encodeRows(src.height());
}

sk_sp<SkData> Encode(GrDirectContext* ctx, const SkImage* img, const Options& options) {
    if (!img) {
        return nullptr;
    }

    sk_sp<SkImage> raster_image = img->makeRasterImage(ctx);
    if (!raster_image) {
        return nullptr;
    }

    SkPixmap pixmap;
    bool success = raster_image->peekPixels(&pixmap);

    // `peekPixels` should always succeed for raster images.
    SkASSERT_RELEASE(success);

    SkDynamicMemoryWStream stream;
    if (!SkPngRustEncoder::Encode(&stream, pixmap, options)) {
        return nullptr;
    }
    return stream.detachAsData();
}

std::unique_ptr<SkEncoder> Make(SkWStream* dst, const SkPixmap& src, const Options& options) {
    return SkPngRustEncoderImpl::Make(dst, src, options);
}

}  // namespace SkPngRustEncoder
