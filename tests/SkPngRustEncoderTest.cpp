/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/encoder/SkPngRustEncoder.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "tests/Test.h"
#include "tools/DecodeUtils.h"
#include "tools/ToolUtils.h"

static SkBitmap copy_bitmap(const SkBitmap& src, SkColorType colorType) {
    const SkBitmap* srcPtr = &src;
    SkBitmap tmp(src);
    if (kRGB_565_SkColorType == colorType) {
        tmp.setAlphaType(kOpaque_SkAlphaType);
        srcPtr = &tmp;
    }

    SkBitmap copy;
    ToolUtils::copy_to(&copy, colorType, *srcPtr);
    copy.setImmutable();
    return copy;
}

DEF_TEST(RustEncodePng_smoke_test, r) {
    SkBitmap bitmap;
    bool success = ToolUtils::GetResourceAsBitmap("images/mandrill_128.png", &bitmap);
    if (!success) {
        return;
    }

    for (SkColorType colorType : {kRGBA_8888_SkColorType,
                                  kRGB_565_SkColorType,
                                  kRGBA_F16_SkColorType,
                                  })
    {
        SkBitmap copy = copy_bitmap(bitmap, colorType);
        SkPixmap src;
        success = copy.peekPixels(&src);
        REPORTER_ASSERT(r, success);
        if (!success) {
            return;
        }

        SkDynamicMemoryWStream dst;
        SkPngRustEncoder::Options options;
        success = SkPngRustEncoder::Encode(&dst, src, options);
        REPORTER_ASSERT(r, success);
        if (!success) {
            return;
        }

        SkBitmap roundtrip;
        success = ToolUtils::DecodeDataToBitmap(dst.detachAsData(), &roundtrip);
        REPORTER_ASSERT(r, success);
        if (!success) {
            return;
        }

        success = ToolUtils::equal_pixels(copy, roundtrip);
        REPORTER_ASSERT(r, success);
    }
}
