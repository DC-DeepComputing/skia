/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/SkPngRustDecoder.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <memory>

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result)                                   \
    do {                                                                                     \
        if (result != SkCodec::kSuccess) {                                                   \
            REPORT_FAILURE(                                                                  \
                    r,                                                                       \
                    #result,                                                                 \
                    SkStringPrintf("actual result is %s", SkCodec::ResultToString(result))); \
        }                                                                                    \
    } while (0)

// Helper wrapping a call to `SkPngRustDecoder::Decode`.
std::unique_ptr<SkCodec> SkPngRustDecoderDecode(skiatest::Reporter* r, const char* path) {
    sk_sp<SkData> data(GetResourceAsData(path));
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return nullptr;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkPngRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    return codec;
}

// * `r` is `skiatest::Reporter*`
// * `color` is `SkColor` (32 bits).
// * `SkColorGetX` is a function that takes `color` and returns a component
//   value (e.g. `SkColorGetR` to get the red channel)
// * `expectedValue` is the expected value of a component retrieved via
//   `SkColorGetX` (8 bits)
#define REPORTER_ASSERT_PIXEL_COMPONENT(r, color, SkColorGetX, expectedValue) \
    REPORTER_ASSERT(r,                                                        \
                    SkColorGetX(color) == expectedValue,                      \
                    "%s=0x%02hhX != expectedValue=0x%02hhX",                  \
                    #SkColorGetX,                                             \
                    SkColorGetX(color),                                       \
                    expectedValue)

void AssertGreenPixel(skiatest::Reporter* r, const SkImage& image, int x, int y) {
    SkASSERT(r);
    SkASSERT(x >= 0);
    SkASSERT(y >= 0);

    REPORTER_ASSERT(r, x < image.width(), "x=%d >= width=%d", x, image.width());
    REPORTER_ASSERT(r, y < image.height(), "y=%d >= height=%d", y, image.height());

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image.peekPixels(&pixmap));

    SkColor color = pixmap.getColor(0, 0);
    REPORTER_ASSERT_PIXEL_COMPONENT(r, color, SkColorGetA, 0xFFu);
    REPORTER_ASSERT_PIXEL_COMPONENT(r, color, SkColorGetR, 0x00u);
    REPORTER_ASSERT_PIXEL_COMPONENT(r, color, SkColorGetG, 0xFFu);
    REPORTER_ASSERT_PIXEL_COMPONENT(r, color, SkColorGetB, 0x00u);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#trivial-animated-image-one-frame-ignoring-default-image
//
// The input file contains the following PNG chunks: IHDR, acTL, IDAT, fcTL,
// fdAT, IEND.  Presence of acTL chunk + no fcTL chunk before IDAT means that
// the IDAT chunk is *not* part of the animation (i.e. APNG-aware decoders
// should ignore IDAT frame and start with fdAT frame).  This test will fail
// with non-APNG-aware decoders (e.g. with `SkPngCodec`), because the `IDAT`
// chunk represents a red image (the `fdAT` chunk represents a green image).
DEF_TEST(Codec_apng_basic_ignoring_default_image, r) {
    std::unique_ptr<SkCodec> codec =
            SkPngRustDecoderDecode(r, "images/apng-test-suite--basic--ignoring-default-image.png");
    REPORTER_ASSERT(r, codec);
    REPORTER_ASSERT(r, codec->getFrameCount() == 1);
    REPORTER_ASSERT(r, codec->getRepetitionCount() == 0);

    auto [image, res] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, res);
    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r, image->width() == 128, "width %d != 128", image->width());
    REPORTER_ASSERT(r, image->height() == 64, "height %d != 64", image->height());
    AssertGreenPixel(r, *image, 0, 0);
}
