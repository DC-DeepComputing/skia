/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkExif_DEFINED
#define SkExif_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

#include <cstdint>
#include <optional>

struct SK_API SkExifMetadata {
public:
    SkExifMetadata() = default;

    /*
     * Parse the metadata specified in |data| and write them to |this|. Stop only at an
     * unrecoverable error (allow truncated input).
     */
    void parse(sk_sp<SkData> data);

    // The image orientation.
    static constexpr uint16_t kOriginTag = 0x112;
    std::optional<SkEncodedOrigin> fOrigin;

    // The HDR headroom property.
    // https://developer.apple.com/documentation/appkit/images_and_pdf/applying_apple_hdr_effect_to_your_photos
    std::optional<float> fHdrHeadroom;

    // Resolution.
    static constexpr uint16_t kResolutionUnitTag = 0x0128;
    std::optional<uint16_t> fResolutionUnit;
    static constexpr uint16_t kXResolutionTag = 0x011a;
    std::optional<float> fXResolution;
    static constexpr uint16_t kYResolutionTag = 0x011b;
    std::optional<float> fYResolution;

    // Size in pixels.
    static constexpr uint16_t kPixelXDimensionTag = 0xa002;
    std::optional<uint32_t> fPixelXDimension;
    static constexpr uint16_t kPixelYDimensionTag = 0xa003;
    std::optional<uint32_t> fPixelYDimension;
};

#endif
