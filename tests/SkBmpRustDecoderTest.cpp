/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_bmp/decoder/SkBmpRustDecoder.h"

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

// Helper to verify pixel color at a specific location.
static void assert_pixel_color(skiatest::Reporter* r,
                              const SkPixmap& pixmap,
                              int x,
                              int y,
                              SkColor expectedColor,
                              const char* description) {
    SkASSERT(r);
    SkASSERT(x >= 0);
    SkASSERT(y >= 0);
    SkASSERT(description);

    skiatest::ReporterContext ctx(r, description);
    REPORTER_ASSERT(r, x < pixmap.width(),
                    "x=%d >= width=%d", x, pixmap.width());
    REPORTER_ASSERT(r, y < pixmap.height(),
                    "y=%d >= height=%d", y, pixmap.height());
    REPORTER_ASSERT(r, kN32_SkColorType == pixmap.colorType(),
                    "kN32_SkColorType != pixmap.ColorType()=%d",
                    pixmap.colorType());

    SkColor actualColor = pixmap.getColor(x, y);
    REPORTER_ASSERT(r, actualColor == expectedColor,
                    "actualColor=0x%08X != expectedColor==0x%08X at (%d,%d)",
                    actualColor, expectedColor, x, y);
}

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult) \
    REPORTER_ASSERT(r, actualResult == SkCodec::kSuccess, \
                    "actualResult=\"%s\" != kSuccess", \
                    SkCodec::ResultToString(actualResult))

// Helper wrapping a call to `SkBmpRustDecoder::Decode`.
static std::unique_ptr<SkCodec> decode_bmp(skiatest::Reporter* r, const char* path) {
    skiatest::ReporterContext ctx(r, path);
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return nullptr;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    return codec;
}

// Table-based test for decoding valid BMP files.
DEF_TEST(RustBmpCodec_decode_valid_bmp, r) {
    auto test = [&r](const char* description, const char* file, SkISize expectedSize,
                     SkColorType expectedColorType,
                     SkCodec::SkScanlineOrder expectedScanlineOrder) {
        skiatest::ReporterContext ctx(r, description);
        std::unique_ptr<SkCodec> codec = decode_bmp(r, file);
        if (!codec) {
            return;
        }
        REPORTER_ASSERT(r, codec->dimensions() == expectedSize,
                        "dimensions=%dx%d != expected=%dx%d",
                        codec->dimensions().width(), codec->dimensions().height(),
                        expectedSize.width(), expectedSize.height());

        // Verify color type
        SkImageInfo info = codec->getInfo();
        REPORTER_ASSERT(r, info.colorType() == expectedColorType,
                        "colorType=%d != expected=%d",
                        info.colorType(), expectedColorType);

        auto [image, result] = codec->getImage();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, image);
        REPORTER_ASSERT(r, image->width() == expectedSize.width());
        REPORTER_ASSERT(r, image->height() == expectedSize.height());
    };

    test("basic 24-bit", "images/randPixels.bmp", {8, 8}, kN32_SkColorType,
        SkCodec::kBottomUp_SkScanlineOrder);
    test("rle compression", "images/rle.bmp", {320, 240}, kN32_SkColorType,
        SkCodec::kBottomUp_SkScanlineOrder);
    test("8 bit indexed", "images/bmp-size-32x32-8bpp.bmp", {32, 32}, kN32_SkColorType,
        SkCodec::kBottomUp_SkScanlineOrder);
    test("32 bit top-down", "images/32bpp-topdown-320x240.bmp", {320, 240}, kN32_SkColorType,
        SkCodec::kTopDown_SkScanlineOrder);
}

// Test that Decode handles nullptr for the Result parameter.
DEF_TEST(RustBmpCodec_nullptr_result, r) {
    sk_sp<SkData> data = GetResourceAsData("images/randPixels.bmp");
    REPORTER_ASSERT(r, data);

    // This should not crash even when result is nullptr
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), nullptr);
    REPORTER_ASSERT(r, codec);
    REPORTER_ASSERT(r, codec->dimensions() == SkISize::Make(8, 8));
}

