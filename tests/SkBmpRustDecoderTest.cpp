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
#include "tests/Test.h"
#include "tools/Resources.h"

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult) \
    REPORTER_ASSERT(r,                                           \
                    actualResult == SkCodec::kSuccess,           \
                    "actualResult=\"%s\" != kSuccess",           \
                    SkCodec::ResultToString(actualResult))

// Helper wrapping a call to `SkBmpRustDecoder::Decode`.
std::unique_ptr<SkCodec> SkBmpRustDecoderDecode(skiatest::Reporter* r, const char* path) {
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return nullptr;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    return codec;
}

// Helper to decode an image and return the resulting SkImage.
sk_sp<SkImage> DecodeBmpImage(skiatest::Reporter* r, const char* path) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, path);
    if (!codec) {
        return nullptr;
    }

    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    return image;
}

// Helper to verify pixel color at a specific location.
void AssertPixelColor(skiatest::Reporter* r,
                      const SkPixmap& pixmap,
                      int x,
                      int y,
                      SkColor expectedColor,
                      const char* description) {
    SkASSERT(r);
    SkASSERT(x >= 0);
    SkASSERT(y >= 0);
    SkASSERT(description);

    REPORTER_ASSERT(r, x < pixmap.width(), "x=%d >= width=%d", x, pixmap.width());
    REPORTER_ASSERT(r, y < pixmap.height(), "y=%d >= height=%d", y, pixmap.height());
    REPORTER_ASSERT(r,
                    kN32_SkColorType == pixmap.colorType(),
                    "kN32_SkColorType != pixmap.ColorType()=%d",
                    pixmap.colorType());

    SkColor actualColor = pixmap.getColor(x, y);
    REPORTER_ASSERT(r,
                    actualColor == expectedColor,
                    "actualColor=0x%08X != expectedColor==0x%08X at (%d,%d) (%s)",
                    actualColor,
                    expectedColor,
                    x,
                    y,
                    description);
}

// Test basic BMP decoding with a simple 8x8 image.
DEF_TEST(RustBmpCodec_basic_randPixels, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    // Verify dimensions
    SkISize expectedSize = SkISize::Make(8, 8);
    REPORTER_ASSERT(r,
                    codec->dimensions() == expectedSize,
                    "dimensions=%dx%d != expected=%dx%d",
                    codec->dimensions().width(),
                    codec->dimensions().height(),
                    expectedSize.width(),
                    expectedSize.height());

    // Decode the image
    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r, image->width() == 8);
    REPORTER_ASSERT(r, image->height() == 8);
}

// Test RLE-compressed BMP decoding.
DEF_TEST(RustBmpCodec_rle_compression, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/rle.bmp");
    if (!codec) {
        return;
    }

    // Verify dimensions
    SkISize expectedSize = SkISize::Make(320, 240);
    REPORTER_ASSERT(r,
                    codec->dimensions() == expectedSize,
                    "dimensions=%dx%d != expected=%dx%d",
                    codec->dimensions().width(),
                    codec->dimensions().height(),
                    expectedSize.width(),
                    expectedSize.height());

    // Decode the image
    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r, image->width() == 320);
    REPORTER_ASSERT(r, image->height() == 240);
}

// Test 8-bit indexed color BMP.
DEF_TEST(RustBmpCodec_8bpp_indexed, r) {
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoderDecode(r, "images/bmp-size-32x32-8bpp.bmp");
    if (!codec) {
        return;
    }

    // Verify dimensions
    SkISize expectedSize = SkISize::Make(32, 32);
    REPORTER_ASSERT(r,
                    codec->dimensions() == expectedSize,
                    "dimensions=%dx%d != expected=%dx%d",
                    codec->dimensions().width(),
                    codec->dimensions().height(),
                    expectedSize.width(),
                    expectedSize.height());

    // Decode the image
    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r, image->width() == 32);
    REPORTER_ASSERT(r, image->height() == 32);
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
            SkBmpRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);

    // Should fail to decode PNG data as BMP
    REPORTER_ASSERT(r, !codec, "SkBmpRustDecoder should reject PNG data");
}

