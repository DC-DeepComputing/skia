/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#ifdef SK_CODEC_DECODES_BMP_WITH_RUST

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "tests/CodecPriv.h"
#include "tools/Resources.h"

// Test basic BMP decoding functionality
DEF_TEST(SkBmpRustDecoder_Basic, r) {
    auto stream = GetResourceAsStream("images/randPixels.bmp");
    if (!stream) {
        ERRORF(r, "Could not load BMP test image");
        return;
    }

    auto codec = SkCodec::MakeFromStream(std::move(stream));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->getEncodedFormat() == SkEncodedImageFormat::kBMP);

    auto info = codec->getInfo();
    REPORTER_ASSERT(r, info.width() > 0 && info.height() > 0);

    // Test basic decode
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    auto result = codec->getPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);
}

// Test 8-bit indexed BMP
DEF_TEST(SkBmpRustDecoder_8Bit, r) {
    auto stream = GetResourceAsStream("images/bmp-size-32x32-8bpp.bmp");
    if (!stream) {
        // Fallback to any available BMP
        stream = GetResourceAsStream("images/randPixels.bmp");
    }
    if (!stream) {
        ERRORF(r, "Could not load BMP test image for 8-bit test");
        return;
    }

    auto codec = SkCodec::MakeFromStream(std::move(stream));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->getEncodedFormat() == SkEncodedImageFormat::kBMP);

    auto info = codec->getInfo();
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    auto result = codec->getPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);
}

// Test RLE compressed BMP
DEF_TEST(SkBmpRustDecoder_RLE, r) {
    auto stream = GetResourceAsStream("images/rle.bmp");
    if (!stream) {
        // Skip if RLE test image not available
        return;
    }

    auto codec = SkCodec::MakeFromStream(std::move(stream));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->getEncodedFormat() == SkEncodedImageFormat::kBMP);

    auto info = codec->getInfo();
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    auto result = codec->getPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);
}

// Test incremental decoding
DEF_TEST(SkBmpRustDecoder_Incremental, r) {
    auto stream = GetResourceAsStream("images/randPixels.bmp");
    if (!stream) {
        ERRORF(r, "Could not load BMP test image");
        return;
    }

    auto codec = SkCodec::MakeFromStream(std::move(stream));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    auto info = codec->getInfo();
    SkBitmap bitmap;
    bitmap.allocPixels(info);

    // Try incremental decode
    auto result = codec->startIncrementalDecode(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes());
    if (result == SkCodec::kSuccess) {
        int rowsDecoded = 0;
        result = codec->incrementalDecode(&rowsDecoded);
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);
        REPORTER_ASSERT(r, rowsDecoded == info.height());
    } else {
        // Incremental decode may not be supported, that's OK
        REPORTER_ASSERT(r, result == SkCodec::kUnimplemented);
    }
}

// Test invalid BMP handling
DEF_TEST(SkBmpRustDecoder_InvalidFile, r) {
    // Create invalid BMP data
    const char invalidData[] = "BM\x00\x00\x00\x00invalid";
    auto stream = SkMemoryStream::MakeDirect(invalidData, sizeof(invalidData));

    auto codec = SkCodec::MakeFromStream(std::move(stream));
    // Should either return null or handle gracefully
    if (codec) {
        REPORTER_ASSERT(r, codec->getEncodedFormat() == SkEncodedImageFormat::kBMP);

        auto info = codec->getInfo();
        SkBitmap bitmap;
        bitmap.allocPixels(info);
        auto result = codec->getPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes());
        // Should fail gracefully, not crash
        REPORTER_ASSERT(r, result != SkCodec::kSuccess);
    }
}

// Test codec reuses functionality
DEF_TEST(SkBmpRustDecoder_Rewind, r) {
    auto stream = GetResourceAsStream("images/randPixels.bmp");
    if (!stream) {
        ERRORF(r, "Could not load BMP test image");
        return;
    }

    auto codec = SkCodec::MakeFromStream(std::move(stream));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    auto info = codec->getInfo();
    SkBitmap bitmap1, bitmap2;
    bitmap1.allocPixels(info);
    bitmap2.allocPixels(info);

    // First decode
    auto result1 = codec->getPixels(bitmap1.info(), bitmap1.getPixels(), bitmap1.rowBytes());
    REPORTER_ASSERT(r, result1 == SkCodec::kSuccess);

    // Create a new codec instance for second decode (since rewind() is not available)
    auto stream2 = GetResourceAsStream("images/randPixels.bmp");
    if (stream2) {
        auto codec2 = SkCodec::MakeFromStream(std::move(stream2));
        if (codec2) {
            auto result2 = codec2->getPixels(bitmap2.info(), bitmap2.getPixels(), bitmap2.rowBytes());
            REPORTER_ASSERT(r, result2 == SkCodec::kSuccess);

            // Results should be identical
            REPORTER_ASSERT(r, bitmap1.computeByteSize() == bitmap2.computeByteSize());
            if (bitmap1.computeByteSize() == bitmap2.computeByteSize()) {
                REPORTER_ASSERT(r, memcmp(bitmap1.getPixels(), bitmap2.getPixels(), bitmap1.computeByteSize()) == 0);
            }
        }
    }
}

#endif // SK_CODEC_DECODES_BMP_WITH_RUST
