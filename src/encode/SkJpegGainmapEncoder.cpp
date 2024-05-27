/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkJpegGainmapEncoder.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkEncoder.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/SkGainmapInfo.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegMultiPicture.h"
#include "src/codec/SkJpegPriv.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/encode/SkJpegEncoderImpl.h"

#include <vector>

static bool is_single_channel(SkColor4f c) { return c.fR == c.fG && c.fG == c.fB; };

////////////////////////////////////////////////////////////////////////////////////////////////////
// HDRGM encoding

// Generate the XMP metadata for an HDRGM file.
sk_sp<SkData> get_gainmap_image_xmp_metadata(const SkGainmapInfo& gainmapInfo) {
    SkDynamicMemoryWStream s;
    const float kLog2 = std::log(2.f);
    const SkColor4f gainMapMin = {std::log(gainmapInfo.fGainmapRatioMin.fR) / kLog2,
                                  std::log(gainmapInfo.fGainmapRatioMin.fG) / kLog2,
                                  std::log(gainmapInfo.fGainmapRatioMin.fB) / kLog2,
                                  1.f};
    const SkColor4f gainMapMax = {std::log(gainmapInfo.fGainmapRatioMax.fR) / kLog2,
                                  std::log(gainmapInfo.fGainmapRatioMax.fG) / kLog2,
                                  std::log(gainmapInfo.fGainmapRatioMax.fB) / kLog2,
                                  1.f};
    const SkColor4f gamma = {1.f / gainmapInfo.fGainmapGamma.fR,
                             1.f / gainmapInfo.fGainmapGamma.fG,
                             1.f / gainmapInfo.fGainmapGamma.fB,
                             1.f};
    // Write a scalar attribute.
    auto write_scalar_attr = [&s](const char* attrib, SkScalar value) {
        s.writeText("        ");
        s.writeText(attrib);
        s.writeText("=\"");
        s.writeScalarAsText(value);
        s.writeText("\"\n");
    };

    // Write a scalar attribute only if all channels of |value| are equal (otherwise, write
    // nothing).
    auto maybe_write_scalar_attr = [&write_scalar_attr](const char* attrib, SkColor4f value) {
        if (!is_single_channel(value)) {
            return;
        }
        write_scalar_attr(attrib, value.fR);
    };

    // Write a float3 attribute as a list ony if not all channels of |value| are equal (otherwise,
    // write nothing).
    auto maybe_write_float3_attr = [&s](const char* attrib, SkColor4f value) {
        if (is_single_channel(value)) {
            return;
        }
        s.writeText("      <");
        s.writeText(attrib);
        s.writeText(">\n");
        s.writeText("        <rdf:Seq>\n");
        s.writeText("          <rdf:li>");
        s.writeScalarAsText(value.fR);
        s.writeText("</rdf:li>\n");
        s.writeText("          <rdf:li>");
        s.writeScalarAsText(value.fG);
        s.writeText("</rdf:li>\n");
        s.writeText("          <rdf:li>");
        s.writeScalarAsText(value.fB);
        s.writeText("</rdf:li>\n");
        s.writeText("        </rdf:Seq>\n");
        s.writeText("      </");
        s.writeText(attrib);
        s.writeText(">\n");
    };

    s.writeText(
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 5.5.0\">\n"
            "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "    <rdf:Description rdf:about=\"\"\n"
            "        xmlns:hdrgm=\"http://ns.adobe.com/hdr-gain-map/1.0/\"\n"
            "        hdrgm:Version=\"1.0\"\n");
    maybe_write_scalar_attr("hdrgm:GainMapMin", gainMapMin);
    maybe_write_scalar_attr("hdrgm:GainMapMax", gainMapMax);
    maybe_write_scalar_attr("hdrgm:Gamma", gamma);
    maybe_write_scalar_attr("hdrgm:OffsetSDR", gainmapInfo.fEpsilonSdr);
    maybe_write_scalar_attr("hdrgm:OffsetHDR", gainmapInfo.fEpsilonHdr);
    write_scalar_attr("hdrgm:HDRCapacityMin", std::log(gainmapInfo.fDisplayRatioSdr) / kLog2);
    write_scalar_attr("hdrgm:HDRCapacityMax", std::log(gainmapInfo.fDisplayRatioHdr) / kLog2);
    switch (gainmapInfo.fBaseImageType) {
        case SkGainmapInfo::BaseImageType::kSDR:
            s.writeText("        hdrgm:BaseRenditionIsHDR=\"False\">\n");
            break;
        case SkGainmapInfo::BaseImageType::kHDR:
            s.writeText("        hdrgm:BaseRenditionIsHDR=\"True\">\n");
            break;
    }

    // Write any of the vector parameters that cannot be represented as scalars (and thus cannot
    // be written inline as above).
    maybe_write_float3_attr("hdrgm:GainMapMin", gainMapMin);
    maybe_write_float3_attr("hdrgm:GainMapMax", gainMapMax);
    maybe_write_float3_attr("hdrgm:Gamma", gamma);
    maybe_write_float3_attr("hdrgm:OffsetSDR", gainmapInfo.fEpsilonSdr);
    maybe_write_float3_attr("hdrgm:OffsetHDR", gainmapInfo.fEpsilonHdr);
    s.writeText(
            "    </rdf:Description>\n"
            "  </rdf:RDF>\n"
            "</x:xmpmeta>");
    return s.detachAsData();
}