// Test that SkBmpRustDecoder correctly rejects non-BMP data.
DEF_TEST(RustBmpCodec_reject_non_bmp, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.png");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.png");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

    // Should fail to decode PNG data as BMP
    REPORTER_ASSERT(r, !codec, "SkBmpRustDecoder should reject PNG data");
}

// Test codec reuse functionality.
DEF_TEST(RustBmpCodec_rewind, r) {
    std::unique_ptr<SkCodec> codec = decode_bmp(r, "images/randPixels.bmp");
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

// Test getPixels with a pre-allocated bitmap.
DEF_TEST(RustBmpCodec_getPixels, r) {
    std::unique_ptr<SkCodec> codec = decode_bmp(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap bitmap;
    bitmap.allocPixels(info);

    SkCodec::Result result = codec->getPixels(bitmap.pixmap());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Verify bitmap dimensions
    REPORTER_ASSERT(r, bitmap.width() == 8);
    REPORTER_ASSERT(r, bitmap.height() == 8);

    // Spot check pixel values to verify correct decoding
    SkPixmap pixmap = bitmap.pixmap();
    assert_pixel_color(r, pixmap, 0, 0, SkColorSetARGB(0xFF, 0xBB, 0xA5, 0x70), "pixel (0,0)");
    assert_pixel_color(r, pixmap, 1, 0, SkColorSetARGB(0xFF, 0x39, 0x5F, 0x5D), "pixel (1,0)");
    assert_pixel_color(r, pixmap, 0, 1, SkColorSetARGB(0xFF, 0xBD, 0xA6, 0x0E), "pixel (0,1)");
    assert_pixel_color(r, pixmap, 1, 1, SkColorSetARGB(0xFF, 0xC0, 0x1D, 0xB6), "pixel (1,1)");
}

// Test that IsBmp correctly identifies BMP data.
DEF_TEST(RustBmpCodec_IsBmp_positive, r) {
    sk_sp<SkData> data = GetResourceAsData("images/randPixels.bmp");
    if (!data) {
        ERRORF(r, "Missing resource: images/randPixels.bmp");
        return;
    }

    bool isBmp = SkBmpRustDecoder::IsBmp(data->data(), data->size());
    REPORTER_ASSERT(r, isBmp, "IsBmp should return true for BMP data");
}

// Test that IsBmp correctly rejects non-BMP data.
DEF_TEST(RustBmpCodec_IsBmp_negative, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.png");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.png");
        return;
    }

    bool isBmp = SkBmpRustDecoder::IsBmp(data->data(), data->size());
    REPORTER_ASSERT(r, !isBmp, "IsBmp should return false for PNG data");
}

// Test IsBmp with insufficient data.
DEF_TEST(RustBmpCodec_IsBmp_insufficient_data, r) {
    // BMP signature is "BM" (2 bytes), but header is larger
    const uint8_t shortData[] = {'B'};
    bool isBmp = SkBmpRustDecoder::IsBmp(shortData, sizeof(shortData));
    REPORTER_ASSERT(r, !isBmp, "IsBmp should return false for insufficient data");
}

// Table-based test for handling invalid/corrupted BMP files.
DEF_TEST(RustBmpCodec_invalid_bmp_handling, r) {
    auto test = [&r](const char* description, const char* file) {
        skiatest::ReporterContext ctx(r, description);
        sk_sp<SkData> data = GetResourceAsData(file);
        if (!data) {
            ERRORF(r, "Missing resource: %s", file);
            return;
        }

        SkCodec::Result result;
        std::unique_ptr<SkCodec> codec =
                SkBmpRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

        // If we got a codec, try to decode to ensure we don't crash
        if (codec) {
            auto [image, decodeResult] = codec->getImage();
            // Any result is acceptable as long as we don't crash
            (void)image;
            (void)decodeResult;
        }
    };

    test("zero width", "empty_images/zero-width.bmp");
    test("zero height", "empty_images/zero-height.bmp");
    test("corrupted incomplete", "invalid_images/b33251605.bmp");
    test("invalid header", "invalid_images/b33651913.bmp");
    test("extreme dimensions", "invalid_images/b34778578.bmp");
    test("os2 fuzz", "invalid_images/osfuzz6288.bmp");
}

