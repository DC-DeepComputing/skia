/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/encoder/SkPngRustEncoder.h"

#include "include/codec/SkCodec.h"
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
        SkBitmap roundtripCopy = copy_bitmap(roundtrip, colorType);
        success = ToolUtils::DecodeDataToBitmap(dst.detachAsData(), &roundtripCopy);
        REPORTER_ASSERT(r, success);
        if (!success) {
            return;
        }

        success = ToolUtils::equal_pixels(copy, roundtripCopy);
        REPORTER_ASSERT(r, success, "colorType=%d", static_cast<int>(colorType));
    }
}

DEF_TEST(RustEncodePng_colortypes, r) {
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
        SkCodec::Result res = SkCodec::kSuccess;
        std::unique_ptr<SkCodec> roundtripCodec = SkPngRustDecoder::Decode(dst, &res);
        REPORTER_ASSERT(r, success);
        if (res != SkCodec::kSuccess) {
            return;
        }
        SkImageInfo roundtripInfo = roundtripCodec->getInfo().makeColorType(colorType);
        roundtrip.allocPixels(roundtripInfo);
        roundtripCodec->getPixels(
        roundtripInfo, roundtrip.getPixels(), roundtrip.rowBytes());

        bool shouldContinue = false;
        for (int y = 0; y < copy.height(); y++) {
            if (shouldContinue) { break; }
            for (int x = 0; x < copy.width(); x++) {
                SkColor originalColor = copy.getColor(x, y);
                SkColor roundtripColor = roundtrip.getColor(x, y);
                SkPMColor originalPremulColor = SkPreMultiplyColor(originalColor);
                SkPMColor roundtripPremulColor = SkPreMultiplyColor(roundtripColor);
                bool almost_same = almost_equals(originalPremulColor,
                                                 roundtripPremulColor,
                                                 /*tolerance=*/0);
                REPORTER_ASSERT(r,
                                almost_same,
                                "x=%d, y=%d, original=0x%08x, roundtrip=0x%08x, color=%d",
                                x,
                                y,
                                originalPremulColor,
                                roundtripPremulColor,
                                static_cast<int>(colorType));
                if (!almost_same) {
                    shouldContinue = true;
                    break;
                }
            }
        }
    }
}
