/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkJpegGainmapEncoder.h"

#ifdef SK_ENCODE_JPEG

#include "include/core/SkBitmap.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/SkGainmapInfo.h"
#include "src/codec/SkJpegPriv.h"

static float mix(float a, float b, float amount) { return (b - a) * amount + a; }

static float compute_range_scaling_factor(const SkGainmapInfo& info) {
    // Find the minimum and maximum log-ratio values that can be encoded. We don't want to encode a
    // range any larger than this.
    const float loadLogRatioMaxComponent =
            std::max(std::max(info.fLogRatioMax.fR, info.fLogRatioMax.fG), info.fLogRatioMax.fB);
    const float loadLogRatioMinComponent =
            std::min(std::min(info.fLogRatioMin.fR, info.fLogRatioMin.fG), info.fLogRatioMin.fB);
    const float logRatioRSF =
            sk_float_exp(std::max(loadLogRatioMaxComponent, -loadLogRatioMinComponent));

    // Limit the range to only encode values that could reach the the maximum rendering brightness.
    float hdrRatioMaxRSF = info.fHdrRatioMax;

    return std::min(logRatioRSF, hdrRatioMaxRSF);
}

// Ensure that the specified gainmap can be encoded as a JpegR. If it cannot, transform it so that
// it can.
void make_jpegr_compatible_if_needed(SkGainmapInfo& info, SkBitmap& bitmap) {
    // If fLogRatioMin == -fLogRatioMax and bitmap has a single channel then this is already
    // compatible with JpegR.
    if (info.fLogRatioMin.fR == -info.fLogRatioMax.fR &&
        info.fLogRatioMin.fG == -info.fLogRatioMax.fG &&
        info.fLogRatioMin.fB == -info.fLogRatioMax.fB &&
        bitmap.colorType() == kGray_8_SkColorType) {
        return;
    }

    // If not, transform the gainmap to a JpegR compatible format.
    SkGainmapInfo oldInfo = info;
    SkBitmap oldBitmap = bitmap;
    SkBitmap newBitmap;
    SkImageInfo newBitmapInfo =
            SkImageInfo::Make(oldBitmap.dimensions(), kGray_8_SkColorType, kOpaque_SkAlphaType);
    newBitmap.allocPixels(newBitmapInfo);

    // Compute the new gainmap rangeScalingFactor and its log.
    const float rangeScalingFactor = compute_range_scaling_factor(oldInfo);
    const float newLogRatioMax = sk_float_log(rangeScalingFactor);
    const float newLogRatioMin = -newLogRatioMax;

    // Transform the old gainmap to the new range.
    {
        const SkColor4f oldLogRatioMin = oldInfo.fLogRatioMin;
        const SkColor4f oldLogRatioMax = oldInfo.fLogRatioMax;
        const SkColor4f gainmapGamma = oldInfo.fGainmapGamma;
        auto newPixmap = newBitmap.pixmap();
        for (int y = 0; y < oldBitmap.height(); ++y) {
            for (int x = 0; x < oldBitmap.width(); ++x) {
                // Convert the gainmap from its encoded value to oldLogRatio, which is log(HDR/SDR).
                SkColor4f oldG = oldBitmap.getColor4f(x, y);
                SkColor4f oldLogRatio = {
                        mix(oldLogRatioMin.fR,
                            oldLogRatioMax.fR,
                            sk_float_pow(oldG.fR, gainmapGamma.fR)),
                        mix(oldLogRatioMin.fG,
                            oldLogRatioMax.fG,
                            sk_float_pow(oldG.fG, gainmapGamma.fG)),
                        mix(oldLogRatioMin.fB,
                            oldLogRatioMax.fB,
                            sk_float_pow(oldG.fB, gainmapGamma.fB)),
                        1.f,
                };

                // Undo the log, computing HDR/SDR, and take the average of the components of this.
                // TODO(ccameron): This assumes that the primaries of the base image are sRGB.
                float averageLinearRatio = 0.2126f * sk_float_exp(oldLogRatio.fR) +
                                           0.7152f * sk_float_exp(oldLogRatio.fG) +
                                           0.0722f * sk_float_exp(oldLogRatio.fB);

                // Compute log(HDR/SDR) for the average HDR/SDR ratio.
                float newLogRatio = sk_float_log(averageLinearRatio);

                // Convert from log(HDR/SDR) to the JpegR gainmap image encoding.
                float newG = (newLogRatio - newLogRatioMin) / (newLogRatioMax - newLogRatioMin);
                *newPixmap.writable_addr8(x, y) =
                        std::min(std::max(sk_float_round(255.f * newG), 0.f), 255.f);
            }
        }
    }

    // Write the gainmap info for the transformed gainmap.
    SkGainmapInfo newInfo;
    newInfo.fLogRatioMin = {newLogRatioMin, newLogRatioMin, newLogRatioMin, 1.f};
    newInfo.fLogRatioMax = {newLogRatioMax, newLogRatioMax, newLogRatioMax, 1.f};
    newInfo.fGainmapGamma = {1.f, 1.f, 1.f, 1.f};
    newInfo.fEpsilonSdr = 0.f;
    newInfo.fEpsilonHdr = 0.f;
    newInfo.fHdrRatioMin = 1.f;
    newInfo.fHdrRatioMax = sk_float_exp(newLogRatioMax);
    newInfo.fType = SkGainmapInfo::Type::kJpegR_Linear;
    info = newInfo;
    bitmap = newBitmap;
}

