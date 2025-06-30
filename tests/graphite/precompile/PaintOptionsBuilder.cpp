/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/precompile/PaintOptionsBuilder.h"

#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"

namespace PaintOptionUtils {

using namespace skgpu::graphite;
using PrecompileShaders::GradientShaderFlags;
using PrecompileShaders::ImageShaderFlags;
using PrecompileShaders::YUVImageShaderFlags;

Builder& Builder::hwImg(ImgColorInfo ci, ImgOptions options) {
    static const SkColorInfo kAlphaInfo(kAlpha_8_SkColorType,
                                        kUnpremul_SkAlphaType,
                                        nullptr);
    static const SkColorInfo kAlphaSRGBInfo(kAlpha_8_SkColorType,
                                            kUnpremul_SkAlphaType,
                                            SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                                  SkNamedGamut::kAdobeRGB));
    static const SkColorInfo kPremulInfo(kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    static const SkColorInfo kSRGBInfo(kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType,
                                       SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                             SkNamedGamut::kAdobeRGB));

    SkSpan<const SkColorInfo> ciSpan;
    switch (ci) {
        case kAlpha:     ciSpan = { &kAlphaInfo, 1 };    break;
        case kAlphaSRGB: ciSpan = { &kAlphaSRGBInfo, 1}; break;
        case kPremul:    ciSpan = { &kPremulInfo, 1 };   break;
        case kSRGB:      ciSpan = { &kSRGBInfo, 1 };     break;
    }

    static const SkTileMode kClampTM = SkTileMode::kClamp;
    static const SkTileMode kRepeatTM = SkTileMode::kRepeat;

    SkSpan<const SkTileMode> tmSpan;
    if (options == kClamp) {
        tmSpan = { &kClampTM, 1 };
    } else if (options == kRepeat) {
        tmSpan = { &kRepeatTM, 1 };
    }
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           ciSpan,
                                                           tmSpan);
    fPaintOptions.setShaders({ std::move(img) });
    return *this;
}

Builder& Builder::yuv(YUVSamplingOptions options) {
    static const SkColorInfo kSRGBInfo(kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType,
                                       SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                             SkNamedGamut::kAdobeRGB));

    sk_sp<PrecompileShader> img = PrecompileShaders::YUVImage(
            options == kNoCubic ? YUVImageShaderFlags::kExcludeCubic
                                : YUVImageShaderFlags::kNoCubicNoNonSwizzledHW,
            { &kSRGBInfo, 1 });
    fPaintOptions.setShaders({ std::move(img) });
    return *this;
}

Builder& Builder::linearGrad(GradientOptions options) {
    sk_sp<PrecompileShader> gradient;

    if (options == kSmall) {
        gradient = PrecompileShaders::LinearGradient(GradientShaderFlags::kSmall);
    } else if (options == kComplex) {
        gradient = PrecompileShaders::LinearGradient(
                GradientShaderFlags::kNoLarge,
                { SkGradientShader::Interpolation::InPremul::kNo,
                  SkGradientShader::Interpolation::ColorSpace::kSRGB,
                  SkGradientShader::Interpolation::HueMethod::kShorter });
    }

    fPaintOptions.setShaders({ std::move(gradient) });
    return *this;
}

Builder& Builder::matrixCF() {
    fPaintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });
    return *this;
}

Builder& Builder::porterDuffCF() {
    fPaintOptions.setColorFilters({ PrecompileColorFilters::Blend({ SkBlendMode::kSrcOver }) });
    return *this;
}

} // namespace PaintOptionUtils
