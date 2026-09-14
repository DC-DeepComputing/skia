/*
 * Copyright 2026 Samsung Research
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkFloatingPoint.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>
#include <vector>

// ============================================================================
// MipMapedBlur (LOD based blur)
//
// Principle (see LOD_BLUR_COMPLETE_GUIDE.md):
//   1. Repeatedly downscale the input by half to build a mipmap chain.
//      (A 3x3 tent kernel is applied while downscaling -> "Downscale + Blur")
//   2. Convert sigma to an LOD value:  lodValue = log2(1 + sigma)
//      (one LOD step = 1/2 resolution = roughly 2x blur radius)
//   3. Upscale the two adjacent LOD levels (lodLevel, lodLevel+1) to the original
//      size and mix (lerp) them by fraction to get a blur without visible steps.
//
// Like SkBlurImageFilter this derives from SkImageFilter_Base and runs on
// skif::FilterResult / Builder::eval (SkSL runtime shaders).
// ============================================================================

namespace {

// ----------------------------------------------------------------------------
// [1] LOD helpers (LodCalculator in the guide)
// ----------------------------------------------------------------------------

// Maximum LOD level: floor(log2(max(w,h))) - 2
// (-2 keeps at least a 4x4 image to avoid blur artifacts)
int calculate_max_lod(const SkISize& dims) {
    static constexpr int kLodLimit = 2;
    const int maxDimension = std::max(dims.width(), dims.height());
    if (maxDimension <= 0) {
        return 0;
    }
    const int maxLod = static_cast<int>(
            std::floor(std::log2(static_cast<float>(maxDimension)))) - kLodLimit;
    return std::max(0, maxLod);
}

// sigma -> lodValue.
// Every LOD step halves the resolution and roughly doubles the effective blur
// radius, so a log-scale mapping is natural. sigma=0 -> lod 0 (original).
float sigma_to_lod_value(float sigma) {
    return std::log2(1.f + sigma);
}

// Layer bounds scaled down to 1/2^level, rounded out and outset by a margin.
// Used to find the minimum area each upsample step has to cover. The margin must be
// >= 2 so that this level's output, once upscaled 2x, still covers the +-2px Kawase
// taps of the next level (margin m at level L covers 2m at L-1, which needs m+2).
SkIRect scaled_bounds(const SkIRect& base, int level) {
    static constexpr int kMargin = 3;
    const float inv = 1.f / static_cast<float>(1 << level);
    SkIRect r = SkIRect::MakeLTRB(
            static_cast<int>(std::floor(base.left()   * inv)),
            static_cast<int>(std::floor(base.top()    * inv)),
            static_cast<int>(std::ceil(base.right()   * inv)),
            static_cast<int>(std::ceil(base.bottom()  * inv)));
    r.outset(kMargin, kMargin);
    return r;
}

// ----------------------------------------------------------------------------
// [2] SkSL runtime shaders (fragment shaders)
// ----------------------------------------------------------------------------

// 3x3 tent filter shader (weights sum to 1.0). Shared by down- and upsampling.
//  - Downsample: averages the neighborhood while drawing at half resolution
//    -> smooth mip level ("DownscaleAndBlur" in the guide)
//  - Upsample: applied right after the 2x upscale to remove the bilinear grid
//    (same role as the upsample filter of the Dual Kawase blur)
static constexpr char kTentFilterSkSL[] = R"(
    uniform shader src;

    half4 main(float2 p) {
        half4 c = src.eval(p) * 0.25;
        c += (src.eval(p + float2( 1.0,  0.0)) +
              src.eval(p + float2(-1.0,  0.0)) +
              src.eval(p + float2( 0.0,  1.0)) +
              src.eval(p + float2( 0.0, -1.0))) * 0.125;
        c += (src.eval(p + float2( 1.0,  1.0)) +
              src.eval(p + float2(-1.0,  1.0)) +
              src.eval(p + float2( 1.0, -1.0)) +
              src.eval(p + float2(-1.0, -1.0))) * 0.0625;
        return c;
    }
)";

// Kawase 8-tap upsample shader (the canonical Dual Kawase upsample kernel).
// Applied right after a 2x bilinear upscale. 4 diagonal taps (weight 2) +
// 4 axis taps (weight 1), normalized by 12. Wider and smoother than the 3x3 tent,
// so it removes the grid better.
static constexpr char kKawaseUpsampleSkSL[] = R"(
    uniform shader src;

    half4 main(float2 p) {
        half4 sum = src.eval(p + float2(-2.0,  0.0));
        sum += src.eval(p + float2(-1.0,  1.0)) * 2.0;
        sum += src.eval(p + float2( 0.0,  2.0));
        sum += src.eval(p + float2( 1.0,  1.0)) * 2.0;
        sum += src.eval(p + float2( 2.0,  0.0));
        sum += src.eval(p + float2( 1.0, -1.0)) * 2.0;
        sum += src.eval(p + float2( 0.0, -2.0));
        sum += src.eval(p + float2(-1.0, -1.0)) * 2.0;
        return sum * (1.0 / 12.0);
    }
)";

// Gradient upsample shader (one step of the pyramid blend).
// Performs the Kawase 8-tap upsample and the "per-pixel level mix" in one pass:
//   w = clamp(lodValue(p) - level, 0, 1)
//   result = mix(mip of this level (sharp), upsampled blur signal (blurry), w)
// lodValue(p) comes from the gradient axis coordinate t run through a cubic-bezier falloff.
// With a constant intensity the result is mathematically identical to the uniform blur.
static constexpr char kGradientUpsampleSkSL[] = R"(
    uniform shader blurred;       // result of the level above, U_{k+1}, upscaled 2x
    uniform shader sharp;         // mip of this level, M_k
    uniform half   uLevel;        // level k we are arriving at
    uniform half   uMaxLodValue;  // log2(1 + sigma): lod value of the full-blur region
    uniform float2 uGradDir;      // gradient direction (unit vector)
    uniform float  uGradMin;      // minimum axis projection (this level's coordinates)
    uniform float  uGradInvRange; // 1 / axis projection range (this level's coordinates)
    uniform half   uPercent;      // end of the blur region (0~1)
    uniform half   uPivotAbs;     // end of the full-blur region = percent * pivotPercent
    uniform half   uInvFalloff;   // 1 / (percent - pivotAbs), 0 if the width is 0

    // Fixed cubic-bezier easing. Pinning the x control points to (1/3, 2/3) gives
    // x(u) = u, so y can be evaluated directly without a Newton inverse. The curve
    // shape is tuned by the y control point constants (default (0, 1) = smooth ease-in-out).
    const half kBezierY1 = 0.0;
    const half kBezierY2 = 1.0;

    half bezier01(half s) {
        half inv = 1.0 - s;
        return 3.0 * inv * inv * s * kBezierY1
             + 3.0 * inv * s   * s * kBezierY2
             + s * s * s;
    }

    half4 main(float2 p) {
        // 1) position -> gradient axis coordinate t (0~1)
        float t = clamp((dot(p, uGradDir) - uGradMin) * uGradInvRange, 0.0, 1.0);

        // 2) t -> blur intensity (full blur -> bezier falloff -> 0)
        half intensity;
        if (t >= float(uPercent)) {
            intensity = 0.0;
        } else if (t <= float(uPivotAbs) || uInvFalloff <= 0.0) {
            intensity = 1.0;
        } else {
            half s = half((t - float(uPivotAbs)) * float(uInvFalloff));
            intensity = 1.0 - bezier01(clamp(s, 0.0, 1.0));
        }

        // 3) intensity -> per-pixel lod value -> mix weight for this level
        half w = clamp(uMaxLodValue * intensity - uLevel, 0.0, 1.0);
        if (w <= 0.0) {
            return sharp.eval(p); // this pixel does not need to be blurrier than this level
        }

        // 4) Kawase 8-tap upsample (sample the 2x upscaled blurred while smoothing it)
        half4 blur = blurred.eval(p + float2(-2.0,  0.0));
        blur += blurred.eval(p + float2(-1.0,  1.0)) * 2.0;
        blur += blurred.eval(p + float2( 0.0,  2.0));
        blur += blurred.eval(p + float2( 1.0,  1.0)) * 2.0;
        blur += blurred.eval(p + float2( 2.0,  0.0));
        blur += blurred.eval(p + float2( 1.0, -1.0)) * 2.0;
        blur += blurred.eval(p + float2( 0.0, -2.0));
        blur += blurred.eval(p + float2(-1.0, -1.0)) * 2.0;
        blur *= (1.0 / 12.0);

        return mix(sharp.eval(p), blur, w);
    }
)";

// Circular gradient upsample shader. Same pyramid blend step as
// kGradientUpsampleSkSL, but uses the normalized distance d from the center
// instead of the gradient axis coordinate t:
//   d = clamp(|p - center| / radius, 0, 1)   (center 0 -> corner 1)
//   d <= pivotAbs : full blur, pivotAbs~percent : bezier falloff, d >= percent : sharp
static constexpr char kCircleUpsampleSkSL[] = R"(
    uniform shader blurred;       // result of the level above, U_{k+1}, upscaled 2x
    uniform shader sharp;         // mip of this level, M_k
    uniform half   uLevel;        // level k we are arriving at
    uniform half   uMaxLodValue;  // log2(1 + sigma): lod value at the center (full blur)
    uniform float2 uCenter;       // circle center (this level's coordinates)
    uniform float  uInvRadius;    // 1 / normalization radius (this level's coordinates)
    uniform half   uPercent;      // normalized radius of the blurred circle (0~1)
    uniform half   uPivotAbs;     // radius of the full-blur circle = percent * pivotPercent
    uniform half   uInvFalloff;   // 1 / (percent - pivotAbs), 0 if the width is 0

    const half kBezierY1 = 0.0;
    const half kBezierY2 = 1.0;

    half bezier01(half s) {
        half inv = 1.0 - s;
        return 3.0 * inv * inv * s * kBezierY1
             + 3.0 * inv * s   * s * kBezierY2
             + s * s * s;
    }

    half4 main(float2 p) {
        // 1) position -> normalized distance d from the center (0~1)
        float d = clamp(length(p - uCenter) * uInvRadius, 0.0, 1.0);

        // 2) d -> blur intensity (full blur -> bezier falloff -> 0)
        half intensity;
        if (d >= float(uPercent)) {
            intensity = 0.0;
        } else if (d <= float(uPivotAbs) || uInvFalloff <= 0.0) {
            intensity = 1.0;
        } else {
            half s = half((d - float(uPivotAbs)) * float(uInvFalloff));
            intensity = 1.0 - bezier01(clamp(s, 0.0, 1.0));
        }

        // 3) intensity -> per-pixel lod value -> mix weight for this level
        half w = clamp(uMaxLodValue * intensity - uLevel, 0.0, 1.0);
        if (w <= 0.0) {
            return sharp.eval(p);
        }

        // 4) Kawase 8-tap upsample
        half4 blur = blurred.eval(p + float2(-2.0,  0.0));
        blur += blurred.eval(p + float2(-1.0,  1.0)) * 2.0;
        blur += blurred.eval(p + float2( 0.0,  2.0));
        blur += blurred.eval(p + float2( 1.0,  1.0)) * 2.0;
        blur += blurred.eval(p + float2( 2.0,  0.0));
        blur += blurred.eval(p + float2( 1.0, -1.0)) * 2.0;
        blur += blurred.eval(p + float2( 0.0, -2.0));
        blur += blurred.eval(p + float2(-1.0, -1.0)) * 2.0;
        blur *= (1.0 / 12.0);

        return mix(sharp.eval(p), blur, w);
    }
)";

// LOD mix shader: mixes two adjacent LODs, upscaled to the original size, by fraction.
// (MipmapSampler::InterpolateColors in the guide. Takes two child shaders directly
//  instead of a packed texture, so no coordinate remapping is needed.)
static constexpr char kLodMixSkSL[] = R"(
    uniform shader lodA;      // lower LOD (sharper)
    uniform shader lodB;      // higher LOD (blurrier)
    uniform half   fraction;  // 0.0 ~ 1.0 interpolation factor

    half4 main(float2 p) {
        return mix(lodA.eval(p), lodB.eval(p), fraction);
    }
)";

const SkRuntimeEffect* tent_filter_effect() {
    static const sk_sp<SkRuntimeEffect> effect =
            SkRuntimeEffect::MakeForShader(SkString(kTentFilterSkSL)).effect;
    return effect.get();
}

const SkRuntimeEffect* kawase_upsample_effect() {
    static const sk_sp<SkRuntimeEffect> effect =
            SkRuntimeEffect::MakeForShader(SkString(kKawaseUpsampleSkSL)).effect;
    return effect.get();
}

const SkRuntimeEffect* gradient_upsample_effect() {
    static const sk_sp<SkRuntimeEffect> effect =
            SkRuntimeEffect::MakeForShader(SkString(kGradientUpsampleSkSL)).effect;
    return effect.get();
}

const SkRuntimeEffect* circle_upsample_effect() {
    static const sk_sp<SkRuntimeEffect> effect =
            SkRuntimeEffect::MakeForShader(SkString(kCircleUpsampleSkSL)).effect;
    return effect.get();
}

const SkRuntimeEffect* lod_mix_effect() {
    static const sk_sp<SkRuntimeEffect> effect =
            SkRuntimeEffect::MakeForShader(SkString(kLodMixSkSL)).effect;
    return effect.get();
}

// ----------------------------------------------------------------------------
// [3] Filter class (same skeleton as SkBlurImageFilter)
// ----------------------------------------------------------------------------

class SkMipMapedBlurImageFilter final : public SkImageFilter_Base {
public:
    SkMipMapedBlurImageFilter(SkSize sigma, SkTileMode legacyTileMode, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1)
            , fSigma{sigma}
            , fLegacyTileMode(legacyTileMode) {}

    // Gradient (variable radius) version
    SkMipMapedBlurImageFilter(SkSize sigma, SkTileMode legacyTileMode,
                              SkScalar percent, SkScalar pivotPercent, SkScalar directionDeg,
                              sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1)
            , fSigma{sigma}
            , fLegacyTileMode(legacyTileMode)
            , fGradient(true)
            , fPercent(percent)
            , fPivotPercent(pivotPercent)
            , fDirectionDeg(directionDeg) {}

    // Circular version (full blur at the center -> sharp outside). percent/pivotPercent
    // mean the same as for the gradient but apply to the normalized distance from the center.
    struct CircleTag {};
    SkMipMapedBlurImageFilter(CircleTag, SkSize sigma, SkTileMode legacyTileMode,
                              SkScalar percent, SkScalar pivotPercent,
                              sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1)
            , fSigma{sigma}
            , fLegacyTileMode(legacyTileMode)
            , fCircle(true)
            , fPercent(percent)
            , fPivotPercent(pivotPercent) {}

    SkRect computeFastBounds(const SkRect&) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterMipMapedBlurImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkMipMapedBlurImageFilter)

    skif::FilterResult onFilterImage(const skif::Context& context) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    // Convert sigma from parameter space to layer space (accounts for CTM scale)
    skif::LayerSpace<SkSize> mapSigma(const skif::Mapping& mapping) const;

    // Bounds outset by the area the blur can bleed into (3*sigma).
    // Safe upper bound covering the LOD upscale leak (~2^(lod+1) ~= 2(1+sigma)).
    skif::LayerSpace<SkIRect> kernelBounds(const skif::Mapping& mapping,
                                           skif::LayerSpace<SkIRect> bounds) const {
        skif::LayerSpace<SkSize> sigma = this->mapSigma(mapping);
        bounds.outset(skif::LayerSpace<SkSize>({3 * sigma.width(), 3 * sigma.height()}).ceil());
        return bounds;
    }

    // Downscale the input to half resolution while applying a 3x3 tent blur (one mip step)
    static skif::FilterResult DownsampleHalf(const skif::Context& ctx,
                                             const skif::FilterResult& source);

    // Upscale the input 2x and remove the grid with the Kawase 8-tap (one upsample step).
    // clampBounds: maximum area that needs rendering at this step (avoids excess work)
    static skif::FilterResult UpsampleDouble(const skif::Context& ctx,
                                             const skif::FilterResult& source,
                                             const skif::LayerSpace<SkIRect>& clampBounds);

    // Spatial info for the variable blur (full-res).
    //  - Linear gradient: range of the content bounds projected onto the direction vector
    //  - Circle: content center and center-to-corner distance (half diagonal)
    struct GradientFrame {
        SkPoint dir;    // direction unit vector (0 deg = +y, 90 deg = +x)
        float   tMin;   // minimum projection
        float   tRange; // projection range (> 0)
        SkPoint center; // circle center
        float   radius; // circle radius (> 0)
    };

    GradientFrame makeGradientFrame(const skif::LayerSpace<SkIRect>& contentBounds) const;

    // Whole variable-radius path (pyramid blend)
    skif::FilterResult onGradientFilterImage(const skif::Context& ctx,
                                             skif::FilterResult childOutput,
                                             float maxSigma) const;

    // One pyramid blend step: Kawase upsample + per-pixel level mix
    skif::FilterResult gradientUpsampleStep(const skif::Context& ctx,
                                            const skif::FilterResult& blurred,
                                            const skif::FilterResult& sharp,
                                            int level, float maxLodValue,
                                            const GradientFrame& frame,
                                            const skif::LayerSpace<SkIRect>& clampBounds) const;

    skif::ParameterSpace<SkSize> fSigma;
    // Same as SkBlurImageFilter: a tileMode given without cropRect is handled here
    SkTileMode fLegacyTileMode = SkTileMode::kDecal;

    // Circular blur: full blur at the center, 0 at radius fPercent. Shares fPercent/fPivotPercent
    bool     fCircle = false;

    // Variable (gradient/circle) blur parameters, used only when fGradient or fCircle is set
    bool     fGradient = false;
    SkScalar fPercent = 1.f;       // blur region (0~1 along the gradient axis / normalized distance)
    SkScalar fPivotPercent = 1.f;  // fraction of the blur region that keeps full blur (0~1)
    SkScalar fDirectionDeg = 0.f;  // 0 deg = top->bottom (y), 90 deg = left->right (x)
};

} // end namespace

// dlgmlals3
sk_sp<SkImageFilter> SkImageFilters::MipMapedBlur(
        SkScalar sigmaX, SkScalar sigmaY, SkTileMode tileMode, sk_sp<SkImageFilter> input,
        const CropRect& cropRect) {

    if (!SkIsFinite(sigmaX, sigmaY) || sigmaX < 0.f || sigmaY < 0.f) {
        return nullptr;
    }

    // Tiling requested without a cropRect -> handled inside the filter (legacy path)
    if (tileMode != SkTileMode::kDecal && !cropRect) {
        return sk_make_sp<SkMipMapedBlurImageFilter>(SkSize{sigmaX, sigmaY}, tileMode, std::move(input));
    }

    // With a cropRect wrap in Crop filters exactly like Blur()
    sk_sp<SkImageFilter> filter = std::move(input);
    if (tileMode != SkTileMode::kDecal && cropRect) {
        filter = SkImageFilters::Crop(*cropRect, tileMode, std::move(filter));
    }

    filter = sk_make_sp<SkMipMapedBlurImageFilter>(SkSize{sigmaX, sigmaY}, SkTileMode::kDecal, std::move(filter));
    if (cropRect) {
        filter = SkImageFilters::Crop(*cropRect, SkTileMode::kDecal, std::move(filter));
    }
    return filter;
}

sk_sp<SkImageFilter> SkImageFilters::MipMapedGradientBlur(
        SkScalar sigmaX, SkScalar sigmaY,
        SkScalar percent, SkScalar pivotPercent, SkScalar directionDeg,
        SkTileMode tileMode, sk_sp<SkImageFilter> input, const CropRect& cropRect) {

    if (!SkIsFinite(sigmaX, sigmaY, percent, pivotPercent, directionDeg) ||
        sigmaX < 0.f || sigmaY < 0.f) {
        return nullptr;
    }
    percent = std::clamp(percent, 0.f, 1.f);
    pivotPercent = std::clamp(pivotPercent, 0.f, 1.f);

    if (tileMode != SkTileMode::kDecal && !cropRect) {
        return sk_make_sp<SkMipMapedBlurImageFilter>(SkSize{sigmaX, sigmaY}, tileMode,
                                                     percent, pivotPercent, directionDeg,
                                                     std::move(input));
    }

    sk_sp<SkImageFilter> filter = std::move(input);
    if (tileMode != SkTileMode::kDecal && cropRect) {
        filter = SkImageFilters::Crop(*cropRect, tileMode, std::move(filter));
    }

    filter = sk_make_sp<SkMipMapedBlurImageFilter>(SkSize{sigmaX, sigmaY}, SkTileMode::kDecal,
                                                   percent, pivotPercent, directionDeg,
                                                   std::move(filter));
    if (cropRect) {
        filter = SkImageFilters::Crop(*cropRect, SkTileMode::kDecal, std::move(filter));
    }
    return filter;
}

sk_sp<SkImageFilter> SkImageFilters::MipMapedCircleBlur(
        SkScalar sigmaX, SkScalar sigmaY, SkScalar percent, SkScalar pivotPercent,
        SkTileMode tileMode, sk_sp<SkImageFilter> input, const CropRect& cropRect) {

    if (!SkIsFinite(sigmaX, sigmaY, percent, pivotPercent) || sigmaX < 0.f || sigmaY < 0.f) {
        return nullptr;
    }
    percent = std::clamp(percent, 0.f, 1.f);
    pivotPercent = std::clamp(pivotPercent, 0.f, 1.f);

    using CircleTag = SkMipMapedBlurImageFilter::CircleTag;
    if (tileMode != SkTileMode::kDecal && !cropRect) {
        return sk_make_sp<SkMipMapedBlurImageFilter>(CircleTag{}, SkSize{sigmaX, sigmaY},
                                                     tileMode, percent, pivotPercent,
                                                     std::move(input));
    }

    sk_sp<SkImageFilter> filter = std::move(input);
    if (tileMode != SkTileMode::kDecal && cropRect) {
        filter = SkImageFilters::Crop(*cropRect, tileMode, std::move(filter));
    }

    filter = sk_make_sp<SkMipMapedBlurImageFilter>(CircleTag{}, SkSize{sigmaX, sigmaY},
                                                   SkTileMode::kDecal, percent, pivotPercent,
                                                   std::move(filter));
    if (cropRect) {
        filter = SkImageFilters::Crop(*cropRect, SkTileMode::kDecal, std::move(filter));
    }
    return filter;
}

// ----------------------------------------------------------------------------
// [5] Serialization (SKP support)
// ----------------------------------------------------------------------------

void SkRegisterMipMapedBlurImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkMipMapedBlurImageFilter);
}

sk_sp<SkFlattenable> SkMipMapedBlurImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar sigmaX = buffer.readScalar();
    SkScalar sigmaY = buffer.readScalar();
    SkTileMode tileMode = buffer.read32LE(SkTileMode::kLastTileMode);
    const bool gradient = buffer.readBool();
    if (gradient) {
        SkScalar percent = buffer.readScalar();
        SkScalar pivotPercent = buffer.readScalar();
        SkScalar directionDeg = buffer.readScalar();
        return SkImageFilters::MipMapedGradientBlur(
                sigmaX, sigmaY, percent, pivotPercent, directionDeg, tileMode,
                common.getInput(0), common.cropRect());
    }
    const bool circle = buffer.readBool();
    if (circle) {
        SkScalar percent = buffer.readScalar();
        SkScalar pivotPercent = buffer.readScalar();
        return SkImageFilters::MipMapedCircleBlur(
                sigmaX, sigmaY, percent, pivotPercent, tileMode,
                common.getInput(0), common.cropRect());
    }
    return SkImageFilters::MipMapedBlur(
            sigmaX, sigmaY, tileMode, common.getInput(0), common.cropRect());
}

void SkMipMapedBlurImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeScalar(SkSize(fSigma).fWidth);
    buffer.writeScalar(SkSize(fSigma).fHeight);
    buffer.writeInt(static_cast<int>(fLegacyTileMode));
    buffer.writeBool(fGradient);
    if (fGradient) {
        buffer.writeScalar(fPercent);
        buffer.writeScalar(fPivotPercent);
        buffer.writeScalar(fDirectionDeg);
    }
    buffer.writeBool(fCircle);
    if (fCircle) {
        buffer.writeScalar(fPercent);
        buffer.writeScalar(fPivotPercent);
    }
}

// ----------------------------------------------------------------------------
// [6] One mip step: downscale by half + 3x3 tent blur
// ----------------------------------------------------------------------------

skif::FilterResult SkMipMapedBlurImageFilter::DownsampleHalf(const skif::Context& ctx,
                                                             const skif::FilterResult& source) {
    // Target area: current content bounds scaled down by half
    const SkIRect srcBounds = SkIRect(source.layerBounds());
    SkIRect halfRect = SkIRect::MakeLTRB(
            static_cast<int>(std::floor(srcBounds.left()   * 0.5f)),
            static_cast<int>(std::floor(srcBounds.top()    * 0.5f)),
            static_cast<int>(std::ceil(srcBounds.right()   * 0.5f)),
            static_cast<int>(std::ceil(srcBounds.bottom()  * 0.5f)));
    if (halfRect.isEmpty()) {
        // Cannot shrink further (around 1px), return as is
        return source;
    }
    // The tent filter bleeds content 1px outward, so leave a margin
    halfRect.outset(1, 1);

    skif::LayerSpace<SkIRect> halfBounds{halfRect};
    // Restrict the context to the half-size area -> eval() renders to an actual
    // half-resolution surface, so the resolution physically drops.
    skif::Context stepCtx = ctx.withNewDesiredOutput(halfBounds);

    // Apply a 0.5x downscale transform to the content (bilinear sampling)
    skif::FilterResult halfScaled = source.applyTransform(
            stepCtx,
            skif::LayerSpace<SkMatrix>(SkMatrix::Scale(0.5f, 0.5f)),
            SkSamplingOptions(SkFilterMode::kLinear));

    // The tent kernel samples +-1px outside, so outset sampleBounds by 1px
    skif::LayerSpace<SkIRect> sampleBounds = halfBounds;
    sampleBounds.outset(skif::LayerSpace<SkISize>({1, 1}));

    using ShaderFlags = skif::FilterResult::ShaderFlags;
    skif::FilterResult::Builder builder{stepCtx};
    builder.add(halfScaled, sampleBounds,
                ShaderFlags::kSampledRepeatedly | ShaderFlags::kNonTrivialSampling);
    return builder.eval([&](SkSpan<sk_sp<SkShader>> inputs) -> sk_sp<SkShader> {
        if (!inputs[0]) {
            return nullptr; // fully transparent input -> transparent output
        }
        const SkRuntimeEffect* effect = tent_filter_effect();
        if (!effect) {
            return inputs[0]; // shader compile failed, fall back to the bilinear downscale
        }
        SkRuntimeShaderBuilder shaderBuilder(sk_ref_sp(effect));
        shaderBuilder.child("src") = inputs[0];
        return shaderBuilder.makeShader();
    });
}

// ----------------------------------------------------------------------------
// [6-2] One upsample step: 2x upscale + 3x3 tent filter
//
// Upscaling a low-res LOD to the original size in one bilinear step leaves a
// visible grid/diamond pattern where the gradient kinks at every low-res texel
// boundary (2^level px apart). Upscaling only 2x at a time and smoothing with a
// tent filter at each step, like Dual Kawase, erases the grid step by step and
// brings the final quality close to a Gaussian.
// ----------------------------------------------------------------------------

skif::FilterResult SkMipMapedBlurImageFilter::UpsampleDouble(
        const skif::Context& ctx,
        const skif::FilterResult& source,
        const skif::LayerSpace<SkIRect>& clampBounds) {
    // Target area: current content bounds scaled up 2x
    const SkIRect srcBounds = SkIRect(source.layerBounds());
    SkIRect doubledRect = SkIRect::MakeLTRB(srcBounds.left()   * 2,
                                            srcBounds.top()    * 2,
                                            srcBounds.right()  * 2,
                                            srcBounds.bottom() * 2);
    // The Kawase kernel bleeds content 2px outward, so leave a margin, and clip
    // anything outside the area actually needed (clampBounds) to save work
    doubledRect.outset(2, 2);
    if (!doubledRect.intersect(SkIRect(clampBounds))) {
        return {}; // does not overlap the needed area
    }

    skif::LayerSpace<SkIRect> targetBounds{doubledRect};
    skif::Context stepCtx = ctx.withNewDesiredOutput(targetBounds);

    // The Kawase kernel samples +-2px outside the output, so the upscaled input has to
    // stay valid over sampleBounds, not just targetBounds. applyTransform() crops its
    // result to the context's desired output, so it runs under a widened context;
    // otherwise the taps read transparent black past the clip edge and leave a dark
    // fringe that follows the visible region (visible as a moving band when scrolling).
    skif::LayerSpace<SkIRect> sampleBounds = targetBounds;
    sampleBounds.outset(skif::LayerSpace<SkISize>({2, 2}));

    // Upscale the content 2x (bilinear) - the grid is still present at this point
    skif::FilterResult upscaled = source.applyTransform(
            ctx.withNewDesiredOutput(sampleBounds),
            skif::LayerSpace<SkMatrix>(SkMatrix::Scale(2.f, 2.f)),
            SkSamplingOptions(SkFilterMode::kLinear));

    // kNonTrivialSampling prevents the linear->nearest downgrade (cause of blocky edges).

    using ShaderFlags = skif::FilterResult::ShaderFlags;
    skif::FilterResult::Builder builder{stepCtx};
    builder.add(upscaled, sampleBounds,
                ShaderFlags::kSampledRepeatedly | ShaderFlags::kNonTrivialSampling,
                SkSamplingOptions(SkFilterMode::kLinear));
    return builder.eval([&](SkSpan<sk_sp<SkShader>> inputs) -> sk_sp<SkShader> {
        if (!inputs[0]) {
            return nullptr;
        }
        const SkRuntimeEffect* effect = kawase_upsample_effect();
        if (!effect) {
            return inputs[0]; // shader compile failed, fall back to the bilinear upscale
        }
        SkRuntimeShaderBuilder shaderBuilder(sk_ref_sp(effect));
        shaderBuilder.child("src") = inputs[0];
        return shaderBuilder.makeShader();
    });
}

// ----------------------------------------------------------------------------
// [7] Main filtering: build the mip chain -> mix two adjacent LODs
// ----------------------------------------------------------------------------

skif::FilterResult SkMipMapedBlurImageFilter::onFilterImage(const skif::Context& ctx) const {
    // 7-1. Convert sigma to layer space and pick the representative (max) value (LOD is isotropic)
    skif::LayerSpace<SkSize> sigma = this->mapSigma(ctx.mapping());
    const float maxSigma = std::max(sigma.width(), sigma.height());


    // 7-2. Fetch the child (input) image. Request a wider area to account for blur bleed.
    skif::Context inputCtx = ctx.withNewDesiredOutput(
            this->kernelBounds(ctx.mapping(), ctx.desiredOutput()));
    skif::FilterResult childOutput = this->getChildOutput(0, inputCtx);
    if (!childOutput) {
        return {};
    }


    if (maxSigma < 0.5f) {
        // Effectively no blur -> return the input as is
        return childOutput;
    }

    // 7-3. Legacy tile mode handling (same as SkBlurImageFilter)
    if (fLegacyTileMode != SkTileMode::kDecal) {
        childOutput = childOutput.applyCrop(inputCtx, childOutput.layerBounds(),
                                            fLegacyTileMode);
    }

    // Variable (gradient/circle) radius -> branch to the pyramid blend path, section [7-2]
    if (fGradient || fCircle) {
        return this->onGradientFilterImage(ctx, std::move(childOutput), maxSigma);
    }

    // 7-4. LOD parameters
    //   maxLod   : maximum level the content size allows
    //   lodValue : continuous LOD value derived from sigma (log2 scale)
    //   lodLevel : integer level, fraction : interpolation factor between adjacent levels
    const SkISize contentSize = SkIRect(childOutput.layerBounds()).size();
    const int maxLod = calculate_max_lod(contentSize);
    if (maxLod < 1) {
        return childOutput;
    }

    const float lodValue = std::min(sigma_to_lod_value(maxSigma), static_cast<float>(maxLod));
    const int   lodLevel = std::min(static_cast<int>(lodValue), maxLod - 1);
    const float fraction = lodValue - static_cast<float>(lodLevel);


    // 7-5. Build the mip chain: downscale to lodLevel, then one more step for lodB
    skif::FilterResult lodA = childOutput;
    for (int i = 0; i < lodLevel; ++i) {
        lodA = DownsampleHalf(ctx, lodA);
    }
    skif::FilterResult lodB = DownsampleHalf(ctx, lodA);

    // 7-6. Upsample lodB by one step only, to the same resolution as lodA.
    //      Upsampling and mix are both linear, so their order does not matter.
    //      -> Mixing at low resolution first and running the upsample chain once
    //        costs half as much as upscaling both LODs to full size, same quality.
    const SkIRect desiredRect = SkIRect(ctx.desiredOutput());

    skif::LayerSpace<SkIRect> mixBounds = lodA.layerBounds();
    if (!mixBounds.intersect(
                skif::LayerSpace<SkIRect>(scaled_bounds(desiredRect, lodLevel)))) {
        return {};
    }

    skif::FilterResult lodBUp = UpsampleDouble(ctx, lodB, mixBounds);

    // 7-7. Mix the two adjacent LODs by fraction at lodA resolution (SkSL fragment shader)
    skif::Context mixCtx = ctx.withNewDesiredOutput(mixBounds);
    skif::FilterResult::Builder builder{mixCtx};
    builder.add(lodA);
    builder.add(lodBUp);
    skif::FilterResult result = builder.eval([&](SkSpan<sk_sp<SkShader>> inputs)
                                                     -> sk_sp<SkShader> {
        const SkRuntimeEffect* effect = lod_mix_effect();
        if (!effect) {
            // mix shader compile failed -> fall back to the blurrier one (lodB)
            return inputs[1] ? inputs[1] : inputs[0];
        }
        // Replace transparent (null) inputs with a transparent shader so mix works
        sk_sp<SkShader> shaderA = inputs[0] ? inputs[0]
                                            : SkShaders::Color(SK_ColorTRANSPARENT);
        sk_sp<SkShader> shaderB = inputs[1] ? inputs[1]
                                            : SkShaders::Color(SK_ColorTRANSPARENT);
        SkRuntimeShaderBuilder shaderBuilder(sk_ref_sp(effect));
        shaderBuilder.child("lodA") = std::move(shaderA);
        shaderBuilder.child("lodB") = std::move(shaderB);
        shaderBuilder.uniform("fraction") = fraction;
        return shaderBuilder.makeShader();
    });

    // 7-8. Upsample chain: 2x at a time + tent filter at each step (Dual Kawase style).
    //      Erases the grid pattern a single 2^level bilinear upscale would produce.
    for (int level = lodLevel; level >= 1; --level) {
        result = UpsampleDouble(
                ctx, result,
                skif::LayerSpace<SkIRect>(scaled_bounds(desiredRect, level - 1)));
        if (!result) {
            return {};
        }
    }

    return result;
}

// ----------------------------------------------------------------------------
// [7-2] Gradient (variable radius) filtering: pyramid blend
//
// Idea: at each upsample step k, mix "the mip of this level (sharp)" with
//     w_k(p) = clamp(lodValue(p) - k, 0, 1)
//     lodValue(p) = log2(1 + sigma) * intensity(p)
// "the blur signal upsampled from the level above (blurry)" using the per-pixel weight
//
// e.g. for a pixel with lodValue = 2.4:
//     level >= 3 : w=0   -> mip passes through unchanged
//     level 2    : w=0.4 -> interpolate M2 and M3 (= LOD 2.4)
//     level 1, 0 : w=1   -> Kawase upsample only (upscale while removing the grid)
//
// With a constant intensity this equals the uniform blur. With a per-pixel intensity
// each position picks its own LOD. Variable radius without atlas packing or dynamic
// child indexing, and the Kawase upsample quality (grid removal) holds everywhere.
// ----------------------------------------------------------------------------

SkMipMapedBlurImageFilter::GradientFrame SkMipMapedBlurImageFilter::makeGradientFrame(
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    const SkRect cb = SkRect::Make(SkIRect(contentBounds));

    if (fCircle) {
        // Use the center-to-corner distance as the radius so the whole content lies
        // inside the falloff range (intensity is exactly 0 at the corners).
        const float halfDiag = 0.5f * std::sqrt(cb.width() * cb.width() +
                                                cb.height() * cb.height());
        return {{0.f, 1.f}, 0.f, 1.f, cb.center(), std::max(halfDiag, 1.f)};
    }

    // 0 deg = top->bottom (+y), 90 deg = left->right (+x)
    const float rad = SkDegreesToRadians(fDirectionDeg);
    const SkPoint dir = {std::sin(rad), std::cos(rad)};

    // Project the four corners of the content rect onto the direction vector to get the t range
    const float t00 = dir.fX * cb.left()  + dir.fY * cb.top();
    const float t10 = dir.fX * cb.right() + dir.fY * cb.top();
    const float t01 = dir.fX * cb.left()  + dir.fY * cb.bottom();
    const float t11 = dir.fX * cb.right() + dir.fY * cb.bottom();
    const float tMin = std::min(std::min(t00, t10), std::min(t01, t11));
    const float tMax = std::max(std::max(t00, t10), std::max(t01, t11));
    return {dir, tMin, std::max(tMax - tMin, 1.f), cb.center(), 1.f};
}

skif::FilterResult SkMipMapedBlurImageFilter::onGradientFilterImage(
        const skif::Context& ctx, skif::FilterResult childOutput, float maxSigma) const {
    const float percent = std::clamp(static_cast<float>(fPercent), 0.f, 1.f);
    if (percent <= 0.f) {
        return childOutput;
    }

    // The gradient axis / circle center are based on the whole source layer. childOutput
    // only covers desiredOutput (the current clip, e.g. the visible part inside a
    // ScrollView); using its bounds would shift the axis range on every scroll and make
    // the mask slide over the image.
    skif::LayerSpace<SkIRect> contentBounds = ctx.source().layerBounds();
    if (SkIRect(contentBounds).isEmpty()) {
        contentBounds = childOutput.layerBounds();
    }
    const SkISize contentSize = SkIRect(contentBounds).size();
    const int maxLod = calculate_max_lod(contentSize);
    if (maxLod < 1) {
        return childOutput;
    }

    // lod value the full-blur region has to reach, and the chain depth needed for it
    const float maxLodValue = std::min(sigma_to_lod_value(maxSigma),
                                       static_cast<float>(maxLod));
    const int chainDepth = std::max(1, std::min(static_cast<int>(std::ceil(maxLodValue)),
                                                maxLod));

    const GradientFrame frame = this->makeGradientFrame(contentBounds);

    if (fCircle) {
    } else {
    }

    // Build the mip chain - the pyramid blend uses every level, so keep them all
    std::vector<skif::FilterResult> mips;
    mips.reserve(chainDepth + 1);
    mips.push_back(std::move(childOutput));
    for (int k = 1; k <= chainDepth; ++k) {
        mips.push_back(DownsampleHalf(ctx, mips.back()));
    }

    // Pyramid blend: walk from the top (blurriest level) down to level 0
    const SkIRect desiredRect = SkIRect(ctx.desiredOutput());
    skif::FilterResult result = mips[chainDepth];
    for (int level = chainDepth - 1; level >= 0; --level) {
        result = this->gradientUpsampleStep(
                ctx, result, mips[level], level, maxLodValue, frame,
                skif::LayerSpace<SkIRect>(scaled_bounds(desiredRect, level)));
        if (!result) {
            return {};
        }
    }

    return result;
}

skif::FilterResult SkMipMapedBlurImageFilter::gradientUpsampleStep(
        const skif::Context& ctx,
        const skif::FilterResult& blurred,
        const skif::FilterResult& sharp,
        int level, float maxLodValue,
        const GradientFrame& frame,
        const skif::LayerSpace<SkIRect>& clampBounds) const {
    // Target area: blur signal scaled up 2x, joined with this level's mip bounds
    const SkIRect srcBounds = SkIRect(blurred.layerBounds());
    SkIRect targetRect = SkIRect::MakeLTRB(srcBounds.left()   * 2,
                                           srcBounds.top()    * 2,
                                           srcBounds.right()  * 2,
                                           srcBounds.bottom() * 2);
    targetRect.outset(2, 2); // 2px bleed margin for the Kawase kernel
    targetRect.join(SkIRect(sharp.layerBounds()));
    if (!targetRect.intersect(SkIRect(clampBounds))) {
        return {}; // does not overlap the needed area
    }

    skif::LayerSpace<SkIRect> targetBounds{targetRect};
    skif::Context stepCtx = ctx.withNewDesiredOutput(targetBounds);

    // Same as UpsampleDouble: keep the upscaled blur signal valid over the +-2px Kawase
    // taps by transforming under a context widened to sampleBounds.
    skif::LayerSpace<SkIRect> sampleBounds = targetBounds;
    sampleBounds.outset(skif::LayerSpace<SkISize>({2, 2}));

    // Upscale the blur signal 2x (bilinear) - the Kawase taps in the shader smooth the grid
    skif::FilterResult upscaled = blurred.applyTransform(
            ctx.withNewDesiredOutput(sampleBounds),
            skif::LayerSpace<SkMatrix>(SkMatrix::Scale(2.f, 2.f)),
            SkSamplingOptions(SkFilterMode::kLinear));

    // Convert the gradient axis to this level's coordinates (1/2^level scale):
    // full-res position = p * 2^level, so pre-scale min/range
    const float levelScale = static_cast<float>(1 << level);
    const float gradMin = frame.tMin / levelScale;
    const float gradInvRange = levelScale / frame.tRange;

    const float percent = std::clamp(static_cast<float>(fPercent), 0.f, 1.f);
    const float pivotAbs =
            percent * std::clamp(static_cast<float>(fPivotPercent), 0.f, 1.f);
    const float falloff = percent - pivotAbs;
    const float invFalloff = falloff > 0.f ? 1.f / falloff : 0.f;

    using ShaderFlags = skif::FilterResult::ShaderFlags;
    skif::FilterResult::Builder builder{stepCtx};
    builder.add(upscaled, sampleBounds,
                ShaderFlags::kSampledRepeatedly | ShaderFlags::kNonTrivialSampling,
                SkSamplingOptions(SkFilterMode::kLinear));
    builder.add(sharp, targetBounds);
    return builder.eval([&](SkSpan<sk_sp<SkShader>> inputs) -> sk_sp<SkShader> {
        const SkRuntimeEffect* effect = fCircle ? circle_upsample_effect()
                                                : gradient_upsample_effect();
        if (!effect) {
            return inputs[0] ? inputs[0] : inputs[1]; // fallback: prefer the blur signal
        }
        // Replace transparent (null) inputs with a transparent shader so mix works
        sk_sp<SkShader> blurShader = inputs[0] ? inputs[0]
                                               : SkShaders::Color(SK_ColorTRANSPARENT);
        sk_sp<SkShader> sharpShader = inputs[1] ? inputs[1]
                                                : SkShaders::Color(SK_ColorTRANSPARENT);
        SkRuntimeShaderBuilder shaderBuilder(sk_ref_sp(effect));
        shaderBuilder.child("blurred") = std::move(blurShader);
        shaderBuilder.child("sharp") = std::move(sharpShader);
        shaderBuilder.uniform("uLevel") = static_cast<float>(level);
        shaderBuilder.uniform("uMaxLodValue") = maxLodValue;
        if (fCircle) {
            // Convert the circle center/radius to this level's coordinates (1/2^level)
            shaderBuilder.uniform("uCenter") =
                    SkV2{frame.center.fX / levelScale, frame.center.fY / levelScale};
            shaderBuilder.uniform("uInvRadius") = levelScale / frame.radius;
            shaderBuilder.uniform("uPercent") = percent;
            shaderBuilder.uniform("uPivotAbs") = pivotAbs;
            shaderBuilder.uniform("uInvFalloff") = invFalloff;
            return shaderBuilder.makeShader();
        }
        shaderBuilder.uniform("uGradDir") = SkV2{frame.dir.fX, frame.dir.fY};
        shaderBuilder.uniform("uGradMin") = gradMin;
        shaderBuilder.uniform("uGradInvRange") = gradInvRange;
        shaderBuilder.uniform("uPercent") = percent;
        shaderBuilder.uniform("uPivotAbs") = pivotAbs;
        shaderBuilder.uniform("uInvFalloff") = invFalloff;
        return shaderBuilder.makeShader();
    });
}

// ----------------------------------------------------------------------------
// [8] Bounds (same pattern as SkBlurImageFilter)
// ----------------------------------------------------------------------------

skif::LayerSpace<SkSize> SkMipMapedBlurImageFilter::mapSigma(const skif::Mapping& mapping) const {
    skif::LayerSpace<SkSize> sigma = mapping.paramToLayer(fSigma);

    // Non-finite or negligible axes are treated as 0
    if (!SkIsFinite(sigma.width()) || sigma.width() < 0.f) {
        sigma = skif::LayerSpace<SkSize>({0.f, sigma.height()});
    }
    if (!SkIsFinite(sigma.height()) || sigma.height() < 0.f) {
        sigma = skif::LayerSpace<SkSize>({sigma.width(), 0.f});
    }
    return sigma;
}

skif::LayerSpace<SkIRect> SkMipMapedBlurImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    skif::LayerSpace<SkIRect> requiredInput = this->kernelBounds(mapping, desiredOutput);
    return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
}

std::optional<skif::LayerSpace<SkIRect>> SkMipMapedBlurImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    auto childOutput = this->getChildOutputLayerBounds(0, mapping, contentBounds);
    if (childOutput) {
        return this->kernelBounds(mapping, *childOutput);
    } else {
        return skif::LayerSpace<SkIRect>::Unbounded();
    }
}

SkRect SkMipMapedBlurImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    bounds.outset(SkSize(fSigma).width() * 3, SkSize(fSigma).height() * 3);
    return bounds;
}
