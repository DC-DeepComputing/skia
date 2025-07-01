/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/precompile/PaintOptionsBuilder.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"

namespace PaintOptionsUtils {

using namespace skgpu::graphite;
using PrecompileShaders::GradientShaderFlags;
using PrecompileShaders::ImageShaderFlags;
using PrecompileShaders::YUVImageShaderFlags;

Builder& Builder::hwImg(ImgColorInfo ci, ImgTileModeOptions tmOptions) {
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

    static const SkTileMode kClampTM  = SkTileMode::kClamp;
    static const SkTileMode kRepeatTM = SkTileMode::kRepeat;

    SkSpan<const SkTileMode> tmSpan;
    switch (tmOptions) {
        case kNone:                               break;
        case kClamp:  tmSpan = { &kClampTM,  1 }; break;
        case kRepeat: tmSpan = { &kRepeatTM, 1 }; break;
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

    YUVImageShaderFlags flags = YUVImageShaderFlags::kNone;
    switch (options) {
        case kNoCubic:     flags = YUVImageShaderFlags::kExcludeCubic;           break;
        case kHWAndShader: flags = YUVImageShaderFlags::kNoCubicNoNonSwizzledHW; break;
    }

    sk_sp<PrecompileShader> img = PrecompileShaders::YUVImage(flags, { &kSRGBInfo, 1 });
    fPaintOptions.setShaders({ std::move(img) });
    return *this;
}

Builder& Builder::linearGrad(LinearGradientOptions options) {
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

Builder& Builder::blend() {
    static const SkColorInfo kCI { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &kCI, 1 },
                                                           {});
    SkBlendMode kBlendModes = SkBlendMode::kPlus;
    fPaintOptions.setShaders({ PrecompileShaders::Blend({ &kBlendModes, 1 },
                                                        { std::move(img) },
                                                        { PrecompileShaders::Color() }) });
    return *this;
}
namespace {

// Note: passing in a name to 'makeEffect' is a difference from Android's factory functions.
sk_sp<SkRuntimeEffect> makeEffect(const SkString& sksl, const char* name) {
    SkRuntimeEffect::Options options;
    options.fName = name;

    auto [effect, error] = SkRuntimeEffect::MakeForShader(sksl, options);
    if (!effect) {
        SkDebugf("%s\n", error.c_str());
    }
    return effect;
}

class EdgeExtension {
public:
    EdgeExtension() {
        // The following code block is just a stub for the Android code. For Skia's testing
        // purposes it only needs to have the same name and number of children as the real code.
        // When the following PaintOptions are used in Android the real SkSL must be supplied.
        static const SkString kEdgeExtensionCode(R"(
            uniform shader img;

            vec4 main(vec2 xy) {
                float3 sample = img.eval(0.115 * xy).rgb;
                return float4(sample, 1.0);
            }
        )");

        fEdgeExtensionEffect = makeEffect(kEdgeExtensionCode, "RE_EdgeExtensionEffect");
        SkASSERT(fEdgeExtensionEffect);
    }

    sk_sp<SkRuntimeEffect> edgeExtensionEffect() const { return fEdgeExtensionEffect; }

private:
    sk_sp<SkRuntimeEffect> fEdgeExtensionEffect;
};

const EdgeExtension& EdgeExtensionSingleton() {
    static class EdgeExtension sEdgeExtension;

    return sEdgeExtension;
}

} // anonymous namespace

Builder& Builder::edgeExt(bool passThrough) {
    static const SkColorInfo kPassthroughInfo { kRGBA_8888_SkColorType,
                                                kPremul_SkAlphaType,
                                                nullptr };
    // This usage of kUnpremul is non-obvious. It acts to short circuit the identity-colorspace
    // optimization for runtime effects. In this case, the Pipeline requires a
    // ColorSpaceTransformPremul instead of the (optimized) Passthrough.
    static const SkColorInfo kPremulInfo { kRGBA_8888_SkColorType,
                                           kUnpremul_SkAlphaType,
                                           nullptr };

    SkSpan<const SkColorInfo> ciSpan;
    if (passThrough) {
        ciSpan = { &kPassthroughInfo, 1 };
    } else {
        ciSpan = { &kPremulInfo, 1 };
    }
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           ciSpan,
                                                           {});

    sk_sp<PrecompileShader> edgeEffect = PrecompileRuntimeEffects::MakePrecompileShader(
            EdgeExtensionSingleton().edgeExtensionEffect(),
            { { std::move(img) } });

    fPaintOptions.setShaders({ std::move(edgeEffect) });
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

} // namespace PaintOptionsUtils
