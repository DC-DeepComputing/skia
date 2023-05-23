/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkSafe32.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/imagefilters/SkCropImageFilter.h"

#include <cstdint>
#include <memory>
#include <utility>

#include "include/core/SkM44.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"

namespace {

class SkDisplacementMapImageFilter final : public SkImageFilter_Base {
    static constexpr int kDisplacement = 0;
    static constexpr int kColor = 1;

    static constexpr SkSamplingOptions kDisplacementSampling{SkFilterMode::kLinear};

public:
    SkDisplacementMapImageFilter(SkColorChannel xChannel, SkColorChannel yChannel,
                                 SkScalar scale, sk_sp<SkImageFilter> inputs[2])
            : SkImageFilter_Base(inputs, 2, nullptr)
            , fXChannel(xChannel)
            , fYChannel(yChannel)
            , fScale(scale) {}

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterDisplacementMapImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkDisplacementMapImageFilter)

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    SkColorChannel fXChannel;
    SkColorChannel fYChannel;
    // Scale is really a ParameterSpace<Vector> where width = height = fScale, but we store just the
    // float here for easier serialization and convert to a size in onFilterImage().
    SkScalar fScale;
};

bool channel_selector_type_is_valid(SkColorChannel cst) {
    switch (cst) {
        case SkColorChannel::kR:
        case SkColorChannel::kG:
        case SkColorChannel::kB:
        case SkColorChannel::kA:
            return true;
        default:
            break;
    }
    return false;
}

// FIXME guard with SK_ENABLE_SKSL (but maybe just the whole file since we don't have an alternative)
char channel_swizzle(SkColorChannel c) {
    switch(c) {
        case SkColorChannel::kR: return 'r';
        case SkColorChannel::kG: return 'g';
        case SkColorChannel::kB: return 'b';
        case SkColorChannel::kA: return 'a';
        default: SkUNREACHABLE;
    }
};

sk_sp<SkShader> make_displacement_shader(
        sk_sp<SkShader> displacement,
        sk_sp<SkShader> color,
        skif::LayerSpace<skif::Vector> scale,
        SkColorChannel xChannel,
        SkColorChannel yChannel) {
    // FIXME: I should look at if this can be done in ES2, since it's a reduction by 16x.
    // NOTE: This creates a new runtime effect per unique combination of XY channel swizzles. If
    // this is creating too many pipeline variations, we can revisit swizzling via uniform.
    struct EffectCacheEntry {
        SkOnce once;
        SkRuntimeEffect* effect;
    };
    static EffectCacheEntry kEffectCache[16]; // 4(rgba)^2(xy)

    const int cacheIndex = (int) xChannel | (((int) yChannel) << 2);
    SkASSERT(cacheIndex >= 0 && cacheIndex < (int) std::size(kEffectCache));
    EffectCacheEntry* cacheEntry = &kEffectCache[cacheIndex];

    cacheEntry->once([&] {
        SkString sksl = SkStringPrintf(
            "uniform shader displMap;"
            "uniform shader colorMap;"
            "uniform half2 scale;"

            "half4 main(float2 coord) {"
                "half2 displ = scale * (unpremul(displMap.eval(coord)).%c%c - 0.5);"
                "return colorMap.eval(coord + displ);"
            "}", channel_swizzle(xChannel), channel_swizzle(yChannel));
        cacheEntry->effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, sksl.c_str());
    });

    SkRuntimeShaderBuilder builder(sk_ref_sp(cacheEntry->effect));
    builder.child("displMap") = std::move(displacement);
    builder.child("colorMap") = std::move(color);
    builder.uniform("scale") = SkV2{scale.x(), scale.y()};

    return builder.makeShader();
}

}  // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkImageFilters::DisplacementMap(
        SkColorChannel xChannelSelector, SkColorChannel yChannelSelector, SkScalar scale,
        sk_sp<SkImageFilter> displacement, sk_sp<SkImageFilter> color, const CropRect& cropRect) {
    if (!channel_selector_type_is_valid(xChannelSelector) ||
        !channel_selector_type_is_valid(yChannelSelector)) {
        return nullptr;
    }

    sk_sp<SkImageFilter> inputs[2] = { std::move(displacement), std::move(color) };
    sk_sp<SkImageFilter> filter(new SkDisplacementMapImageFilter(xChannelSelector, yChannelSelector,
                                                                 scale, inputs));
    if (cropRect) {
        filter = SkMakeCropImageFilter(*cropRect, std::move(filter));
    }
    return filter;
}

void SkRegisterDisplacementMapImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkDisplacementMapImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkDisplacementMapEffect", SkDisplacementMapImageFilter::CreateProc);
    SkFlattenable::Register("SkDisplacementMapEffectImpl",
                            SkDisplacementMapImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkDisplacementMapImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);

    SkColorChannel xsel = buffer.read32LE(SkColorChannel::kLastEnum);
    SkColorChannel ysel = buffer.read32LE(SkColorChannel::kLastEnum);
    SkScalar      scale = buffer.readScalar();

    return SkImageFilters::DisplacementMap(xsel, ysel, scale, common.getInput(0),
                                           common.getInput(1), common.cropRect());
}

void SkDisplacementMapImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeInt((int) fXChannel);
    buffer.writeInt((int) fYChannel);
    buffer.writeScalar(fScale);
}

///////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkDisplacementMapImageFilter::onFilterImage(const skif::Context& ctx) const {
    // FIXME share a outset mapped rect function between these 3 members
    // NOTE: The scale is a "vector" not a "size" since we want to preserve negations on the final
    // displacement vector.
    skif::LayerSpace<skif::Vector> scale =
            ctx.mapping().paramToLayer(skif::ParameterSpace<skif::Vector>({fScale, fScale}));
    // But we treat the maximum displacement the scale can produce as a size for bounds purposes.
    skif::LayerSpace<SkISize> maxDisplacement =
            skif::LayerSpace<SkSize>({0.5f*SkScalarAbs(scale.x()),
                                      0.5f*SkScalarAbs(scale.y())}).ceil();

    skif::LayerSpace<SkIRect> requiredColorInput = ctx.desiredOutput();
    requiredColorInput.outset(maxDisplacement);
    skif::FilterResult colorOutput =
            this->getChildOutput(kColor, ctx.withNewDesiredOutput(requiredColorInput));
    if (!colorOutput) {
        return {}; // No non-transparent black colors to displace
    }

    // When the color image filter is unrestricted, its output will be 'maxDisplacement' larger than
    // this filter's desired output. However, if it is cropped, we can restrict this filter's output
    // It's not a simple intersection, since we have to account for how far the color's output might
    // be displaced.
    skif::LayerSpace<SkIRect> outputBounds = colorOutput.layerBounds();
    // This is a double outset for edges where color was unrestricted, but that's fine since we are
    // intersecting against 'desiredOutput'. For edges that were cropped, the outset represents
    // how far they can be displaced, which might represent a subset of 'desiredOutput'.
    outputBounds.outset(maxDisplacement);
    if (!outputBounds.intersect(ctx.desiredOutput())) {
        // None of the non-transparent black colors can be displaced into the desired bounds.
        return {};
    }

    // Creation of the displacement map should happen in a non-colorspace aware context. This
    // texture is a purely mathematical construct, so we want to just operate on the stored
    // values. Consider:
    //
    //   User supplies an sRGB displacement map. If we're rendering to a wider gamut, then we could
    //   end up filtering the displacement map into that gamut, which has the effect of reducing
    //   the amount of displacement that it represents (as encoded values move away from the
    //   primaries).
    //
    //   With a more complex DAG attached to this input, it's not clear that working in ANY specific
    //   color space makes sense, so we ignore color spaces (and gamma) entirely. This may not be
    //   ideal, but it's at least consistent and predictable.
    skif::FilterResult displacementOutput =
            this->getChildOutput(kDisplacement, ctx.withNewDesiredOutput(outputBounds)
                                                   .withNewColorSpace(/*cs=*/nullptr));
    if (!displacementOutput) {
        // A null displacement map means its transparent black, but (0,0,0,0) becomes the vector
        // (-scale/2, -scale/2) applied to the color image, so represent the displacement as a
        // simple transform.
        skif::LayerSpace<SkMatrix> constantDisplacement{SkMatrix::Translate(-0.5f * scale.x(),
                                                                            -0.5f * scale.y())};
        return colorOutput.applyTransform(ctx, constantDisplacement, kDisplacementSampling);
    }

    // If we made it this far, then we actually have per-pixel displacement affecting the color
    // image. We need to evaluate each pixel within 'outputBounds'.
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    skif::FilterResult::Builder builder{ctx};
    builder.add(displacementOutput);
    builder.add(colorOutput);
    // FIXME -> maybe have flags and extra sampling per added? color is bilerp and non-linear
    // but displacement is NN and pixel aligned with the layer.
    return builder.eval([&](SkSpan<sk_sp<SkShader>> inputs) -> sk_sp<SkShader> {
            sk_sp<SkShader> displacement = inputs[kDisplacement];
            sk_sp<SkShader> color = inputs[kColor];
            if (!color) {
                // Color is fully transparent, so no point in displacing it
                return nullptr;
            }
            if (!displacement) {
                // Somehow we had a valid displacement image but failed to produce a shader
                // (e.g. an internal resolve to a new image failed). Treat the displacement as
                // transparent, but it's too late to switch to the applyTransform() optimization.
                displacement = SkShaders::Color(SK_ColorTRANSPARENT);
            }

            return make_displacement_shader(std::move(displacement), std::move(color),
                                            scale, fXChannel, fYChannel);
        }, ShaderFlags::kNonLinearSampling |
           ShaderFlags::kExplicitOutputBounds, outputBounds, kDisplacementSampling);
}

