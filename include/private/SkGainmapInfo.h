/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGainmapInfo_DEFINED
#define SkGainmapInfo_DEFINED

#include "include/core/SkColor.h"

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
 *    L = mix(log(fGainmapMin), log(fGainmapMax), pow(G, fGainmapGamma))
 *
 *  Finally, apply the gainmap to compute D, the displayed pixel.
 *    D = (S + fEpsilonSdr) * exp(L * W) - fEpsilonHdr
 *
 *  In the above math, log() is a natural logarithm and exp() is natural exponentiation.
 */
struct SkGainmapInfo {
    /**
     *  The maximum and minimum HDR to SDR ratio (in linear space) that the gainmap image can
     *  encode, and the gamma used to encode the gainmap image. These are specified per color
     *  channel. The alpha value is unused.
     */
    SkColor4f fGainmapMin = {1.f, 1.f, 1.f, 1.0};
    SkColor4f fGainmapMax = {2.f, 2.f, 2.f, 1.0};
    SkColor4f fGainmapGamma = {1.f, 1.f, 1.f, 1.f};

    /**
     *  Parameters selected to avoid divide by zero errors while creating the gainmap. These
     *  may be preserved or set to zero for rendering.
     */
    float fEpsilonSdr = 0.f;
    float fEpsilonHdr = 0.f;

    /**
     *  Parameters that indicate the minimum HDR capability below which the gainmap is not
     *  applied at all, and the maximum HDR capacity above which the gainmap is fully applied.
     */
    float fHdrRatioMin = 1.f;
    float fHdrRatioMax = 2.f;
};

#endif
