/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegMetadata_codec_DEFINED
#define SkJpegMetadata_codec_DEFINED

#include "include/private/SkJpegMetadataDecoder.h"

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
class SkJpegSourceMgr;
#endif

using SkJpegMarker = SkJpegMetadataDecoder::Segment;
using SkJpegMarkerList = std::vector<SkJpegMarker>;

class SkJpegMetadataDecoderImpl : public SkJpegMetadataDecoder {
public:
    SkJpegMetadataDecoderImpl(SkJpegMarkerList markerList);

    // SkJpegMetadataDecoder implementation.
    sk_sp<SkData> getExifMetadata(bool copyData) const override;
    sk_sp<SkData> getICCProfileData(bool copyData) const override;
    bool mightHaveGainmapImage() const override;
    bool findGainmapImage(sk_sp<SkData> baseImageData,
                          sk_sp<SkData>& outGainmapImageData,
                          SkGainmapInfo& outGainmapInfo) override;

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    static bool get_gainmap_info(const SkJpegMarkerList& markerList,
                                 SkJpegSourceMgr* sourceMgr,
                                 SkGainmapInfo* info,
                                 std::unique_ptr<SkStream>* gainmapImageStream);
#endif

private:
    SkJpegMarkerList fMarkerList;
};

#endif  // SkJpegMetadata_codec_DEFINED