// Generate the GContainer metadata for an image with a JPEG gainmap.
static sk_sp<SkData> get_base_image_xmp_metadata(size_t gainmapItemLength) {
    SkDynamicMemoryWStream s;
    s.writeText(
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP Core 5.1.2\">\n"
            "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "    <rdf:Description\n"
            "        xmlns:Container=\"http://ns.google.com/photos/1.0/container/\"\n"
            "        xmlns:Item=\"http://ns.google.com/photos/1.0/container/item/\"\n"
            "        xmlns:hdrgm=\"http://ns.adobe.com/hdr-gain-map/1.0/\"\n"
            "        hdrgm:Version=\"1.0\">\n"
            "      <Container:Directory>\n"
            "        <rdf:Seq>\n"
            "          <rdf:li rdf:parseType=\"Resource\">\n"
            "            <Container:Item\n"
            "             Item:Semantic=\"Primary\"\n"
            "             Item:Mime=\"image/jpeg\"/>\n"
            "          </rdf:li>\n"
            "          <rdf:li rdf:parseType=\"Resource\">\n"
            "            <Container:Item\n"
            "             Item:Semantic=\"GainMap\"\n"
            "             Item:Mime=\"image/jpeg\"\n"
            "             Item:Length=\"");
    s.writeDecAsText(gainmapItemLength);
    s.writeText(
            "\"/>\n"
            "          </rdf:li>\n"
            "        </rdf:Seq>\n"
            "      </Container:Directory>\n"
            "    </rdf:Description>\n"
            "  </rdf:RDF>\n"
            "</x:xmpmeta>\n");
    return s.detachAsData();
}

static sk_sp<SkData> encode_to_data(const SkPixmap& pm,
                                    const SkJpegEncoder::Options& options,
                                    const SkJpegMetadataEncoder::SegmentList& metadataSegments) {
    SkDynamicMemoryWStream encodeStream;
    auto encoder = SkJpegEncoderImpl::MakeRGB(&encodeStream, pm, options, metadataSegments);
    if (!encoder || !encoder->encodeRows(pm.height())) {
        return nullptr;
    }
    return encodeStream.detachAsData();
}

static sk_sp<SkData> get_mpf_segment(const SkJpegMultiPictureParameters& mpParams,
                                     size_t imageNumber) {
    SkDynamicMemoryWStream s;
    auto segmentParameters = mpParams.serialize(static_cast<uint32_t>(imageNumber));
    const size_t mpParameterLength = kJpegSegmentParameterLengthSize + segmentParameters->size();
    s.write8(0xFF);
    s.write8(kMpfMarker);
    s.write8(mpParameterLength / 256);
    s.write8(mpParameterLength % 256);
    s.write(segmentParameters->data(), segmentParameters->size());
    return s.detachAsData();
}

static sk_sp<SkData> get_iso_gainmap_segment_params(sk_sp<SkData> data) {
    SkDynamicMemoryWStream s;
    s.write(kISOGainmapSig, sizeof(kISOGainmapSig));
    s.write(data->data(), data->size());
    return s.detachAsData();
}

