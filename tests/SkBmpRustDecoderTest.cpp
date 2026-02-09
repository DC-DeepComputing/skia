/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_bmp/decoder/SkBmpRustDecoder.h"

#include <memory>
#include <utility>

#include "include/codec/SkBmpDecoder.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "modules/skcms/skcms.h"
#include "tests/ComparePixels.h"
#include "tests/FakeStreams.h"
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

    // Decode with standard BMP decoder for reference
    std::unique_ptr<SkCodec> stdCodec = SkBmpDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, stdCodec, "Failed to create standard BMP codec for %s", path);
    if (!stdCodec) {
        return;
    }

    SkBitmap reference;
    reference.allocPixels(stdCodec->getInfo());
    SkCodec::Result result = stdCodec->getPixels(reference.pixmap());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Now decode using Rust BMP decoder with incremental decode
    std::unique_ptr<SkCodec> rustCodec = SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, rustCodec, "Failed to create Rust BMP codec for %s", path);
    if (!rustCodec) {
        return;
    }
    SkBitmap incremental;
    incremental.allocPixels(rustCodec->getInfo());

    result = rustCodec->startIncrementalDecode(incremental.info(),
                                          incremental.getPixels(),
                                          incremental.rowBytes());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Decode all rows incrementally
    // For BMP with all data available, this should complete in one call
    result = rustCodec->incrementalDecode();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Verify that Rust incremental decode produces the same output as standard BMP decoder
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
                    "Rust incremental decode should produce identical output to standard BMP decoder");
}

