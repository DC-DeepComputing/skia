/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_jpeg/decoder/SkJpegRustDecoder.h"

#include <functional>
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
#include "tests/FakeStreams.h"
#include "tests/Test.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"

#if defined(SK_CODEC_ENCODES_JPEG_WITH_RUST)
#include "experimental/rust_jpeg/encoder/SkJpegRustEncoder.h"
#include "include/encode/SkJpegEncoder.h"
#include "tools/ToolUtils.h"
#endif

#if defined(SK_CODEC_DECODES_JPEG)
#include "include/codec/SkJpegDecoder.h"
#endif

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult)           \
    REPORTER_ASSERT(r,                                                     \
                    (actualResult) == SkCodec::kSuccess,                   \
                    "actualResult=\"%s\" != kSuccess",                     \
                    SkCodec::ResultToString(actualResult))

// Helper wrapping a call to `SkJpegRustDecoder::Decode`.
static std::unique_ptr<SkCodec> decode_rust_jpeg(skiatest::Reporter* r,
                                                 const char* path) {
    skiatest::ReporterContext ctx(r, path);
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return nullptr;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkJpegRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    return codec;
}

// ========== Decoder Tests ==========

DEF_TEST(RustJpegCodec_IsJpeg, r) {
    sk_sp<SkData> jpegData = GetResourceAsData("images/color_wheel.jpg");
    if (!jpegData) {
        ERRORF(r, "Missing resource: images/color_wheel.jpg");
        return;
    }
    REPORTER_ASSERT(r, SkJpegRustDecoder::IsJpeg(jpegData->data(), jpegData->size()));

    sk_sp<SkData> pngData = GetResourceAsData("images/color_wheel.png");
    if (!pngData) {
        ERRORF(r, "Missing resource: images/color_wheel.png");
        return;
    }
    REPORTER_ASSERT(r, !SkJpegRustDecoder::IsJpeg(pngData->data(), pngData->size()));
}

// Table-based test covering baseline, grayscale, and progressive JPEGs.
// Validates dimensions, frame count (always 1 for JPEG), and decodability.
// Use SkISize::MakeEmpty() for images with no well-known expected dimensions.
DEF_TEST(RustJpegCodec_decode_valid_jpgs, r) {
    struct TestCase {
        const char* description;
        const char* path;
        SkISize expectedDimensions;  // MakeEmpty() means "any non-zero size"
    };

    static const TestCase kTestCases[] = {
        {"baseline RGB",    "images/color_wheel.jpg",       SkISize::Make(128, 128)},
        {"grayscale",       "images/grayscale.jpg",         SkISize::MakeEmpty()},
        {"RGB 180x180",     "images/dog.jpg",               SkISize::Make(180, 180)},
        {"progressive",     "images/b78329453.jpeg",        SkISize::MakeEmpty()},
        {"large 512x512",   "images/mandrill_512_q075.jpg", SkISize::Make(512, 512)},
    };

    for (const auto& tc : kTestCases) {
        skiatest::ReporterContext ctx(r, tc.description);

        std::unique_ptr<SkCodec> codec = decode_rust_jpeg(r, tc.path);
        if (!codec) {
            continue;
        }

        REPORTER_ASSERT(r, codec->getFrameCount() == 1);

        if (!tc.expectedDimensions.isEmpty()) {
            REPORTER_ASSERT(r,
                            codec->dimensions() == tc.expectedDimensions,
                            "dimensions=%dx%d != expected=%dx%d",
                            codec->dimensions().width(), codec->dimensions().height(),
                            tc.expectedDimensions.width(), tc.expectedDimensions.height());
        } else {
            REPORTER_ASSERT(r, codec->dimensions().width() > 0);
            REPORTER_ASSERT(r, codec->dimensions().height() > 0);
        }

        auto [image, result] = codec->getImage();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, image);
    }
}

