/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkDataTable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <memory>

DEF_TEST(AlphaEncodedInfo, r) {
    auto codec = SkCodec::MakeFromStream(GetResourceAsStream("images/grayscale.jpg"));
    REPORTER_ASSERT(r, codec->getInfo().colorType() == kGray_8_SkColorType);

    SkBitmap bm;
    bm.allocPixels(codec->getInfo().makeColorType(kAlpha_8_SkColorType).makeColorSpace(nullptr));
    auto result = codec->getPixels(codec->getInfo(), bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    sk_sp<SkData> data = SkPngEncoder::Encode(bm.pixmap(), {});
    REPORTER_ASSERT(r, data);
    REPORTER_ASSERT(r, data->size() > 0);

    codec = SkCodec::MakeFromData(data);
    REPORTER_ASSERT(r, codec);
    // TODO: Make SkEncodedInfo public and compare to its version of kAlpha_8.
    REPORTER_ASSERT(r, codec->getInfo().colorType() == kAlpha_8_SkColorType);

    SkBitmap bm2;
    bm2.allocPixels(codec->getInfo().makeColorSpace(nullptr));
    result = codec->getPixels(bm2.pixmap());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    REPORTER_ASSERT(r, ToolUtils::equal_pixels(bm.pixmap(), bm2.pixmap()));
}

DEF_TEST(IncompleteAlpha8Decode, r) {
    SkBitmap srcBm;
    srcBm.allocPixels(SkImageInfo::Make(64, 64, kAlpha_8_SkColorType, kPremul_SkAlphaType));
    // Fill with 0x55 (distinct from zero and the 0xAA canary).
    memset(srcBm.getPixels(), 0x55, srcBm.computeByteSize());

    sk_sp<SkData> data = SkPngEncoder::Encode(srcBm.pixmap(), {});
    REPORTER_ASSERT(r, data && data->size() > 0);

    // Try several truncation lengths to test various points in the IDAT stream.
    for (double frac : { 0.25, 0.40, 0.50, 0.75 }) {
        size_t truncateLen = static_cast<size_t>(data->size() * frac);
        sk_sp<SkData> truncated = SkData::MakeSubset(data.get(), 0, truncateLen);
        auto codec = SkCodec::MakeFromData(truncated);
        if (!codec) {
            continue;
        }

        REPORTER_ASSERT(r, codec->getInfo().colorType() == kAlpha_8_SkColorType);

        SkBitmap dstBm;
        dstBm.allocPixels(codec->getInfo().makeColorSpace(nullptr));
        // Fill destination with 0xAA canary to detect untouched memory.
        memset(dstBm.getPixels(), 0xAA, dstBm.computeByteSize());

        SkCodec::Result result = codec->getPixels(dstBm.pixmap());
        if (result == SkCodec::kIncompleteInput || result == SkCodec::kErrorInInput) {
            const uint8_t* pixels = static_cast<const uint8_t*>(dstBm.getPixels());
            bool hasCanary = false;
            for (size_t i = 0; i < dstBm.computeByteSize(); ++i) {
                if (pixels[i] == 0xAA) {
                    hasCanary = true;
                    break;
                }
            }
            REPORTER_ASSERT(r, !hasCanary);
        }
    }
}