// Test incremental decode with progressively available data.
// Simulates a streaming scenario where data arrives in chunks (like over a network).
// This tests true resumability: starting with partial data, getting kIncompleteInput,
// adding more data, and resuming until complete.
DEF_TEST(Codec_Bmp_Rust_IncrementalDecode_ProgressiveStream, r) {
    const char* path = "images/32bpp-topdown-320x240.bmp";
    sk_sp<SkData> data = GetResourceAsData(path);
    REPORTER_ASSERT(r, data, "Missing test image: %s", path);
    if (!data) {
        return;
    }

    const size_t fullSize = data->size();

    // First, decode the full image for comparison
    std::unique_ptr<SkCodec> referenceCodec =
        SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, referenceCodec, "Failed to create reference codec");
    if (!referenceCodec) {
        return;
    }
    SkBitmap reference;
    reference.allocPixels(referenceCodec->getInfo());
    SkCodec::Result refResult = referenceCodec->getPixels(reference.pixmap());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, refResult);

    // Create a HaltingStream that starts with enough data for the BMP header.
    // BMP header is typically 54 bytes (14 byte file header + 40 byte DIB header),
    // so 128 bytes should always be sufficient for metadata parsing.
    const size_t initialBytes = std::min(size_t{128}, fullSize);
    const size_t chunkSize = 8 * 1024;  // Add 8KB at a time

    // Create stream and retain raw pointer before moving to codec
    auto haltingStream = std::make_unique<HaltingStream>(data, initialBytes);
    HaltingStream* streamPtr = haltingStream.get();

    // Create codec - 128 bytes should be enough for header parsing
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
        SkBmpRustDecoder::Decode(std::move(haltingStream), &result);

    REPORTER_ASSERT(r, codec, "Failed to create codec with %zu bytes, result: %s",
                    initialBytes, SkCodec::ResultToString(result));
    if (!codec) {
        return;
    }

    SkBitmap bitmap;
    bitmap.allocPixels(codec->getInfo());
    // Fill with non-zero pattern to verify the decoder properly writes all pixels
    // even when the buffer is not zero-initialized (fZeroInitialized = kNo_ZeroInitialized)
    memset(bitmap.getPixels(), 0xAB, bitmap.computeByteSize());

    SkCodec::Options options;
    options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
    result = codec->startIncrementalDecode(
        bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(), &options);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Now do the actual resumable decode loop
    int totalRowsDecoded = 0;
    int iterations = 0;
    const int maxIterations = 1000;  // Safety limit
    bool sawProgressiveRows = false;  // Track if we saw partial row decoding
    bool verifiedDecodedRows = false;  // Track if we verified decoded rows are correct

    while (iterations < maxIterations) {
        iterations++;
        int rowsDecoded = 0;
        result = codec->incrementalDecode(&rowsDecoded);

        if (result == SkCodec::kSuccess) {
            // Decoding complete - rowsDecoded should be total height
            totalRowsDecoded = rowsDecoded;
            break;
        } else if (result == SkCodec::kIncompleteInput) {
            // Track if we got partial rows (progressive rendering)
            if (rowsDecoded > 0 && rowsDecoded < codec->getInfo().height()) {
                sawProgressiveRows = true;

                // Verify that decoded rows match the reference image.
                // This confirms the decoder properly writes pixels even when the
                // destination buffer was initialized with garbage (0xAB).
                // Only check once to avoid slowing down the test.
                if (!verifiedDecodedRows) {
                    verifiedDecodedRows = true;
                    const size_t rowBytes = bitmap.rowBytes();
                    const uint8_t* bitmapPixels = static_cast<const uint8_t*>(bitmap.getPixels());
                    const uint8_t* refPixels = static_cast<const uint8_t*>(reference.getPixels());

                    // Check that decoded rows match the reference (not garbage 0xAB)
                    // For this top-down image, decoded rows are at the top
                    bool decodedRowsMatch = true;
                    for (int y = 0; y < rowsDecoded && decodedRowsMatch; ++y) {
                        if (memcmp(bitmapPixels + y * rowBytes,
                                   refPixels + y * rowBytes, rowBytes) != 0) {
                            decodedRowsMatch = false;
                            ERRORF(r, "Decoded row %d doesn't match reference", y);
                        }
                    }
                    REPORTER_ASSERT(r, decodedRowsMatch,
                        "Decoded rows should match reference, not contain garbage");
                }
            }
            totalRowsDecoded = rowsDecoded;

            // Need more data - simulate receiving more from network
            if (streamPtr->isAllDataReceived()) {
                // All data available but still incomplete - unexpected
                ERRORF(r, "Got kIncompleteInput with all data available after %d rows",
                       totalRowsDecoded);
                break;
            }
            streamPtr->addNewData(chunkSize);
        } else {
            ERRORF(r, "Unexpected result during incremental decode: %s",
                   SkCodec::ResultToString(result));
            break;
        }
    }

    REPORTER_ASSERT(r, iterations < maxIterations, "Decode loop exceeded max iterations");
    REPORTER_ASSERT(r, result == SkCodec::kSuccess,
                    "Should complete successfully, got %s after %d iterations",
                    SkCodec::ResultToString(result), iterations);
    REPORTER_ASSERT(r, totalRowsDecoded == codec->getInfo().height(),
                    "Should decode all %d rows, decoded %d",
                    codec->getInfo().height(), totalRowsDecoded);

    // Verify progressive rendering occurred - we should have seen partial rows
    // during the decode process since we're using a small chunk size
    REPORTER_ASSERT(r, sawProgressiveRows,
                    "Should see progressive row decoding with small chunk sizes");

    // Verify we checked decoded rows during partial decode
    REPORTER_ASSERT(r, verifiedDecodedRows,
                    "Should have verified decoded rows during partial decode");

    // Verify the progressively decoded image matches the reference
    REPORTER_ASSERT(r, bitmap.dimensions() == reference.dimensions(),
                    "Dimensions should match reference");

    bool pixelsMatch = memcmp(bitmap.getPixels(), reference.getPixels(),
                              bitmap.computeByteSize()) == 0;
    REPORTER_ASSERT(r, pixelsMatch,
                    "Progressive decode should produce identical output to full decode");
}