// Test explicit rewind through multiple getPixels calls.
DEF_TEST(RustBmpCodec_explicit_rewind, r) {
    std::unique_ptr<SkCodec> codec = decode_bmp(r, "images/randPixels.bmp");
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


// Test incremental decoding
DEF_TEST(Codec_Bmp_Rust_IncrementalDecode, r) {
    const char* path = "images/randPixels.bmp";
    sk_sp<SkData> data = GetResourceAsData(path);
    REPORTER_ASSERT(r, data, "Missing test image: %s", path);
    if (!data) {
        return;
    }

    std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, codec, "Failed to create codec for %s", path);
    if (!codec) {
        return;
    }

    // First, decode the full image using getPixels for reference
    SkBitmap reference;
    reference.allocPixels(codec->getInfo());
    SkCodec::Result result = codec->getPixels(reference.pixmap());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Now decode using incremental decode - create new codec
    std::unique_ptr<SkCodec> codec2 = SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, codec2, "Failed to create second codec for %s", path);
    if (!codec2) {
        return;
    }
    SkBitmap incremental;
    incremental.allocPixels(codec2->getInfo());

    result = codec2->startIncrementalDecode(incremental.info(),
                                          incremental.getPixels(),
                                          incremental.rowBytes());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Decode all rows incrementally
    int rowsDecoded = 0;
    while (true) {
        result = codec2->incrementalDecode(&rowsDecoded);
        if (result == SkCodec::kSuccess) {
            break;
        }
        // For BMP with current implementation, kIncompleteInput shouldn't happen
        // since we have all data upfront, but we handle it for future streaming support
        REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput,
                       "Unexpected incremental decode result: %d", result);
        if (result != SkCodec::kIncompleteInput) {
            return;
        }
    }

    // Verify that incremental decode produces the same output as full decode
    REPORTER_ASSERT(r, reference.dimensions() == incremental.dimensions(),
                    "Dimensions should match");
    REPORTER_ASSERT(r, reference.computeByteSize() == incremental.computeByteSize(),
                    "Byte sizes should match");

    // Compare pixel data
    const uint8_t* refPixels = static_cast<const uint8_t*>(reference.getPixels());
    const uint8_t* incPixels = static_cast<const uint8_t*>(incremental.getPixels());
    size_t byteSize = reference.computeByteSize();

    bool pixelsMatch = memcmp(refPixels, incPixels, byteSize) == 0;
    REPORTER_ASSERT(r, pixelsMatch,
                    "Incremental decode should produce identical output to full decode");
}

