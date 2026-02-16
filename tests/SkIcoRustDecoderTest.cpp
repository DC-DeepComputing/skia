/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_ico/decoder/SkIcoRustDecoder.h"

#include <memory>
#include <utility>

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "tests/ComparePixels.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult) \
    REPORTER_ASSERT(r, actualResult == SkCodec::kSuccess, \
                    "actualResult=\"%s\" != kSuccess", \
                    SkCodec::ResultToString(actualResult))

// Helper wrapping a call to `SkIcoRustDecoder::Decode`.
static std::unique_ptr<SkCodec> decode_ico(skiatest::Reporter* r, const char* path) {
    skiatest::ReporterContext ctx(r, path);
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return nullptr;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    return codec;
}

// Test decoding a valid ICO file with multiple BMP images.
DEF_TEST(RustIcoCodec_decode_multi_bmp, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/color_wheel.ico");
    if (!codec) {
        return;
    }

    // color_wheel.ico has 5 images, largest should be reported
    SkISize dimensions = codec->dimensions();
    REPORTER_ASSERT(r, dimensions.width() > 0, "width=%d", dimensions.width());
    REPORTER_ASSERT(r, dimensions.height() > 0, "height=%d", dimensions.height());

    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
}

// Test decoding a valid ICO file with PNG images (google_chrome.ico has PNG).
DEF_TEST(RustIcoCodec_decode_with_png, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/google_chrome.ico");
    if (!codec) {
        return;
    }

    // google_chrome.ico has 9 images including 256x256 PNG
    SkISize dimensions = codec->dimensions();
    REPORTER_ASSERT(r, dimensions.width() == 256,
                    "Expected 256, got width=%d", dimensions.width());
    REPORTER_ASSERT(r, dimensions.height() == 256,
                    "Expected 256, got height=%d", dimensions.height());

    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r, image->width() == 256);
    REPORTER_ASSERT(r, image->height() == 256);
}

// Test that Decode handles nullptr for the Result parameter.
DEF_TEST(RustIcoCodec_nullptr_result, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.ico");
    REPORTER_ASSERT(r, data);

    // This should not crash even when result is nullptr
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), nullptr);
    REPORTER_ASSERT(r, codec);
}

// Test that SkIcoRustDecoder correctly rejects non-ICO data.
DEF_TEST(RustIcoCodec_reject_non_ico, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.png");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.png");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

    // Should fail to decode PNG data as ICO
    REPORTER_ASSERT(r, !codec, "SkIcoRustDecoder should reject PNG data");
}

// Test codec reuse functionality.
DEF_TEST(RustIcoCodec_rewind, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/color_wheel.ico");
    if (!codec) {
        return;
    }

    // Decode first time
    auto [image1, result1] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result1);
    REPORTER_ASSERT(r, image1);

    // Decode again using the same codec (tests reuse)
    auto [image2, result2] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result2);
    REPORTER_ASSERT(r, image2);

    // Both images should have the same dimensions
    REPORTER_ASSERT(r,
                    image1->dimensions() == image2->dimensions(),
                    "Images after rewind should have same dimensions");

    // Verify pixel data is identical
    SkBitmap bm1, bm2;
    REPORTER_ASSERT(r, bm1.tryAllocPixels(image1->imageInfo()));
    REPORTER_ASSERT(r, bm2.tryAllocPixels(image2->imageInfo()));
    REPORTER_ASSERT(r, image1->readPixels(nullptr, bm1.pixmap(), 0, 0));
    REPORTER_ASSERT(r, image2->readPixels(nullptr, bm2.pixmap(), 0, 0));

    // Use zero tolerance for exact pixel match
    const float tols[4] = {0, 0, 0, 0};
    auto error = std::function<ComparePixmapsErrorReporter>(
            [&](int x, int y, const float diffs[4]) {
        ERRORF(r, "Pixels differ at (%d, %d) after rewind. Diffs: (%f, %f, %f, %f)",
               x, y, diffs[0], diffs[1], diffs[2], diffs[3]);
    });
    ComparePixels(bm1.pixmap(), bm2.pixmap(), tols, error);
}

// Test that IsIco correctly identifies ICO data.
DEF_TEST(RustIcoCodec_IsIco_positive, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.ico");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.ico");
        return;
    }

    bool isIco = SkIcoRustDecoder::IsIco(data->data(), data->size());
    REPORTER_ASSERT(r, isIco, "IsIco should return true for ICO data");
}

// Test that IsIco correctly rejects non-ICO data.
DEF_TEST(RustIcoCodec_IsIco_negative, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.png");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.png");
        return;
    }

    bool isIco = SkIcoRustDecoder::IsIco(data->data(), data->size());
    REPORTER_ASSERT(r, !isIco, "IsIco should return false for PNG data");
}

// Test IsIco with insufficient data.
DEF_TEST(RustIcoCodec_IsIco_insufficient_data, r) {
    // ICO signature needs at least 4 bytes (00 00 01 00 or 00 00 02 00)
    const uint8_t shortData[] = {0x00, 0x00, 0x01};
    bool isIco = SkIcoRustDecoder::IsIco(shortData, sizeof(shortData));
    REPORTER_ASSERT(r, !isIco, "IsIco should return false for insufficient data");
}

// Test IsIco recognizes CUR files (cursor format, similar to ICO).
DEF_TEST(RustIcoCodec_IsIco_cursor, r) {
    // CUR signature: 00 00 02 00
    const uint8_t curData[] = {0x00, 0x00, 0x02, 0x00, 0x01, 0x00};
    bool isIco = SkIcoRustDecoder::IsIco(curData, sizeof(curData));
    REPORTER_ASSERT(r, isIco, "IsIco should return true for CUR data");
}