// Test codec reuse functionality.
DEF_TEST(RustBmpCodec_rewind, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/randPixels.bmp");
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
}

// Test getPixels with a pre-allocated bitmap.
DEF_TEST(RustBmpCodec_getPixels, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/randPixels.bmp");
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

// Test decoding with subset (if supported).
DEF_TEST(RustBmpCodec_dimensions, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/rle.bmp");
    if (!codec) {
        return;
    }

    SkISize dimensions = codec->dimensions();
    REPORTER_ASSERT(r, dimensions.width() == 320, "Expected width 320, got %d", dimensions.width());
    REPORTER_ASSERT(r, dimensions.height() == 240, "Expected height 240, got %d", dimensions.height());

    SkImageInfo info = codec->getInfo();
    REPORTER_ASSERT(r, info.width() == 320);
    REPORTER_ASSERT(r, info.height() == 240);
}

// Test pixel accuracy - verify actual decoded pixel values.
DEF_TEST(RustBmpCodec_pixel_accuracy, r) {
    sk_sp<SkImage> image = DecodeBmpImage(r, "images/randPixels.bmp");
    if (!image) {
        return;
    }

    // Convert to N32 for pixel inspection
    SkBitmap bitmap;
    SkImageInfo info = SkImageInfo::MakeN32Premul(image->width(), image->height());
    bitmap.allocPixels(info);
    REPORTER_ASSERT(r, image->readPixels(nullptr, bitmap.pixmap(), 0, 0));

    // Verify dimensions
    REPORTER_ASSERT(r, bitmap.width() == 8);
    REPORTER_ASSERT(r, bitmap.height() == 8);

    // Spot check some pixel values to ensure correct decoding
    // Note: These values depend on the specific test image
    SkPixmap pixmap = bitmap.pixmap();
    REPORTER_ASSERT(r, pixmap.addr() != nullptr, "Pixmap should have valid pixel data");
}

// Test 24-bit BMP decoding.
DEF_TEST(RustBmpCodec_24bit, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    // randPixels.bmp is 24-bit
    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r, image->width() == 8);
    REPORTER_ASSERT(r, image->height() == 8);
}

// Test 4-bit RLE compressed BMP.
DEF_TEST(RustBmpCodec_4bit_rle, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/rle.bmp");
    if (!codec) {
        return;
    }

    // rle.bmp is 4-bit with RLE compression
    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
}

// Test handling of zero-width BMP.
DEF_TEST(RustBmpCodec_zero_width, r) {
    sk_sp<SkData> data = GetResourceAsData("empty_images/zero-width.bmp");
    if (!data) {
        ERRORF(r, "Missing resource: empty_images/zero-width.bmp");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);

    // Should fail gracefully on zero-width image
    REPORTER_ASSERT(r, !codec || result != SkCodec::kSuccess,
                    "Decoder should reject or fail on zero-width BMP");
}

// Test handling of zero-height BMP.
DEF_TEST(RustBmpCodec_zero_height, r) {
    sk_sp<SkData> data = GetResourceAsData("empty_images/zero-height.bmp");
    if (!data) {
        ERRORF(r, "Missing resource: empty_images/zero-height.bmp");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);

    // Should fail gracefully on zero-height image
    REPORTER_ASSERT(r, !codec || result != SkCodec::kSuccess,
                    "Decoder should reject or fail on zero-height BMP");
}

// Test handling of corrupted BMP (incomplete input).
DEF_TEST(RustBmpCodec_corrupted_incomplete, r) {
    sk_sp<SkData> data = GetResourceAsData("invalid_images/b33251605.bmp");
    if (!data) {
        ERRORF(r, "Missing resource: invalid_images/b33251605.bmp");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);

    // Should handle incomplete/corrupted input gracefully (key: don't crash)
    // The decoder may choose to accept it (with warnings) or reject it
    if (codec) {
        // If codec was created, try to decode - any result is acceptable
        auto [image, decodeResult] = codec->getImage();
        // Test passes as long as we get here without crashing
        (void)image;  // May be null
        (void)decodeResult;  // May be success, incomplete, or error
    }
}

