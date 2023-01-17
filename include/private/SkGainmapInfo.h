/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGainmapInfo_DEFINED
#define SkGainmapInfo_DEFINED

/**
 *  Gainmap rendering parameters. Suppose our display has HDR to SDR ratio of H, and we wish to
 *  display an image with gainmap on this display. Let S be the pixel value from the SDR base
 *  image, in a color space that has the primaries of the SDR base image and a linear transfer
 *  function. Let G be the pixel value from the gainmap. Let D be the output pixel in the same
 *  color space as S. The value of D is computed as follows:
 *
 *  First, let W be a weight parameter determing how much the gainmap will be applied.
 *    W = clamp((log(H) - log(fHdrRatioMax)) / (log(fHdrRatioMax) - log(fHdrRatioMin), 0, 1)
 *
 *  Next, let L be the gainmap value in log space. We compute this from the value G that was
 *  sampled from the texture as follows:
 *    L = mix(log(fGainmapRatioMin), log(fGainmapRatioMax), pow(G, fGainmapGamma))
 *
 *  Finally, apply the gainmap to compute D, the displayed pixel.
 *    D = (S + fEpsilonSdr) * exp(L * W) - fEpsilonHdr
 *
 *  In the above math, log() is a natural logarithm and exp() is natural exponentiation.
 */
struct SkGainmapInfo {
    /**
     *  Parameters for converting the gainmap from its image encoding to the space of HDR to SDR
     *  ratios. These are specified per color channel.
     */
    float fGainmapRatioMin[3] = {0.f, 0.f, 0.f};
    float fGainmapRatioMax[3] = {1.f, 1.f, 1.f};
    float fGainmapGamma[3] = {1.f, 1.f, 1.f};

    /**
     *  Parameters that are sometimes used to avoid divide-by-zero errors.
     */
    float fEpsilonSdr = 0.0f;
    float fEpsilonHdr = 0.0f;

    /**
     *  Parameters that indicate the minimum HDR capability below which the gainmap is not
     *  applied at all, and the maximum HDR capacity above which the gainmap is fully applied.
     */
    float fHdrRatioMin = 1.f;
    float fHdrRatioMax = 100.f;

    /**
     *  The type of file that created this gainmap.
     */
    enum class Type {
        kUnknown,
        kMultiPicture,
        kJpegR_Linear,
        kJpegR_HLG,
        kJpegR_PQ,
    };
    Type fType = Type::kUnknown;
};

#endif
