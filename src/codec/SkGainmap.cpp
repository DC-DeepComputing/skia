/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkGainmapShader.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/private/SkGainmapInfo.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkFilterPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

static constexpr char gGainmapApplySKSL[] =
        "uniform shader base;"
        "uniform shader gainmap;"
        "uniform half4 logGainmapMin;"
        "uniform half4 logGainmapMax;"
        "uniform half4 gainmapGamma;"
        "uniform half epsilonSdr;"
        "uniform half epsilonHdr;"
        "uniform half W;"
        ""
        "half4 main(float2 coord) {"
        "half4 S = base.eval(coord);"
        "half4 G = gainmap.eval(coord);"
        "half3 L = mix(logGainmapMin.rgb, logGainmapMax.rgb, pow(G.rgb, gainmapGamma.rgb));"
        "half3 H = (S.rgb + epsilonSdr) * exp(L * W) - epsilonHdr;"
        "return half4(H.r, H.g, H.b, S.a);"
        "}";

static sk_sp<SkRuntimeEffect> gainmap_apply_effect() {
    static const SkRuntimeEffect* effect =
            SkRuntimeEffect::MakeForShader(SkString(gGainmapApplySKSL), {}).effect.release();
    SkASSERT(effect);
    return sk_ref_sp(effect);
}

sk_sp<SkShader> SkGainmapGetShader(sk_sp<SkImage> baseImage,
                                   const SkRect& baseRect,
                                   const SkSamplingOptions& baseSamplingOptions,
                                   sk_sp<SkImage> gainmapImage,
                                   const SkRect& gainmapRect,
                                   const SkSamplingOptions& gainmapSamplingOptions,
                                   const SkGainmapInfo& gainmapInfo,
                                   const SkRect& dstRect,
                                   float dstHdrRatio,
                                   sk_sp<SkColorSpace> dstColorSpace) {
    sk_sp<SkColorSpace> baseColorSpace =
            baseImage->colorSpace() ? baseImage->refColorSpace() : SkColorSpace::MakeSRGB();

    // Determine the color space in which the gainmap math is to be applied.
    sk_sp<SkColorSpace> gainmapMathColorSpace = baseColorSpace->makeLinearGamma();
    if (!dstColorSpace) {
        dstColorSpace = SkColorSpace::MakeSRGB();
    }

    // Create a color filter to transform from the base image's color space to the color space in
    // which the gainmap is to be applied.
    auto colorXformSdrToGainmap =
            SkFilterPriv::MakeColorSpaceXform(baseColorSpace, gainmapMathColorSpace);

    // Create a color filter to transform from the color space in which the gainmap is applied to
    // the destination color space.
    auto colorXformGainmapToDst =
            SkFilterPriv::MakeColorSpaceXform(gainmapMathColorSpace, dstColorSpace);

    // The base image shader will convert into the color space in which the gainmap is applied.
    const SkMatrix baseRectToDstRect = SkMatrix::RectToRect(baseRect, dstRect);
    auto baseImageShader = baseImage->reinterpretColorSpace(nullptr)
                                   ->makeShader(baseSamplingOptions, &baseRectToDstRect)
                                   ->makeWithColorFilter(colorXformSdrToGainmap);

    // The gainmap image shader will ignore any color space that the gainmap has.
    const SkMatrix gainmapRectToDstRect = SkMatrix::RectToRect(gainmapRect, dstRect);
    auto gainmapImageShader = gainmapImage->reinterpretColorSpace(nullptr)->makeShader(
            gainmapSamplingOptions, &gainmapRectToDstRect);

    // Create the shader to apply the gainmap.
    sk_sp<SkShader> gainmapMathShader;
    {
        SkRuntimeShaderBuilder builder(gainmap_apply_effect());
        const SkColor4f logGainmapMin({sk_float_log(gainmapInfo.fGainmapMin.fR),
                                       sk_float_log(gainmapInfo.fGainmapMin.fG),
                                       sk_float_log(gainmapInfo.fGainmapMin.fB),
                                       1.f});
        const SkColor4f logGainmapMax({sk_float_log(gainmapInfo.fGainmapMax.fR),
                                       sk_float_log(gainmapInfo.fGainmapMax.fG),
                                       sk_float_log(gainmapInfo.fGainmapMax.fB),
                                       1.f});
        const float Wunclamped =
                (sk_float_log(dstHdrRatio) - sk_float_log(gainmapInfo.fRenderRatioMin)) /
                (sk_float_log(gainmapInfo.fRenderRatioMax) -
                 sk_float_log(gainmapInfo.fRenderRatioMin));
        const float W = std::max(std::min(Wunclamped, 1.f), 0.f);

        builder.child("base") = baseImageShader;
        builder.child("gainmap") = gainmapImageShader;
        builder.uniform("logGainmapMin") = logGainmapMin;
        builder.uniform("logGainmapMax") = logGainmapMax;
        builder.uniform("gainmapGamma") = gainmapInfo.fGainmapGamma;
        builder.uniform("epsilonSdr") = gainmapInfo.fEpsilonSdr;
        builder.uniform("epsilonHdr") = gainmapInfo.fEpsilonHdr;
        builder.uniform("W") = W;
        gainmapMathShader = builder.makeShader();
        SkASSERT(gainmapMathShader);
    }

    // Return a shader that will apply the gainmap and then convert to the destination color space.
    return gainmapMathShader->makeWithColorFilter(colorXformGainmapToDst);
}