bool SkJpegGainmapEncoder::EncodeHDRGM(SkWStream* dst,
                                       const SkPixmap& base,
                                       const SkJpegEncoder::Options& baseOptions,
                                       const SkPixmap& gainmap,
                                       const SkJpegEncoder::Options& gainmapOptions,
                                       const SkGainmapInfo& gainmapInfo) {
    bool includeUltraHDRv1 = gainmapInfo.isUltraHDRv1Compatible();

    // Encode the gainmap image.
    sk_sp<SkData> gainmapData;
    {
        SkJpegMetadataEncoder::SegmentList metadataSegments;

        // Add XMP metadata.
        if (includeUltraHDRv1) {
            SkJpegMetadataEncoder::AppendXMPStandard(
                    metadataSegments, get_gainmap_image_xmp_metadata(gainmapInfo).get());
        }

        // Include the ICC profile of the alternate color space, if it is used.
        if (gainmapInfo.fGainmapMathColorSpace) {
            SkJpegMetadataEncoder::AppendICC(
                    metadataSegments, gainmapOptions, gainmapInfo.fGainmapMathColorSpace.get());
        }

        // Add the ISO 21946-1 metadata.
        metadataSegments.emplace_back(kISOGainmapMarker,
                                      get_iso_gainmap_segment_params(gainmapInfo.serialize()));

        // Encode the gainmap image.
        gainmapData = encode_to_data(gainmap, gainmapOptions, metadataSegments);
        if (!gainmapData) {
            SkCodecPrintf("Failed to encode gainmap image.\n");
            return false;
        }
    }

    // Encode the base image.
    sk_sp<SkData> baseData;
    {
        SkJpegMetadataEncoder::SegmentList metadataSegments;

        // Include XMP.
        if (includeUltraHDRv1) {
            size_t gainmapDataSize = gainmapData->size() +
                                     get_mpf_segment(SkJpegMultiPictureParameters(2), 1)->size();
            SkJpegMetadataEncoder::AppendXMPStandard(
                    metadataSegments,
                    get_base_image_xmp_metadata(static_cast<int32_t>(gainmapDataSize)).get());
        }

        // Include ICC profile metadata.
        SkJpegMetadataEncoder::AppendICC(metadataSegments, baseOptions, base.colorSpace());

        // Include the ISO 21946-1 version metadata.
        metadataSegments.emplace_back(
                kISOGainmapMarker,
                get_iso_gainmap_segment_params(SkGainmapInfo::SerializeVersion()));

        // Encode the base image.
        baseData = encode_to_data(base, baseOptions, metadataSegments);
        if (!baseData) {
            SkCodecPrintf("Failed to encode base image.\n");
            return false;
        }
    }

    // Combine them into an MPF.
    const SkData* images[] = {
            baseData.get(),
            gainmapData.get(),
    };
    return MakeMPF(dst, images, 2);
}

bool SkJpegGainmapEncoder::MakeMPF(SkWStream* dst, const SkData** images, size_t imageCount) {
    if (imageCount < 1) {
        return true;
    }

    SkJpegMultiPictureParameters mpParams(imageCount);

    // Let |mpSegments| be the full segment that will be added to each image. Initialize them, and
    // use that to populate sizes in mpParams. Note that the content (but not size) of segment 0 is
    // still incorrect, because we have not yet computed the appropriate offsets.
    std::vector<sk_sp<SkData>> mpSegments(imageCount);
    for (size_t i = 0; i < imageCount; ++i) {
        mpSegments[i] = get_mpf_segment(mpParams, i);
        mpParams.images[i].size = static_cast<uint32_t>(images[i]->size() + mpSegments[i]->size());
    }

    // The MP header offset into the first individual image off of which the other image offsets are
    // computed.
    size_t mpHeaderOffset = 0;

    for (size_t i = 0; i < imageCount; ++i) {
        auto image = images[i];

        // Scan the image until StartOfScan marker.
        SkJpegSegmentScanner scan(kJpegMarkerStartOfScan);
        scan.onBytes(image->data(), image->size());
        if (!scan.isDone()) {
            SkCodecPrintf("Failed to scan encoded image header.\n");
            return false;
        }

        // We'll insert the MPF segment just before the StartOfScan.
        // TODO(b/338342146): This is technically not compliant. See 5.1. Basic MP File Structure,
        // which indicates "The MP Extensions are specified in the APP2 marker segment which follows
        // immediately after the Exif Attributes in the APP1 marker segment expect as specified in
        // section 7". We currently do not include Exif metadata in encoded files yet.
        size_t mpSegmentOffset = scan.getSegments().back().offset;

        // If this is the first image, we can now compute the MPF offsets, and recompute the first
        // individiual image's parameters.
        if (i == 0) {
            mpHeaderOffset = mpSegmentOffset + kJpegMarkerCodeSize +
                             kJpegSegmentParameterLengthSize + sizeof(kMpfSig);
            uint32_t offset = mpParams.images[0].size - mpHeaderOffset;
            for (size_t j = 1; j < imageCount; ++j) {
                mpParams.images[j].dataOffset = offset;
                offset += mpParams.images[j].size;
            }
            mpSegments[0] = get_mpf_segment(mpParams, 0);
        }

        // Write the image up to the MPF segment, then the MPF segment, then the rest of the image.
        if (!dst->write(image->bytes(), mpSegmentOffset) ||
            !dst->write(mpSegments[i]->data(), mpSegments[i]->size()) ||
            !dst->write(image->bytes() + mpSegmentOffset, image->size() - mpSegmentOffset)) {
            SkCodecPrintf("Failed to write remainder of primary image.\n");
            return false;
        }
    }

    return true;
}