skif::LayerSpace<SkIRect> SkDisplacementMapImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // Pixels up to the maximum displacement away from 'desiredOutput' can be moved into those
    // bounds, depending on how the displacement map renders. To ensure those colors are defined,
    // we require that outset buffer around 'desiredOutput' from the color map.
    skif::LayerSpace<SkSize> scale =
            mapping.paramToLayer(skif::ParameterSpace<SkSize>({0.5f * fScale, 0.5f * fScale}));
    skif::LayerSpace<SkIRect> requiredInput = desiredOutput;
    requiredInput.outset(scale.ceil());

    requiredInput = this->getChildInputLayerBounds(kColor, mapping, requiredInput, contentBounds);
    // Accumulate the required input for the displacement filter to cover the original desired out
    requiredInput.join(this->getChildInputLayerBounds(
            kDisplacement, mapping, desiredOutput, contentBounds));
    return requiredInput;
}

skif::LayerSpace<SkIRect> SkDisplacementMapImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // The maximum displacement is [-scale/2, scale/2], applied to the color image. We treat this
    // as a "size" for bounds purposes. The displacement image's output doesn't impact the output.
    // FIXME not true exactly?
    skif::LayerSpace<SkSize> scale =
            mapping.paramToLayer(skif::ParameterSpace<SkSize>({0.5f * fScale, 0.5f * fScale}));
    skif::LayerSpace<SkIRect> colorOutput =
            this->getChildOutputLayerBounds(kColor, mapping, contentBounds);
    colorOutput.outset(scale.ceil());
    return colorOutput;
}

SkRect SkDisplacementMapImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect colorBounds = this->getInput(kColor) ? this->getInput(kColor)->computeFastBounds(src)
                                                : src;
    float maxDisplacement = 0.5f * SkScalarAbs(fScale);
    return colorBounds.makeOutset(maxDisplacement, maxDisplacement);
}