// Table-based test for handling invalid/corrupted ICO files.
DEF_TEST(RustIcoCodec_invalid_ico_handling, r) {
    auto test = [&r](const char* description, const char* file) {
        skiatest::ReporterContext ctx(r, description);
        sk_sp<SkData> data = GetResourceAsData(file);
        if (!data) {
            ERRORF(r, "Missing resource: %s", file);
            return;
        }

        SkCodec::Result result;
        std::unique_ptr<SkCodec> codec =
                SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

        // If we got a codec, try to decode to ensure we don't crash
        if (codec) {
            auto [image, decodeResult] = codec->getImage();
            // Any result is acceptable as long as we don't crash
            (void)image;
            (void)decodeResult;
        }
    };

    test("zero embedded", "empty_images/zero-embedded.ico");
    test("fuzz0", "invalid_images/ico_fuzz0.ico");
    test("fuzz1", "invalid_images/ico_fuzz1.ico");
    test("leak01", "invalid_images/ico_leak01.ico");
    test("int_overflow", "invalid_images/int_overflow.ico");
    test("mask-bmp-ico", "invalid_images/mask-bmp-ico.ico");
    test("sigabort_favicon", "invalid_images/sigabort_favicon.ico");
    test("sigsegv_favicon", "invalid_images/sigsegv_favicon.ico");
    test("sigsegv_favicon_2", "invalid_images/sigsegv_favicon_2.ico");
    test("b37623797", "invalid_images/b37623797.ico");
    test("b38116746", "invalid_images/b38116746.ico");
}

// Test getPixels with a pre-allocated bitmap.
DEF_TEST(RustIcoCodec_getPixels, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/color_wheel.ico");
    if (!codec) {
        return;
    }

    SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap bitmap;
    bitmap.allocPixels(info);

    SkCodec::Result result = codec->getPixels(bitmap.pixmap());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Verify bitmap dimensions match codec dimensions
    REPORTER_ASSERT(r, bitmap.width() == codec->dimensions().width());
    REPORTER_ASSERT(r, bitmap.height() == codec->dimensions().height());
}

// Test explicit rewind through multiple getPixels calls.
DEF_TEST(RustIcoCodec_explicit_rewind, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/color_wheel.ico");
    if (!codec) {
        return;
    }

    // First decode
    SkBitmap bitmap1;
    bitmap1.allocPixels(codec->getInfo());
    SkCodec::Result result1 = codec->getPixels(bitmap1.pixmap());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result1);

    // Second decode (implicitly rewinds internally)
    SkBitmap bitmap2;
    bitmap2.allocPixels(codec->getInfo());
    SkCodec::Result result2 = codec->getPixels(bitmap2.pixmap());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result2);

    // Verify dimensions match
    REPORTER_ASSERT(r, bitmap1.dimensions() == bitmap2.dimensions(),
                    "Dimensions should match after rewind");

    // Verify pixel data matches
    REPORTER_ASSERT(r, bitmap1.computeByteSize() == bitmap2.computeByteSize(),
                    "Byte sizes should match");
    REPORTER_ASSERT(r,
                    memcmp(bitmap1.getPixels(), bitmap2.getPixels(),
                           bitmap1.computeByteSize()) == 0,
                    "Pixel data should match after rewind");
}

// Test decoding ICO files with non-standard (non-square) dimensions.
// These files have non-square aspect ratios, which is unusual for ICO files.
DEF_TEST(RustIcoCodec_nonstandard_dimensions, r) {
    struct TestCase {
        const char* file;
        int expectedWidth;
        int expectedHeight;
    };
    const TestCase testCases[] = {
        {"images/ico_nonsquare_48x32.ico", 48, 32},
        {"images/ico_nonsquare_64x48.ico", 64, 48},
        {"images/ico_nonsquare_128x96.ico", 128, 96},
        {"images/ico_nonsquare_200x150.ico", 200, 150},
    };

    for (const auto& testCase : testCases) {
        skiatest::ReporterContext ctx(r, testCase.file);
        std::unique_ptr<SkCodec> codec = decode_ico(r, testCase.file);
        if (!codec) {
            continue;
        }

        SkISize dimensions = codec->dimensions();
        REPORTER_ASSERT(r, dimensions.width() == testCase.expectedWidth,
                        "Expected width=%d, got width=%d",
                        testCase.expectedWidth, dimensions.width());
        REPORTER_ASSERT(r, dimensions.height() == testCase.expectedHeight,
                        "Expected height=%d, got height=%d",
                        testCase.expectedHeight, dimensions.height());

        auto [image, result] = codec->getImage();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, image);
        REPORTER_ASSERT(r, image->width() == testCase.expectedWidth);
        REPORTER_ASSERT(r, image->height() == testCase.expectedHeight);
    }
}

// Test that ICO files with oversized dimensions (>256, requiring PNG payload) are rejected.
// The Rust ICO decoder currently only supports BMP payloads with dimensions <= 256.
DEF_TEST(RustIcoCodec_reject_oversized, r) {
    sk_sp<SkData> data = GetResourceAsData("images/ico_oversized_512x384.ico");
    if (!data) {
        ERRORF(r, "Missing resource: images/ico_oversized_512x384.ico");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

    // Should fail to decode ICO with PNG payload (oversized dimensions)
    REPORTER_ASSERT(r, !codec,
                    "SkIcoRustDecoder should reject ICO files with oversized dimensions");
}