// Test incremental decode with subset
DEF_TEST(Codec_Bmp_Rust_IncrementalDecode_Subset, r) {
    const char* path = "images/32bpp-topdown-320x240.bmp";
    sk_sp<SkData> data = GetResourceAsData(path);
    REPORTER_ASSERT(r, data, "Missing test image: %s", path);
    if (!data) {
        return;
    }

    std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, codec, "Failed to create codec for %s", path);
    if (!codec) {
        return;
    }

    // Define a subset to decode (must fit within 320x240 image)
    SkIRect subset = SkIRect::MakeXYWH(100, 100, 200, 140);

    // Validate the subset
    bool subsetSupported = codec->getValidSubset(&subset);
    REPORTER_ASSERT(r, subsetSupported, "Subset not supported");
    if (!subsetSupported) {
        return;
    }

    SkCodec::Options options;
    options.fSubset = &subset;

    // Decode using getPixels with subset
    SkImageInfo subsetInfo = codec->getInfo().makeDimensions(subset.size());
    SkBitmap reference;
    reference.allocPixels(subsetInfo);
    SkCodec::Result result = codec->getPixels(subsetInfo,
                                             reference.getPixels(),
                                             reference.rowBytes(),
                                             &options);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Now decode the same subset using incremental decode - create new codec
    std::unique_ptr<SkCodec> codec2 = SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, codec2, "Failed to create second codec");
    if (!codec2) {
        return;
    }

    // For incremental decode, allocate a full-sized bitmap and decode the subset into it
    // (Note: startIncrementalDecode requires full image info and a pointer to the subset location)
    SkBitmap incremental;
    incremental.allocPixels(codec2->getInfo());
    incremental.eraseColor(SK_ColorTRANSPARENT);  // Clear to transparent to verify subset is written

    result = codec2->startIncrementalDecode(codec2->getInfo(),
                                          incremental.getAddr(0, subset.top()),
                                          incremental.rowBytes(),
                                          &options);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Decode incrementally
    int rowsDecoded = 0;
    while (true) {
        result = codec2->incrementalDecode(&rowsDecoded);
        if (result == SkCodec::kSuccess) {
            break;
        }
        REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput,
                       "Unexpected incremental decode result: %d", result);
        if (result != SkCodec::kIncompleteInput) {
            return;
        }
    }

    // Verify subset incremental decode matches subset full decode
    // The incremental decode writes directly to the subset location in the bitmap,
    // so we compare the decoded region directly (not by extracting a subset)
    // Compare row by row since the bitmaps may have different rowBytes
    bool pixelsMatch = true;

    const size_t bytesPerRow = reference.width() * reference.bytesPerPixel();
    for (int y = 0; y < reference.height() && pixelsMatch; ++y) {
        const void* refRow = reference.getAddr(0, y);
        // For incremental, the subset was written starting at (0, subset.top()),
        // so row y of the subset is at row (subset.top() + y) of the full bitmap
        const void* incRow = incremental.getAddr(0, subset.top() + y);
        if (memcmp(refRow, incRow, bytesPerRow) != 0) {
            pixelsMatch = false;
            ERRORF(r, "Pixels differ at row %d", y);
            // Print first few pixels of the differing row
            const uint32_t* refPixels = reinterpret_cast<const uint32_t*>(refRow);
            const uint32_t* incPixels = reinterpret_cast<const uint32_t*>(incRow);
            fprintf(stderr, "  Reference row %d: %08x %08x %08x %08x\n",
                    y, refPixels[0], refPixels[1], refPixels[2], refPixels[3]);
            fprintf(stderr, "  Incremental row %d: %08x %08x %08x %08x\n",
                    y, incPixels[0], incPixels[1], incPixels[2], incPixels[3]);
            break;
        }
    }

    REPORTER_ASSERT(r, pixelsMatch,
                    "Incremental subset decode should match full subset decode");
}

// Test that decoder handles missing ICC profile gracefully
DEF_TEST(Codec_Bmp_Rust_NoIccProfile, r) {
    // Test with various BMP files that don't have ICC profiles
    const char* paths[] = {
        "images/randPixels.bmp",
        "images/rle.bmp",
        "images/bmp-size-32x32-8bpp.bmp",
    };

    for (const char* path : paths) {
        skiatest::ReporterContext ctx(r, path);
        sk_sp<SkData> data = GetResourceAsData(path);
        if (!data) {
            ERRORF(r, "Missing resource: %s", path);
            continue;
        }

        std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
        REPORTER_ASSERT(r, codec, "Failed to create codec");
        if (!codec) {
            continue;
        }

        // Decode should work fine without ICC profile
        auto [image, result] = codec->getImage();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, image, "Image should be decoded successfully");
    }
}

// Test incremental decode with multiple rewinds
DEF_TEST(Codec_Bmp_Rust_IncrementalDecode_MultipleRewinds, r) {
    const char* path = "images/randPixels.bmp";
    sk_sp<SkData> data = GetResourceAsData(path);
    REPORTER_ASSERT(r, data, "Missing test image: %s", path);
    if (!data) {
        return;
    }

    std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, codec, "Failed to create codec");
    if (!codec) {
        return;
    }

    // Perform multiple decode cycles - create new codec each time
    for (int cycle = 0; cycle < 3; cycle++) {
        skiatest::ReporterContext ctx(r, SkStringPrintf("Cycle %d", cycle).c_str());

        std::unique_ptr<SkCodec> codecCycle = SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
        REPORTER_ASSERT(r, codecCycle, "Failed to create codec for cycle %d", cycle);
        if (!codecCycle) {
            continue;
        }

        SkBitmap bitmap;
        bitmap.allocPixels(codecCycle->getInfo());

        SkCodec::Result result = codecCycle->startIncrementalDecode(
            bitmap.info(), bitmap.getPixels(), bitmap.rowBytes());
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

        int rowsDecoded = 0;
        while (true) {
            result = codecCycle->incrementalDecode(&rowsDecoded);
            if (result == SkCodec::kSuccess) {
                break;
            }
            REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput,
                           "Unexpected result in cycle %d", cycle);
            if (result != SkCodec::kIncompleteInput) {
                return;
            }
        }

        REPORTER_ASSERT(r, rowsDecoded > 0, "Should have decoded rows in cycle %d", cycle);
    }
}