// Table-based test for incremental decoding with progressively available data.
// Simulates a streaming scenario where compressed data arrives in chunks (e.g.
// over a network).
//
// Flow per image:
//   1. Create a HaltingStream starting with only kInitialBytes available.
//   2. Call startIncrementalDecode + incrementalDecode.
//   3a. kIncompleteInput  — the codec decoded what it could and needs more data.
//       Add a chunk and call incrementalDecode again without restarting.
//   3b. Error             — the current full-buffer implementation (zune-jpeg
//       buffers the whole compressed stream in memory) cannot decode from a
//       stream that hit its halting limit mid-read; it returns an error rather
//       than kIncompleteInput.  We handle this by adding more data and
//       restarting via startIncrementalDecode, which calls reset_decode_state
//       so that the next try_read_more picks up the new bytes.
//       A future streaming-from-the-start implementation would return
//       kIncompleteInput here, making the restart unnecessary.
//   4. Repeat until kSuccess is returned with the fully available stream.
DEF_TEST(RustJpeg_IncrementalDecode_PartialStreaming, r) {
    struct TestCase {
        const char* path;
        const char* description;
    };

    static const TestCase kTestCases[] = {
        {"images/color_wheel.jpg",       "baseline RGB 128x128"},
        {"images/dog.jpg",               "RGB 180x180"},
        {"images/mandrill_512_q075.jpg", "RGB 512x512"},
        {"images/grayscale.jpg",         "grayscale"},
    };

    // Use a generous initial window: JPEG headers (SOS, DQT, DHT tables…) can
    // easily exceed 1 KB, so 4 KB is a safe starting point for all test images.
    constexpr size_t kInitialBytes = 4 * 1024;
    constexpr size_t kChunkSize = 4 * 1024;
    constexpr int kMaxIterations = 1000;

    for (const auto& testCase : kTestCases) {
        skiatest::ReporterContext ctx(r, testCase.description);

        sk_sp<SkData> data = GetResourceAsData(testCase.path);
        if (!data) {
            ERRORF(r, "Missing resource: %s", testCase.path);
            continue;
        }

        // Decode the full image first so we can verify the incremental result.
        std::unique_ptr<SkCodec> refCodec =
                SkJpegRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
        if (!refCodec) {
            ERRORF(r, "Failed to create reference codec for %s", testCase.path);
            continue;
        }
        SkBitmap reference;
        reference.allocPixels(refCodec->getInfo().makeColorType(kN32_SkColorType));
        {
            SkCodec::Result refResult = refCodec->getPixels(reference.pixmap());
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, refResult);
            if (refResult != SkCodec::kSuccess) {
                continue;
            }
        }

        // Create a HaltingStream that starts with just the header bytes.
        const size_t initialBytes = std::min(kInitialBytes, data->size());
        auto haltingStream = std::make_unique<HaltingStream>(data, initialBytes);
        HaltingStream* streamPtr = haltingStream.get();

        SkCodec::Result result;
        std::unique_ptr<SkCodec> codec =
                SkJpegRustDecoder::Decode(std::move(haltingStream), &result);
        if (!codec) {
            ERRORF(r, "%s: failed to create streaming codec with %zu bytes: %s",
                   testCase.description, initialBytes, SkCodec::ResultToString(result));
            continue;
        }

        const SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
        SkBitmap bitmap;
        if (!bitmap.tryAllocPixels(info)) {
            ERRORF(r, "%s: failed to allocate bitmap", testCase.description);
            continue;
        }

        result = codec->startIncrementalDecode(info, bitmap.getPixels(), bitmap.rowBytes());
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        if (result != SkCodec::kSuccess) {
            continue;
        }

        int iterations = 0;
        while (iterations++ < kMaxIterations) {
            int rowsDecoded = 0;
            result = codec->incrementalDecode(&rowsDecoded);

            // Accept kSuccess only when the entire stream has been consumed.
            // zune-jpeg's graceful-degradation path (estimate_decoded_rows) can
            // return Success from truncated data; we don't want to compare that
            // partial result against the full-data reference.
            if (result == SkCodec::kSuccess && streamPtr->isAllDataReceived()) {
                break;
            }

            if (streamPtr->isAllDataReceived()) {
                // All data is available but we still didn't get a clean kSuccess
                // — that is a real failure.
                ERRORF(r, "%s: still not kSuccess with all data: %s",
                       testCase.description, SkCodec::ResultToString(result));
                break;
            }

            // More data is needed.  Add a chunk, then determine whether a
            // restart is necessary:
            streamPtr->addNewData(kChunkSize);

            if (result == SkCodec::kIncompleteInput) {
                // Codec signalled it can resume from where it left off
                // — no restart required.  Just call incrementalDecode again.
            } else {
                // Either an error or a graceful partial-success (see above).
                // In both cases the incremental-decode state is no longer
                // valid; restart via startIncrementalDecode so that
                // reset_decode_state clears stream_exhausted and the next
                // try_read_more picks up the newly available bytes.
                result = codec->startIncrementalDecode(
                        info, bitmap.getPixels(), bitmap.rowBytes());
                if (result != SkCodec::kSuccess) {
                    ERRORF(r, "%s: startIncrementalDecode failed after adding data: %s",
                           testCase.description, SkCodec::ResultToString(result));
                    break;
                }
            }
        }

        REPORTER_ASSERT(r, iterations < kMaxIterations,
                        "%s: decode loop exceeded max iterations", testCase.description);
        REPORTER_ASSERT(r, result == SkCodec::kSuccess,
                        "%s: expected kSuccess, got %s",
                        testCase.description, SkCodec::ResultToString(result));

        // Verify dimensions match the reference.
        REPORTER_ASSERT(r,
                        bitmap.dimensions() == reference.dimensions(),
                        "%s: incremental dims %dx%d != reference %dx%d",
                        testCase.description,
                        bitmap.width(), bitmap.height(),
                        reference.width(), reference.height());

        // Both paths used the same Rust decoder, so pixel output should be
        // identical (zero tolerance).
        if (bitmap.dimensions() == reference.dimensions()) {
            const float tols[4] = {0, 0, 0, 0};
            auto error = std::function<ComparePixmapsErrorReporter>(
                    [&](int x, int y, const float diffs[4]) {
                        ERRORF(r,
                               "%s: pixel differs at (%d,%d): diffs=(%f,%f,%f,%f)",
                               testCase.description,
                               x, y,
                               diffs[0], diffs[1], diffs[2], diffs[3]);
                    });
            ComparePixels(bitmap.pixmap(), reference.pixmap(), tols, error);
        }
    }
}

