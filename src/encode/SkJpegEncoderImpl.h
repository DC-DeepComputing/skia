/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegEncoderImpl_DEFINED
#define SkJpegEncoderImpl_DEFINED

#include "include/encode/SkEncoder.h"
#include "include/encode/SkJpegEncoder.h"
#include "src/codec/SkJpegMetadataDecoderImpl.h"

#include <memory>

class SkJpegEncoderMgr;
class SkPixmap;
class SkYUVAPixmaps;

class SkJpegEncoderImpl : public SkEncoder {
public:
    // Common encode function through which all encodings paths run. Either |src| is non-nullptr,
    // or |srcYUVA| is non-nullptr. If |srcYUVA| is non-nullptr, then the color space for the image
    // as a whole is specified by |srcYUVAColorSpace|. Encoding options are specified in |options|.
    // Metadata markers are listed in |metadata|. The ICC profile and XMP metadata are read only
    // from |metadata| and not from |options|.
    static std::unique_ptr<SkEncoder> Make(SkWStream* dst,
                                           const SkPixmap* src,
                                           const SkYUVAPixmaps* srcYUVA,
                                           const SkColorSpace* srcYUVAColorSpace,
                                           const SkJpegEncoder::Options& options,
                                           const SkJpegMarkerList& metadataMarkers);

    SkJpegEncoderImpl(std::unique_ptr<SkJpegEncoderMgr>, const SkPixmap& src);
    SkJpegEncoderImpl(std::unique_ptr<SkJpegEncoderMgr>, const SkYUVAPixmaps* srcYUVA);

    ~SkJpegEncoderImpl() override;

protected:
    bool onEncodeRows(int numRows) override;

private:
    std::unique_ptr<SkJpegEncoderMgr> fEncoderMgr;
    const SkYUVAPixmaps* fSrcYUVA = nullptr;
};

// JPEG metadata is included in marker-based segments in the header of the image (the part before
// the first StartOfScan marker). These functions append these parameters to an SkJpegMarkerList.
namespace SkJpegMetadataEncoder {

// Include an ICC profile in the image. If |colorSpace| is nullptr, then include no profile. If
// |options| specifies ICC profile data, then use that data, otherwise, generate a profile for
// |colorSpace|.
void AppendICC(SkJpegMarkerList& markerList,
               const SkJpegEncoder::Options& options,
               const SkColorSpace* colorSpace);

// Include a standard (as opposed to extended) XMP metadata segment.
void AppendXMPStandard(SkJpegMarkerList& markerList, const SkData* xmpMetadata);

}  // namespace SkJpegMetadataEncoder

#endif
