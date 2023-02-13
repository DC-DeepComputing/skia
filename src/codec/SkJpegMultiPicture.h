/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegMultiPicture_codec_DEFINED
#define SkJpegMultiPicture_codec_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"

#include <cstdint>
#include <memory>
#include <vector>

class SkData;
class SkJpegSourceMgr;
struct SkJpegSegment;

/*
 * Parsed Jpeg Multi-Picture Format structure as specified in CIPA DC-x007-2009. An introduction to
 * the format can be found in Figure 1 (Basic MP File format data structure) and Figure 6 (Internal
 * Structure of the MP Index IFD) in that document. This parsing will extract only the size and
 * offset parameters from the images in the Index Image File Directory.
 */
struct SkJpegMultiPictureParameters {
    // An individual image.
    struct Image {
        // The size of the image in bytes.
        uint32_t size;
        // The offset of the image in bytes. This offset is specified relative to the address of
        // the MP Endian field in the MP Header, unless the image is a First Individual Image, in
        // which case the value of the offest [sic] shall be NULL (from section 5.2.3.3).
        uint32_t dataOffset;
    };

    // The images listed in the Index Image File Directory.
    std::vector<Image> images;
};

/*
 * Parse Jpeg Multi-Picture Format parameters. The specified data should be APP2 segment parameters,
 * which, if they are MPF parameter, should stat with the {'M', 'P', 'F', 0} signature. Returns
 * nullptr the parameters do not start with the MPF signature, or if there is an error in parsing
 * the parameters.
 */
std::unique_ptr<SkJpegMultiPictureParameters> SkJpegParseMultiPicture(
        const sk_sp<const SkData>& segmentParameters);

/*
 * Create SkData for all MultiPicture images, given an SkJpegSourceMgr for an image, and the
 * SkJpegSegment whose parameters produced the parameters. This will return an empty vector if there
 * is not MultiPicture segment, or if the MultiPicture parameters fail to parse.
 *
 * The returned vector will have an entry for each image listed in the Index Image File Directory.
 * The entry is nullptr for the First Individual Image and for any images which encounter errors
 * (e.g, they are outside of the range of the stream). If possible, the result will refer directly
 * to memory owned by stream of the SkJpegSourceMgr that was used to create this object.
 */
std::vector<sk_sp<SkData>> SkJpegExtractMultiPictureImageData(
        const SkJpegMultiPictureParameters* mpParams,
        const SkJpegSegment& mpParamsSegment,
        SkJpegSourceMgr* decoderSource);

/*
 * Serialize Jpeg Multi-Picture Format parameters into a segment. This segment will start with the
 * {'M', 'P', 'F', 0} signature (it will not include the segment marker or parameter length).
 */
sk_sp<SkData> SkJpegSerializeMultiPicture(const SkJpegMultiPictureParameters* parameters);

/*
 * Merge the encoded Jpegs listed in |images| into a single MPF image, and write it to the stream
 * |dst|. If there exists an MPF segment in the first element of |images|, then that segment will
 * not be included in the output. The output stream will have the contents of |images| tightly
 * packed, with no padding inserted between. Return true on success and false on failure.
 */
bool SkJpegMakeMultiPicture(SkWStream* dst, const std::vector<sk_sp<SkData>>& images);

#endif