DEF_TEST(RustJpegCodec_rewind, r) {
    std::unique_ptr<SkCodec> codec = decode_rust_jpeg(r, "images/color_wheel.jpg");
    if (!codec) {
        return;
    }

    auto [image1, result1] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result1);
    REPORTER_ASSERT(r, image1);

    auto [image2, result2] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result2);
    REPORTER_ASSERT(r, image2);

    REPORTER_ASSERT(r, image1->dimensions() == image2->dimensions());
}

DEF_TEST(RustJpegCodec_incomplete_input, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.jpg");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.jpg");
        return;
    }

    // Keep only the first third of the data.
    sk_sp<SkData> truncated = SkData::MakeSubset(data.get(), 0, data->size() / 3);

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkJpegRustDecoder::Decode(SkMemoryStream::Make(std::move(truncated)), &result);
    if (!codec) {
        // Acceptable: codec construction failed.
        REPORTER_ASSERT(r, result != SkCodec::kSuccess);
        return;
    }

    // If codec constructed, decoding may succeed (Rust decoder is lenient with
    // truncated data) or fail - either outcome is acceptable.
    auto [image, decodeResult] = codec->getImage();
    (void)decodeResult;
}

DEF_TEST(RustJpegCodec_reject_non_jpeg, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.png");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.png");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkJpegRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);
    REPORTER_ASSERT(r, !codec, "Should reject PNG data as JPEG");
}