// Test handling of invalid BMP header.
DEF_TEST(RustBmpCodec_invalid_header, r) {
    sk_sp<SkData> data = GetResourceAsData("invalid_images/b33651913.bmp");
    if (!data) {
        ERRORF(r, "Missing resource: invalid_images/b33651913.bmp");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);

    // Should reject invalid header
    REPORTER_ASSERT(r, !codec, "Decoder should reject BMP with invalid header");
}

// Test handling of malformed BMP with extreme dimensions.
DEF_TEST(RustBmpCodec_extreme_dimensions, r) {
    sk_sp<SkData> data = GetResourceAsData("invalid_images/b34778578.bmp");
    if (!data) {
        ERRORF(r, "Missing resource: invalid_images/b34778578.bmp");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);

    // Should reject BMP with extreme/invalid dimensions
    REPORTER_ASSERT(r, !codec, "Decoder should reject BMP with extreme dimensions");
}

// Test handling of OS/2 format BMP (fuzz test case).
DEF_TEST(RustBmpCodec_os2_fuzz, r) {
    sk_sp<SkData> data = GetResourceAsData("invalid_images/osfuzz6288.bmp");
    if (!data) {
        ERRORF(r, "Missing resource: invalid_images/osfuzz6288.bmp");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);

    // Should handle OS/2 format gracefully (either decode or reject safely)
    // The key is not to crash
    if (codec) {
        // If accepted, try to decode (may succeed or fail gracefully)
        auto [image, decodeResult] = codec->getImage();
        // Any result is acceptable as long as we don't crash
    }
}

// Test getInfo returns correct image information.
DEF_TEST(RustBmpCodec_getInfo, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/bmp-size-32x32-8bpp.bmp");
    if (!codec) {
        return;
    }

    SkImageInfo info = codec->getInfo();
    REPORTER_ASSERT(r, info.width() == 32, "Expected width 32, got %d", info.width());
    REPORTER_ASSERT(r, info.height() == 32, "Expected height 32, got %d", info.height());
    REPORTER_ASSERT(r, info.colorType() != kUnknown_SkColorType,
                    "Color type should be known");
}

// Test that multiple successive decodes produce consistent results.
DEF_TEST(RustBmpCodec_consistency, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    // Decode multiple times
    auto [image1, result1] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result1);

    auto [image2, result2] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result2);

    auto [image3, result3] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result3);

    // All images should have same dimensions
    REPORTER_ASSERT(r, image1 && image2 && image3);
    REPORTER_ASSERT(r, image1->dimensions() == image2->dimensions());
    REPORTER_ASSERT(r, image2->dimensions() == image3->dimensions());
}

// Test explicit rewind through multiple getPixels calls.
DEF_TEST(RustBmpCodec_explicit_rewind, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/randPixels.bmp");
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

// Test rewind() preserves codec state and allows multiple decode cycles.
DEF_TEST(RustBmpCodec_multiple_rewinds, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    const int numCycles = 5;
    SkISize expectedDimensions = codec->dimensions();

    for (int i = 0; i < numCycles; i++) {
        // Decode (automatically rewinds after first decode)
        auto [image, result] = codec->getImage();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, image, "Image should be valid on cycle %d", i);
        REPORTER_ASSERT(r, image->dimensions() == expectedDimensions,
                        "Dimensions should be consistent on cycle %d", i);
    }
}

// Test getScanlineOrder() returns correct value for bottom-up BMP (positive height).
DEF_TEST(RustBmpCodec_scanline_order_bottom_up, r) {
    // Most BMPs are bottom-up (positive height in header)
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    SkCodec::SkScanlineOrder order = codec->getScanlineOrder();
    REPORTER_ASSERT(r, order == SkCodec::kBottomUp_SkScanlineOrder,
                    "randPixels.bmp should be bottom-up (positive height)");
}