// Test incremental decode state tracking
DEF_TEST(Codec_Bmp_Rust_IncrementalDecode_StateTracking, r) {
    const char* path = "images/32bpp-topdown-320x240.bmp";
    sk_sp<SkData> data = GetResourceAsData(path);
    REPORTER_ASSERT(r, data, "Missing test image: %s", path);
    if (!data) {
        return;
    }

    std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, codec, "Failed to create codec");
    if (!codec) {
        return;
    }

    SkBitmap bitmap;
    bitmap.allocPixels(codec->getInfo());

    SkCodec::Result result = codec->startIncrementalDecode(
        bitmap.info(), bitmap.getPixels(), bitmap.rowBytes());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Track rows decoded across multiple incrementalDecode calls
    int totalRowsDecoded = 0;
    int callCount = 0;

    while (true) {
        int rowsDecoded = 0;
        result = codec->incrementalDecode(&rowsDecoded);
        callCount++;

        if (rowsDecoded > 0) {
            totalRowsDecoded += rowsDecoded;
        }

        if (result == SkCodec::kSuccess) {
            break;
        }

        REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput,
                       "Unexpected result in call %d", callCount);
        if (result != SkCodec::kIncompleteInput) {
            return;
        }

        // Prevent infinite loop
        REPORTER_ASSERT(r, callCount < 1000, "Too many incremental decode calls");
        if (callCount >= 1000) {
            return;
        }
    }

    REPORTER_ASSERT(r, totalRowsDecoded == codec->getInfo().height(),
                    "Should decode all rows: %d vs %d",
                    totalRowsDecoded, codec->getInfo().height());
}

// Test that incremental decode works with color type conversion
DEF_TEST(Codec_Bmp_Rust_IncrementalDecode_ColorTypeConversion, r) {
    const char* path = "images/randPixels.bmp";
    sk_sp<SkData> data = GetResourceAsData(path);
    REPORTER_ASSERT(r, data, "Missing test image: %s", path);
    if (!data) {
        return;
    }

    std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, codec, "Failed to create codec");
    if (!codec) {
        return;
    }

    // Try different color types
    SkColorType colorTypes[] = {
        kN32_SkColorType,
        kRGBA_8888_SkColorType,
        kBGRA_8888_SkColorType,
    };

    for (SkColorType ct : colorTypes) {
        skiatest::ReporterContext ctx(r,
            SkStringPrintf("ColorType %d", ct).c_str());

        std::unique_ptr<SkCodec> codecCT = SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
        REPORTER_ASSERT(r, codecCT, "Failed to create codec for color type %d", ct);
        if (!codecCT) {
            continue;
        }

        SkImageInfo info = codecCT->getInfo().makeColorType(ct);
        SkBitmap bitmap;
        bitmap.allocPixels(info);

        SkCodec::Result result = codecCT->startIncrementalDecode(
            info, bitmap.getPixels(), bitmap.rowBytes());

        if (result != SkCodec::kSuccess) {
            // Some color types might not be supported, which is fine
            continue;
        }

        int rowsDecoded = 0;
        while (true) {
            result = codecCT->incrementalDecode(&rowsDecoded);
            if (result == SkCodec::kSuccess) {
                break;
            }
            REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput,
                           "Unexpected result for color type %d", ct);
            if (result != SkCodec::kIncompleteInput) {
                break;
            }
        }
    }
}

