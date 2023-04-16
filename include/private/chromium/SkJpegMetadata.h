/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegMetadata_DEFINED
#define SkJpegMetadata_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

#include <memory>
#include <vector>

struct SkGainmapInfo;

class SK_SPI SkJpegMetadata {
public:
    // A segment from a JPEG file. This is usually populated from a jpeg_marker_struct.
    struct SK_SPI Segment {
        Segment(uint8_t marker, sk_sp<SkData> data) : fMarker(marker), fData(std::move(data)) {}

        // The segment's marker.
        uint8_t fMarker = 0;

        // The segment's parameters (not including the marker and parameter length).
        sk_sp<SkData> fData;
    };

    // Create metadata for the specified segments from a JPEG file's header (defined as all segments
    // before the first StartOfScan). This may return nullptr.
    static std::unique_ptr<SkJpegMetadata> Make(std::vector<Segment> headerSegments);
    virtual ~SkJpegMetadata() {}

    // Given a image's full data, return in |offset| and |size| the subset of that image that
    // corresponds to the gainmap image in |info| the gainmap rendering parameters.
    virtual bool findGainmapImage(sk_sp<SkData> baseImageData,
                                  sk_sp<SkData>& outGainmapImagedata,
                                  SkGainmapInfo& outGainmapInfo) = 0;
};

#endif