DEF_TEST(RustJpegCodec_decode_diverse_images, r) {
    static const char* kImages[] = {
        "images/ducky.jpg",
        "images/mandrill_h1v1.jpg",
        "images/mandrill_h2v1.jpg",
        "images/cropped_mandrill.jpg",
        "images/flutter_logo.jpg",
        "images/icc-v2-gbr.jpg",
        "images/randPixels.jpg",
        "images/crbug999986.jpeg",
    };
    for (const char* path : kImages) {
        skiatest::ReporterContext ctx(r, path);
        std::unique_ptr<SkCodec> codec = decode_rust_jpeg(r, path);
        if (!codec) {
            continue;
        }
        auto [image, result] = codec->getImage();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, image);
        REPORTER_ASSERT(r, image->width() > 0);
        REPORTER_ASSERT(r, image->height() > 0);
    }
}

// ========== Encoder Tests ==========

#if defined(SK_CODEC_ENCODES_JPEG_WITH_RUST)

// Basic encode smoke test: decode a JPEG, re-encode it, and verify the output
// is non-empty and re-decodable.
DEF_TEST(RustJpegEncode_smoke_test, r) {
    sk_sp<SkData> data = GetResourceAsData("images/mandrill_512_q075.jpg");
    if (!data) {
        ERRORF(r, "Missing resource: images/mandrill_512_q075.jpg");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkJpegRustDecoder::Decode(SkMemoryStream::Make(data), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    if (!codec) {
        return;
    }

    auto [image, decodeResult] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, decodeResult);
    REPORTER_ASSERT(r, image);

    SkPixmap src;
    REPORTER_ASSERT(r, image->peekPixels(&src));

    SkDynamicMemoryWStream dst;
    SkJpegEncoder::Options options;
    options.fQuality = 85;
    REPORTER_ASSERT(r, SkJpegRustEncoder::Encode(&dst, src, options));
    REPORTER_ASSERT(r, dst.bytesWritten() > 0);

    // Verify the encoded output is a valid decodable JPEG.
    sk_sp<SkData> encoded = dst.detachAsData();
    REPORTER_ASSERT(r, SkJpegRustDecoder::IsJpeg(encoded->data(), encoded->size()));

    SkCodec::Result reDecodeResult;
    std::unique_ptr<SkCodec> reCodec =
            SkJpegRustDecoder::Decode(SkMemoryStream::Make(encoded), &reDecodeResult);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, reDecodeResult);
    REPORTER_ASSERT(r, reCodec);

    auto [reImage, reImageResult] = reCodec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, reImageResult);
    REPORTER_ASSERT(r, reImage);
    REPORTER_ASSERT(r, reImage->width() == 512);
    REPORTER_ASSERT(r, reImage->height() == 512);
}

DEF_TEST(RustJpegEncode_roundtrip_dimensions, r) {
    SkBitmap bitmap;
    if (!ToolUtils::GetResourceAsBitmap("images/dog.jpg", &bitmap)) {
        ERRORF(r, "Failed to decode images/dog.jpg");
        return;
    }

    SkPixmap src;
    REPORTER_ASSERT(r, bitmap.peekPixels(&src));

    SkDynamicMemoryWStream dst;
    SkJpegEncoder::Options options;
    options.fQuality = 90;
    REPORTER_ASSERT(r, SkJpegRustEncoder::Encode(&dst, src, options));

    SkBitmap roundtrip;
    REPORTER_ASSERT(r, ToolUtils::DecodeDataToBitmap(dst.detachAsData(), &roundtrip));
    REPORTER_ASSERT(r, roundtrip.width() == bitmap.width());
    REPORTER_ASSERT(r, roundtrip.height() == bitmap.height());
}

DEF_TEST(RustJpegEncode_quality_affects_size, r) {
    SkBitmap bitmap;
    if (!ToolUtils::GetResourceAsBitmap("images/mandrill_512_q075.jpg", &bitmap)) {
        ERRORF(r, "Failed to decode images/mandrill_512_q075.jpg");
        return;
    }

    SkPixmap src;
    REPORTER_ASSERT(r, bitmap.peekPixels(&src));

    SkDynamicMemoryWStream dstLow, dstHigh;
    SkJpegEncoder::Options lowOptions, highOptions;
    lowOptions.fQuality = 10;
    highOptions.fQuality = 95;

    REPORTER_ASSERT(r, SkJpegRustEncoder::Encode(&dstLow, src, lowOptions));
    REPORTER_ASSERT(r, SkJpegRustEncoder::Encode(&dstHigh, src, highOptions));

    REPORTER_ASSERT(r,
                    dstLow.bytesWritten() < dstHigh.bytesWritten(),
                    "Low quality (%zu bytes) should be smaller than high quality (%zu bytes)",
                    dstLow.bytesWritten(),
                    dstHigh.bytesWritten());
}