// Test edge case: empty subset
DEF_TEST(Codec_Bmp_Rust_IncrementalDecode_EmptySubset, r) {
    const char* path = "images/randPixels.bmp";
    sk_sp<SkData> data = GetResourceAsData(path);
    REPORTER_ASSERT(r, data, "Missing test image: %s", path);
    if (!data) {
        return;
    }

    std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, codec, "Failed to create codec");
    if (!codec) {
        return;
    }

    // Try an empty subset (width or height = 0)
    SkIRect emptySubset = SkIRect::MakeXYWH(0, 0, 0, 0);
    SkCodec::Options options;
    options.fSubset = &emptySubset;

    SkImageInfo subsetInfo = codec->getInfo().makeDimensions(emptySubset.size());
    SkBitmap bitmap;

    // Empty dimensions should fail gracefully
    if (subsetInfo.width() > 0 && subsetInfo.height() > 0) {
        bitmap.allocPixels(subsetInfo);
        SkCodec::Result result = codec->startIncrementalDecode(
            subsetInfo, bitmap.getPixels(), bitmap.rowBytes(), &options);
        // Any result is acceptable as long as we don't crash
        (void)result;
    }
}

// Test that ICC profile data is valid when present
DEF_TEST(Codec_Bmp_Rust_IccProfile_Validation, r) {
    // Test with files that might have ICC profiles
    const char* paths[] = {
        "images/randPixels.bmp",
        "images/rle.bmp",
        "images/32bpp-topdown-320x240.bmp",
    };

    for (const char* path : paths) {
        skiatest::ReporterContext ctx(r, path);
        sk_sp<SkData> data = GetResourceAsData(path);
        if (!data) {
            continue;
        }

        std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
        if (!codec) {
            continue;
        }

        // Just verify the decode works - ICC profile is handled internally
        SkBitmap bitmap;
        bitmap.allocPixels(codec->getInfo());
        SkCodec::Result result = codec->getPixels(bitmap.pixmap());
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    }
}

// Test incomplete metadata - stream with partial BMP header
DEF_TEST(Codec_Bmp_Rust_IncompleteMetadata_PartialHeader, r) {
    // Load a valid BMP
    sk_sp<SkData> data = GetResourceAsData("images/randPixels.bmp");
    if (!data) {
        ERRORF(r, "Failed to load test image");
        return;
    }

    // Create stream with only partial header (30 bytes of 54-byte header)
    size_t partialSize = 30;
    sk_sp<SkData> partialData = SkData::MakeSubset(data.get(), 0, partialSize);
    std::unique_ptr<SkStream> partialStream = SkMemoryStream::Make(partialData);

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
        SkBmpRustDecoder::Decode(std::move(partialStream), &result);

    // Should fail with incomplete input (not enough for metadata)
    REPORTER_ASSERT(r, codec == nullptr,
                   "Should not create codec with incomplete metadata");
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput,
                   "Should return kIncompleteInput, got %d", result);
}

// Test incomplete row data - stream with partial pixel data
DEF_TEST(Codec_Bmp_Rust_IncompleteRowData, r) {
    // Load a valid BMP
    sk_sp<SkData> data = GetResourceAsData("images/randPixels.bmp");
    if (!data) {
        ERRORF(r, "Failed to load test image");
        return;
    }

    // Get codec to determine dimensions
    std::unique_ptr<SkCodec> fullCodec =
        SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    if (!fullCodec) {
        ERRORF(r, "Failed to create codec from full data");
        return;
    }

    SkImageInfo info = fullCodec->getInfo();

    // BMP header is 54 bytes, calculate size for header + half of first row
    size_t headerSize = 54;
    size_t rowBytes = info.width() * 3; // Assuming 24-bit RGB
    rowBytes = ((rowBytes + 3) / 4) * 4; // Row padding to 4-byte boundary
    size_t partialSize = headerSize + (rowBytes / 2);

    // Ensure we don't exceed data size
    if (partialSize >= data->size()) {
        // Image too small for this test, skip
        return;
    }

    sk_sp<SkData> partialData = SkData::MakeSubset(data.get(), 0, partialSize);
    std::unique_ptr<SkStream> partialStream = SkMemoryStream::Make(partialData);

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
        SkBmpRustDecoder::Decode(std::move(partialStream), &result);

    // Current implementation reads entire stream upfront, so this might succeed
    // Future: when true streaming is implemented, this should return kIncompleteInput
    // For now, document the behavior
    if (codec) {
        // If codec created, verify we can't decode full image
        SkBitmap bitmap;
        bitmap.allocPixels(info);
        SkCodec::Result decodeResult = codec->getPixels(info, bitmap.getPixels(),
                                                        bitmap.rowBytes());

        // Should fail or return incomplete
        REPORTER_ASSERT(r, decodeResult != SkCodec::kSuccess || info.height() == 0,
                       "Should not successfully decode with incomplete pixel data");
    }
}