// Test that BMP decoding produces identical output between
// SkBmpRustCodec (using incremental API) and SkBmpCodec (standard decoder).
// This validates row-by-row pixel correctness for both bottom-up and top-down images.
DEF_TEST(Codec_Bmp_Rust_IncrementalDecode_Orientation, r) {
    struct TestCase {
        const char* path;
        const char* description;
    };

    TestCase testCases[] = {
        {"images/randPixels.bmp", "bottom-up 8x8 24-bit"},
        {"images/32bpp-topdown-320x240.bmp", "top-down 320x240 32-bit"},
    };

    for (const auto& testCase : testCases) {
        skiatest::ReporterContext ctx(r, testCase.description);

        sk_sp<SkData> data = GetResourceAsData(testCase.path);
        REPORTER_ASSERT(r, data, "Missing test image: %s", testCase.path);
        if (!data) {
            continue;
        }

        // Decode with STANDARD BMP decoder
        std::unique_ptr<SkCodec> stdCodec =
            SkBmpDecoder::Decode(SkMemoryStream::Make(data), nullptr);
        REPORTER_ASSERT(r, stdCodec, "Failed to create standard BMP codec");
        if (!stdCodec) {
            continue;
        }

        const SkImageInfo info = stdCodec->getInfo();
        const int height = info.height();

        SkBitmap stdBitmap;
        stdBitmap.allocPixels(info);
        SkCodec::Result stdResult = stdCodec->getPixels(stdBitmap.pixmap());
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, stdResult);

        // Decode with RUST BMP decoder using incremental API
        std::unique_ptr<SkCodec> rustCodec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
        REPORTER_ASSERT(r, rustCodec, "Failed to create Rust BMP codec");
        if (!rustCodec) {
            continue;
        }

        SkBitmap rustBitmap;
        rustBitmap.allocPixels(info);

        SkCodec::Result result = rustCodec->startIncrementalDecode(
            info, rustBitmap.getPixels(), rustBitmap.rowBytes());
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

        result = rustCodec->incrementalDecode();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

        // Compare row by row
        const size_t rowBytes = stdBitmap.rowBytes();
        const uint8_t* stdPixels = static_cast<const uint8_t*>(stdBitmap.getPixels());
        const uint8_t* rustPixels = static_cast<const uint8_t*>(rustBitmap.getPixels());

        bool allRowsMatch = true;
        for (int y = 0; y < height; ++y) {
            if (memcmp(stdPixels + y * rowBytes, rustPixels + y * rowBytes, rowBytes) != 0) {
                allRowsMatch = false;
                ERRORF(r, "Row %d doesn't match between standard and Rust decoder", y);
            }
        }
        REPORTER_ASSERT(r, allRowsMatch,
                        "All rows should match between SkBmpCodec and SkBmpRustCodec");
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
        ERRORF(r, "Image too small for this test");
        return;
    }

    sk_sp<SkData> partialData = SkData::MakeSubset(data.get(), 0, partialSize);
    std::unique_ptr<SkStream> partialStream = SkMemoryStream::Make(partialData);

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
        SkBmpRustDecoder::Decode(std::move(partialStream), &result);

    // With streaming support, incomplete pixel data should either:
    // 1. Fail to create codec with kIncompleteInput, or
    // 2. Create codec but fail/return incomplete during pixel decode
    if (codec) {
        // Codec created with partial data - try to decode
        SkBitmap bitmap;
        bitmap.allocPixels(info);
        SkCodec::Result decodeResult = codec->getPixels(info, bitmap.getPixels(),
                                                        bitmap.rowBytes());

        // Should fail or return incomplete since we don't have all pixel data
        REPORTER_ASSERT(r, decodeResult != SkCodec::kSuccess || info.height() == 0,
                       "Should not successfully decode with incomplete pixel data, got %s",
                       SkCodec::ResultToString(decodeResult));
    }
}

// Test that embedded ICC profiles in BMP V5 headers are correctly extracted and applied.
// This uses rgb24prof.bmp from the image-rs test suite, which contains an embedded ICC profile.
DEF_TEST(Codec_Bmp_Rust_ICCProfile, r) {
    const char* path = "images/rgb24prof.bmp";
    sk_sp<SkData> data = GetResourceAsData(path);
    REPORTER_ASSERT(r, data, "Missing test image: %s", path);
    if (!data) {
        return;
    }

    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, codec, "Failed to create Rust BMP codec");
    if (!codec) {
        return;
    }

    // Verify that the codec extracted an ICC profile
    const skcms_ICCProfile* profile = codec->getICCProfile();
    REPORTER_ASSERT(r, profile, "BMP with embedded ICC profile should have getICCProfile() != null");
    if (!profile) {
        return;
    }

    // Verify the profile can be used to create a color space
    sk_sp<SkColorSpace> colorSpace = SkColorSpace::Make(*profile);
    REPORTER_ASSERT(r, colorSpace, "ICC profile should be valid and create a color space");
    if (!colorSpace) {
        return;
    }

    // Verify the color space from getInfo() matches
    SkImageInfo info = codec->getInfo();
    REPORTER_ASSERT(r, info.colorSpace(), "getInfo() should have a color space");
    if (info.colorSpace()) {
        REPORTER_ASSERT(r, SkColorSpace::Equals(info.colorSpace(), colorSpace.get()),
                        "Color space from getInfo() should match ICC profile");
    }

    // Verify decoding works with the ICC profile
    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image, "Should be able to decode BMP with ICC profile");
}