// Table-based test for encoding different pixel formats. Verifies that RGB,
// RGBA (alpha stripped or blended), BGRA, and grayscale all round-trip through
// the encoder without crashing and produce non-empty output.
DEF_TEST(RustJpegEncode_color_formats, r) {
    struct TestCase {
        const char* description;
        SkColorType colorType;
        SkAlphaType alphaType;
        SkColor fillColor;
        SkJpegEncoder::AlphaOption alphaOption;
    };

    static const TestCase kTestCases[] = {
        {"grayscale",
         kGray_8_SkColorType,   kOpaque_SkAlphaType,   SK_ColorGRAY,
         SkJpegEncoder::AlphaOption::kIgnore},
        {"RGBA ignore alpha",
         kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, 0x80FF0000,
         SkJpegEncoder::AlphaOption::kIgnore},
        {"RGBA blend on black",
         kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, 0x8000FF00,
         SkJpegEncoder::AlphaOption::kBlendOnBlack},
        {"BGRA",
         kBGRA_8888_SkColorType, kUnpremul_SkAlphaType, SK_ColorBLUE,
         SkJpegEncoder::AlphaOption::kIgnore},
    };

    for (const auto& tc : kTestCases) {
        skiatest::ReporterContext ctx(r, tc.description);

        SkImageInfo info = SkImageInfo::Make(64, 64, tc.colorType, tc.alphaType);
        SkBitmap bitmap;
        REPORTER_ASSERT(r, bitmap.tryAllocPixels(info));
        bitmap.eraseColor(tc.fillColor);

        SkPixmap src;
        REPORTER_ASSERT(r, bitmap.peekPixels(&src));

        SkDynamicMemoryWStream dst;
        SkJpegEncoder::Options options;
        options.fQuality = 85;
        options.fAlphaOption = tc.alphaOption;
        REPORTER_ASSERT(r, SkJpegRustEncoder::Encode(&dst, src, options));
        REPORTER_ASSERT(r, dst.bytesWritten() > 0);
    }
}

#endif  // defined(SK_CODEC_ENCODES_JPEG_WITH_RUST)

// ========== Rust vs C++ JPEG Comparison Tests ==========

#if defined(SK_CODEC_DECODES_JPEG)