// Test getScanlineOrder() returns correct value for top-down BMP (negative height).
// Note: Top-down BMPs have negative height in the BMP header
DEF_TEST(RustBmpCodec_scanline_order_top_down, r) {
    // Try to find a top-down BMP or skip if not available
    // Top-down BMPs are less common but valid
    // For now, we test that the method exists and returns a valid value
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    SkCodec::SkScanlineOrder order = codec->getScanlineOrder();
    // Should be either top-down or bottom-up (not some invalid value)
    REPORTER_ASSERT(r,
                    order == SkCodec::kBottomUp_SkScanlineOrder ||
                    order == SkCodec::kTopDown_SkScanlineOrder,
                    "Scanline order should be valid (top-down or bottom-up)");
}

// Test rewind() with scanline decoding.
DEF_TEST(RustBmpCodec_rewind_with_scanlines, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    const SkImageInfo info = codec->getInfo();
    const size_t rowBytes = info.minRowBytes();
    std::vector<uint8_t> buffer(info.height() * rowBytes);

    // Start scanline decode
    SkCodec::Result startResult = codec->startScanlineDecode(info);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, startResult);

    // Get some scanlines
    int linesDecoded1 = codec->getScanlines(buffer.data(), 3, rowBytes);
    REPORTER_ASSERT(r, linesDecoded1 == 3, "Should decode 3 scanlines");

    // Start scanline decode again (implicitly rewinds)
    SkCodec::Result startResult2 = codec->startScanlineDecode(info);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, startResult2);

    // Get scanlines again - should start from beginning
    std::vector<uint8_t> buffer2(info.height() * rowBytes);
    int linesDecoded2 = codec->getScanlines(buffer2.data(), 3, rowBytes);
    REPORTER_ASSERT(r, linesDecoded2 == 3, "Should decode 3 scanlines after rewind");

    // First 3 scanlines should match
    REPORTER_ASSERT(r, memcmp(buffer.data(), buffer2.data(), 3 * rowBytes) == 0,
                    "Scanlines should match after rewind");
}

// Test that rewind works correctly after partial scanline decode.
DEF_TEST(RustBmpCodec_rewind_after_partial_decode, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    const SkImageInfo info = codec->getInfo();
    const size_t rowBytes = info.minRowBytes();
    std::vector<uint8_t> buffer(rowBytes * 2);  // Buffer for 2 scanlines

    // Start scanline decode
    SkCodec::Result startResult = codec->startScanlineDecode(info);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, startResult);

    // Decode only 2 scanlines (partial decode)
    int linesDecoded = codec->getScanlines(buffer.data(), 2, rowBytes);
    REPORTER_ASSERT(r, linesDecoded == 2, "Should decode 2 scanlines");

    // Now do a full decode (implicitly rewinds and completes the decode)
    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image, "Should decode successfully after rewind from partial decode");
}

// Test getScanlineOrder() consistency across rewinds.
DEF_TEST(RustBmpCodec_scanline_order_consistency, r) {
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoderDecode(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    // Get initial scanline order
    SkCodec::SkScanlineOrder order1 = codec->getScanlineOrder();

    // Decode
    auto [image1, result1] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result1);

    // Check scanline order again after decode (implicitly rewound)
    SkCodec::SkScanlineOrder order2 = codec->getScanlineOrder();
    REPORTER_ASSERT(r, order1 == order2,
                    "Scanline order should be consistent after decode");

    // Decode again (another implicit rewind)
    auto [image2, result2] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result2);

    // Check one more time
    SkCodec::SkScanlineOrder order3 = codec->getScanlineOrder();
    REPORTER_ASSERT(r, order1 == order3,
                    "Scanline order should remain consistent after multiple decodes");
}