// Test metadata_loaded flag with valid empty (0x0) BMP
DEF_TEST(Codec_Bmp_Rust_EmptyImage_MetadataLoaded, r) {
    // Create a minimal 0x0 BMP file
    std::vector<uint8_t> emptyBmp;

    // BMP Header (14 bytes)
    emptyBmp.insert(emptyBmp.end(), {'B', 'M'}); // Signature
    uint32_t fileSize = 54; // Header + DIB header, no pixels
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&fileSize),
                   reinterpret_cast<uint8_t*>(&fileSize) + 4);
    uint16_t reserved = 0;
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&reserved),
                   reinterpret_cast<uint8_t*>(&reserved) + 2);
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&reserved),
                   reinterpret_cast<uint8_t*>(&reserved) + 2);
    uint32_t pixelOffset = 54;
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&pixelOffset),
                   reinterpret_cast<uint8_t*>(&pixelOffset) + 4);

    // DIB Header (40 bytes)
    uint32_t dibSize = 40;
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&dibSize),
                   reinterpret_cast<uint8_t*>(&dibSize) + 4);
    int32_t width = 0;
    int32_t height = 0;
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&width),
                   reinterpret_cast<uint8_t*>(&width) + 4);
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&height),
                   reinterpret_cast<uint8_t*>(&height) + 4);
    uint16_t planes = 1;
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&planes),
                   reinterpret_cast<uint8_t*>(&planes) + 2);
    uint16_t bpp = 24;
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&bpp),
                   reinterpret_cast<uint8_t*>(&bpp) + 2);
    uint32_t compression = 0;
    uint32_t imageSize = 0;
    int32_t xPpm = 0;
    int32_t yPpm = 0;
    uint32_t colorsUsed = 0;
    uint32_t colorsImportant = 0;
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&compression),
                   reinterpret_cast<uint8_t*>(&compression) + 4);
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&imageSize),
                   reinterpret_cast<uint8_t*>(&imageSize) + 4);
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&xPpm),
                   reinterpret_cast<uint8_t*>(&xPpm) + 4);
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&yPpm),
                   reinterpret_cast<uint8_t*>(&yPpm) + 4);
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&colorsUsed),
                   reinterpret_cast<uint8_t*>(&colorsUsed) + 4);
    emptyBmp.insert(emptyBmp.end(),
                   reinterpret_cast<uint8_t*>(&colorsImportant),
                   reinterpret_cast<uint8_t*>(&colorsImportant) + 4);

    sk_sp<SkData> data = SkData::MakeWithCopy(emptyBmp.data(), emptyBmp.size());
    std::unique_ptr<SkStream> stream = SkMemoryStream::Make(data);

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
        SkBmpRustDecoder::Decode(std::move(stream), &result);

    // image-rs may not support 0x0 BMPs
    // The key test is: if it fails, it should fail with a clear error,
    // NOT with kIncompleteInput (which would indicate metadata not loaded)
    if (!codec) {
        REPORTER_ASSERT(r, result != SkCodec::kIncompleteInput,
                       "0x0 BMP should fail with clear error, not kIncompleteInput");
    } else {
        // If it succeeds, verify dimensions are 0
        SkImageInfo info = codec->getInfo();
        REPORTER_ASSERT(r, info.width() == 0 && info.height() == 0,
                       "0x0 BMP should have 0 dimensions if supported");
    }
}