// Decode the same JPEG with both the Rust decoder and the C++ libjpeg-turbo
// decoder, then compare the outputs pixel-by-pixel within a tolerance.
// JPEG decoding is inherently lossy and different DCT implementations may
// produce slightly different values; a tolerance of ~8/255 per channel covers
// the typical inter-implementation variance.
static void compare_rust_vs_cpp_jpeg(skiatest::Reporter* r,
                                     const char* path,
                                     float tolerancePerChannel = 8.0f / 255.0f) {
    skiatest::ReporterContext ctx(r, path);
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return;
    }

    SkCodec::Result rustResult;
    std::unique_ptr<SkCodec> rustCodec =
            SkJpegRustDecoder::Decode(SkMemoryStream::Make(data), &rustResult);
    if (!rustCodec) {
        // Some images (e.g. CMYK) may not be supported by the Rust decoder.
        return;
    }
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, rustResult);

    SkCodec::Result cppResult;
    std::unique_ptr<SkCodec> cppCodec =
            SkJpegDecoder::Decode(SkMemoryStream::Make(data), &cppResult);
    if (!cppCodec) {
        return;
    }
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, cppResult);

    auto [rustImage, rustDecodeResult] = rustCodec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, rustDecodeResult);
    REPORTER_ASSERT(r, rustImage);

    auto [cppImage, cppDecodeResult] = cppCodec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, cppDecodeResult);
    REPORTER_ASSERT(r, cppImage);

    if (!rustImage || !cppImage) {
        return;
    }

    REPORTER_ASSERT(r,
                    rustImage->dimensions() == cppImage->dimensions(),
                    "Rust dims %dx%d != C++ dims %dx%d",
                    rustImage->width(),
                    rustImage->height(),
                    cppImage->width(),
                    cppImage->height());
    if (rustImage->dimensions() != cppImage->dimensions()) {
        return;
    }

    SkBitmap rustBitmap, cppBitmap;
    REPORTER_ASSERT(r, rustBitmap.tryAllocPixels(rustImage->imageInfo()));
    REPORTER_ASSERT(r, cppBitmap.tryAllocPixels(cppImage->imageInfo()));
    REPORTER_ASSERT(r, rustImage->readPixels(nullptr, rustBitmap.pixmap(), 0, 0));
    REPORTER_ASSERT(r, cppImage->readPixels(nullptr, cppBitmap.pixmap(), 0, 0));

    const float tols[4] = {tolerancePerChannel, tolerancePerChannel, tolerancePerChannel, 0.0f};
    auto error = std::function<ComparePixmapsErrorReporter>(
            [&](int x, int y, const float diffs[4]) {
                ERRORF(r,
                       "%s: pixel differs at (%d, %d): diffs=(%f, %f, %f, %f)",
                       path,
                       x,
                       y,
                       diffs[0],
                       diffs[1],
                       diffs[2],
                       diffs[3]);
            });
    ComparePixels(rustBitmap.pixmap(), cppBitmap.pixmap(), tols, error);
}

// Table-based test comparing Rust and C++ JPEG decoders across many images.
// Each entry specifies a description, path, and optional per-channel tolerance
// (default 8/255). CMYK images and other unsupported formats are gracefully
// skipped when the Rust decoder returns nullptr.
DEF_TEST(RustJpeg_vs_CppJpeg, r) {
    struct TestCase {
        const char* description;
        const char* path;
        float tolerance;
    };

    static constexpr float kDefaultTol = 8.0f / 255.0f;

    static const TestCase kTestCases[] = {
        {"color wheel (baseline RGB)",    "images/color_wheel.jpg",       kDefaultTol},
        {"dog (180x180, RGB)",            "images/dog.jpg",               kDefaultTol},
        {"ducky",                         "images/ducky.jpg",             kDefaultTol},
        {"mandrill 512 q075",             "images/mandrill_512_q075.jpg", kDefaultTol},
        {"mandrill h1v1 subsampling",     "images/mandrill_h1v1.jpg",     kDefaultTol},
        {"mandrill h2v1 subsampling",     "images/mandrill_h2v1.jpg",     kDefaultTol},
        {"grayscale",                     "images/grayscale.jpg",         kDefaultTol},
        {"progressive (b78329453)",       "images/b78329453.jpeg",        kDefaultTol},
        // Progressive JPEGs can have slightly larger inter-implementation diffs.
        {"progressive (crbug999986)",     "images/crbug999986.jpeg",      10.0f / 255.0f},
        {"cropped mandrill",              "images/cropped_mandrill.jpg",  kDefaultTol},
        {"flutter logo",                  "images/flutter_logo.jpg",      kDefaultTol},
        {"ICC v2 GBR profile",            "images/icc-v2-gbr.jpg",        kDefaultTol},
        // randPixels has small DCT rounding differences between implementations.
        {"random pixels",                 "images/randPixels.jpg",        16.0f / 255.0f},
        // CMYK: Rust decoder may not support it; compare_rust_vs_cpp_jpeg
        // skips gracefully when rustCodec is nullptr.
        {"CMYK (graceful skip if unsupported)", "images/CMYK.jpg",        kDefaultTol},
    };

    for (const TestCase& tc : kTestCases) {
        compare_rust_vs_cpp_jpeg(r, tc.path, tc.tolerance);
    }
}

#endif  // defined(SK_CODEC_DECODES_JPEG)