// Generate the XMP metadata for a JpegR file.
sk_sp<SkData> get_jpegr_xmp_data(float rangeScalingFactor,
                                 int32_t transferFunction,
                                 int32_t itemLength) {
    SkDynamicMemoryWStream s;
    s.write(kXMPSig, sizeof(kXMPSig));
    s.writeText(
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP Core 5.1.2\">\n"
            "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "<rdf:Description xmlns:GContainer=\"http://ns.google.com/photos/1.0/container/\" "
            "xmlns:RecoveryMap=\"http://ns.google.com/photos/1.0/recoverymap/\">\n"
            "<GContainer:Version>1</GContainer:Version>\n"
            "<GContainer:Directory>\n"
            "<rdf:Seq>\n"
            "<rdf:li>\n"
            "<GContainer:Item GContainer:ItemSemantic=\"Primary\" "
            "GContainer:ItemMime=\"image/jpeg\" "
            "RecoveryMap:Version=\"1\" "
            "RecoveryMap:RangeScalingFactor=\"");
    s.writeScalarAsText(rangeScalingFactor);
    s.writeText("\" ");
    s.writeText("RecoveryMap:TransferFunction=\"");
    s.writeDecAsText(transferFunction);
    s.writeText("\"/>\n");
    s.writeText(
            "</rdf:li>\n"
            "<rdf:li>\n"
            "<GContainer:Item GContainer:ItemSemantic=\"RecoveryMap\" "
            "GContainer:ItemMime=\"image/jpeg\" "
            "GContainer:ItemLength=\"");
    s.writeDecAsText(itemLength);
    s.writeText("\"/>\n");
    s.writeText(
            "</rdf:li>\n"
            "</rdf:Seq>\n"
            "</GContainer:Directory>\n"
            "</rdf:Description>\n"
            "</rdf:RDF>\n"
            "</x:xmpmeta>\n");
    return s.detachAsData();
}

bool SkJpegGainmapEncoder::EncodeJpegR(SkWStream* dst,
                                       const SkPixmap& base,
                                       const SkJpegEncoder::Options& baseOptions,
                                       const SkPixmap& gainmap,
                                       const SkJpegEncoder::Options& gainmapOptions,
                                       const SkGainmapInfo& gainmapInfo) {
    // Transform the gainmap to be compatible with JpegR, if needed.
    SkBitmap gainmapJpegR;
    gainmapJpegR.installPixels(gainmap);
    SkGainmapInfo gainmapInfoJpegR = gainmapInfo;
    make_jpegr_compatible_if_needed(gainmapInfoJpegR, gainmapJpegR);

    // Encode the gainmap as a Jpeg.
    SkDynamicMemoryWStream gainmapEncodeStream;
    if (!SkJpegEncoder::Encode(&gainmapEncodeStream, gainmapJpegR.pixmap(), gainmapOptions)) {
        return false;
    }
    sk_sp<SkData> gainmapEncoded = gainmapEncodeStream.detachAsData();

    // Compute the XMP metadata.
    sk_sp<SkData> xmpMetadata =
            get_jpegr_xmp_data(gainmapInfo.fHdrRatioMax, 0, gainmapEncoded->size());

    // Send this to the base image encoder.
    auto encoder =
            SkJpegEncoder::Make(dst, base, baseOptions, xmpMetadata.get(), gainmapEncoded.get());
    return encoder.get() && encoder->encodeRows(base.height());
}
#endif  // SK_ENCODE_JPEG
