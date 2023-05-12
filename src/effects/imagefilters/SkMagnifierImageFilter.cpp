/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkTPin.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/imagefilters/SkCropImageFilter.h"

#include <algorithm>
#include <memory>
#include <utility>

#ifdef SK_ENABLE_SKSL
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"
#endif

namespace {

class SkMagnifierImageFilter final : public SkImageFilter_Base {
public:
    SkMagnifierImageFilter(const SkRect& lensBounds,
                           float zoomAmount,
                           float inset,
                           const SkSamplingOptions& sampling,
                           sk_sp<SkImageFilter> input)
        : SkImageFilter_Base(&input, 1, nullptr)
        , fLensBounds(lensBounds)
        , fZoomAmount(zoomAmount)
        , fInset(inset)
        , fSampling(sampling) {}

    SkRect computeFastBounds(const SkRect&) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterMagnifierImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkMagnifierImageFilter)

    skif::FilterResult onFilterImage(const skif::Context& context) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds,
            VisitChildren recurse) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::ParameterSpace<SkRect> fLensBounds;
    // Zoom is relative so does not belong to a coordinate space, see note in onFilterImage().
    float fZoomAmount;
    // Inset is really a ParameterSpace<SkSize> where width = height = fInset, but we store just the
    // float here for easier serialization and convert to a size in onFilterImage().
    float fInset;
    SkSamplingOptions fSampling;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Magnifier(const SkRect& lensBounds,
                                               SkScalar zoomAmount,
                                               SkScalar inset,
                                               const SkSamplingOptions& sampling,
                                               sk_sp<SkImageFilter> input,
                                               const CropRect& cropRect) {
    if (lensBounds.isEmpty() || !lensBounds.isFinite() ||
        zoomAmount <= 0.f || !SkScalarIsFinite(zoomAmount) ||
        inset < 0.f || !SkScalarIsFinite(inset)) {
        return nullptr; // invalid
    }
    // The magnifier automatically restricts its output based on the size of the image it receives
    // as input, so 'cropRect' only applies to its input.
    if (cropRect) {
        input = SkMakeCropImageFilter(*cropRect, std::move(input));
    }

    if (zoomAmount > 1.f) {
        return sk_sp<SkImageFilter>(new SkMagnifierImageFilter(lensBounds, zoomAmount, inset,
                                                               sampling, std::move(input)));
    } else {
        // Zooming with a value less than 1 is technically a downscaling, which "works" but the
        // non-linear distortion behaves unintuitively. At zoomAmount = 1, this filter is an
        // expensive identity function so treat zoomAmount <= 1 as a no-op.
        return input;
    }
}

void SkRegisterMagnifierImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkMagnifierImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkMagnifierImageFilterImpl", SkMagnifierImageFilter::CreateProc);
    SkFlattenable::Register("SkLegacyMagnifierImageFilter", SkMagnifierImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkMagnifierImageFilter::CreateProc(SkReadBuffer& buffer) {
    if (buffer.isVersionLT(SkPicturePriv::kRemoveLegacyMagnifierFilter)) {
        // This was actually a legacy magnifier image filter that was serialized, which is no
        // longer supported. There shouldn't be any long-lived SKPs in the wild that use this since
        // it's primarily ChromeOS oriented.
        return nullptr;
    }

    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    SkRect lensBounds;
    buffer.readRect(&lensBounds);
    SkScalar zoomAmount = buffer.readScalar();
    SkScalar inset = buffer.readScalar();
    SkSamplingOptions sampling = buffer.readSampling();
    return SkImageFilters::Magnifier(lensBounds, zoomAmount, inset, sampling, common.getInput(0));
}

void SkMagnifierImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeRect(SkRect(fLensBounds));
    buffer.writeScalar(fZoomAmount);
    buffer.writeScalar(fInset);
    buffer.writeSampling(fSampling);
}

////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkShader> make_magnifier_shader(
        sk_sp<SkShader> input,
        const skif::LayerSpace<SkRect>& lensBounds,
        const skif::LayerSpace<SkRect>& srcRect,
        const skif::LayerSpace<SkSize>& inset) {
#ifdef SK_ENABLE_SKSL
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader src;"
        "uniform float4 lensBounds;"
        "uniform float4 zoomXform;"
        "uniform float2 invInset;"

        "half4 main(float2 coord) {"
            "float2 zoomCoord = zoomXform.xy + zoomXform.zw*coord;"
            // edgeInset is the smallest distance to the lens bounds edges,
            // in units of "insets".
            "float2 edgeInset = min(coord - lensBounds.xy, lensBounds.zw - coord) * invInset;"

            // The equations for 'weight' ensure that it is 0 along the outside of lensBounds so
            // it seams with any un-zoomed, un-filtered content. The zoomed content fills a rounded
            // rectangle that is 1 "inset" in from lensBounds with circular corners with radii
            // equal to the inset distance. Outside of this region, there is a non-linear weighting
            // to compress the un-zoomed content to the zoomed content. The critical zone about
            // each corner is limited to 2x"inset" square.
            "float weight = (edgeInset.x < 2.0 && edgeInset.y < 2.0)"
                // Circular distortion weighted by distance to inset corner
                "? (2.0 - length(2.0 - edgeInset))"
                // Linear zoom, or single-axis compression outside of the inset area (if delta < 1)
                ": min(edgeInset.x, edgeInset.y);"

            // Saturate before squaring so that negative weights are clamped to 0 before squaring
            "weight = saturate(weight);"
            "return src.eval(mix(coord, zoomCoord, weight*weight));"
        "}"
    );

    SkRuntimeShaderBuilder builder(sk_ref_sp(effect));
    SkMatrix zoomXform = SkMatrix::RectToRect(SkRect(lensBounds), SkRect(srcRect));

    builder.uniform("lensBounds") = SkRect(lensBounds);
    builder.uniform("zoomXform") = SkV4{zoomXform.getTranslateX(), zoomXform.getTranslateY(),
                                        zoomXform.getScaleX(),     zoomXform.getScaleY()};
    builder.uniform("invInset") = SkV2{1.f / inset.width(),
                                       1.f / inset.height()};

    builder.child("src") = std::move(input);
    return builder.makeShader();
#else
    // TODO (michaelludwig): Once the legacy magnifier is deleted, this SK_ENABLE_SKSL guard can
    // be moved to surround the entire implementation. Since sksl is required for this filter, the
    // Magnifier factory can be stubbed out easily at that point to return the input image filter.
    return nullptr;
#endif // SK_ENABLE_SKSL
}

skif::FilterResult SkMagnifierImageFilter::onFilterImage(const skif::Context& context) const {
    skif::FilterResult childOutput = this->filterInput(0, context);

    skif::LayerSpace<SkRect> lensBounds = context.mapping().paramToLayer(fLensBounds);
    skif::LayerSpace<SkPoint> zoomCenter = lensBounds.center();

    // If lensBounds is not partially off screen, 'childOutput' should exactly match the layer-space
    // lens bounds. However, when this is used as a backdrop filter, or if there was a crop on the
    // input, this may not be the case. Stylistically, this filter adjusts the lens bounds and
    // zoomed-in content such that the non-linear inset does not extend beyond what was provided.
    // This avoids zooming in on a clamped texture boundary.
    if (!lensBounds.intersect(skif::LayerSpace<SkRect>(childOutput.layerBounds()))) {
        return {};
    }
    // Clamp the zoom center to be within the childOutput image
    zoomCenter = lensBounds.clamp(zoomCenter);

    // The zoom we want to apply in layer-space is equal to
    // mapping.paramToLayer(SkMatrix::Scale(fZoomAmount)).decomposeScale(&layerZoom).
    // Because this filter only supports scale+translate matrices, the paramToLayer transform of
    // the parameter-space scale matrix is a no-op. Thus layerZoom == fZoomAmount and we can avoid
    // all of that math. This assumption is invalid if the matrix complexity is more than S+T.
    SkASSERT(this->getCTMCapability() == MatrixCapability::kScaleTranslate);
    float invZoom = 1.f / fZoomAmount;

    // The srcRect is the bounding box of the pixels that are linearly scaled up, about zoomCenter.
    // This is not the visual bounds of this upscaled region, but the bounds of the source pixels
    // that will fill the main magnified region (which is simply the inset of lensBounds). When
    // lensBounds has not been cropped by the actual input image, these equations are identical to
    // the more intuitive L/R = center.x -/+ width/(2*zoom) and T/B = center.y -/+ height/(2*zoom).
    // However, when lensBounds is cropped this automatically shifts the source rectangle away from
    // the original zoom center such that the upscaled area is contained within the input image.
    skif::LayerSpace<SkRect> srcRect{{
            lensBounds.left()  * invZoom + zoomCenter.x()*(1.f - invZoom),
            lensBounds.top()   * invZoom + zoomCenter.y()*(1.f - invZoom),
            lensBounds.right() * invZoom + zoomCenter.x()*(1.f - invZoom),
            lensBounds.bottom()* invZoom + zoomCenter.y()*(1.f - invZoom)}};

    skif::LayerSpace<SkSize> inset = context.mapping().paramToLayer(
            skif::ParameterSpace<SkSize>({fInset, fInset}));

    using ShaderFlags = skif::FilterResult::ShaderFlags;
    skif::FilterResult::Builder builder{context};
    builder.add(childOutput);
    return builder.eval([&](SkSpan<sk_sp<SkShader>> inputs) {
            return make_magnifier_shader(inputs[0], lensBounds, srcRect, inset);
        },
        ShaderFlags::kExplicitOutputBounds | ShaderFlags::kNonLinearSampling,
        lensBounds.roundOut(), fSampling);
}

skif::LayerSpace<SkIRect> SkMagnifierImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds,
        VisitChildren recurse) const {
    // The required input is always the lens bounds. The filter distorts the pixels contained within
    // these bounds to zoom in on a portion of it, depending on the inset and zoom amount. However,
    // it adjusts the region based on cropping that occurs between what's requested and what's
    // provided. Theoretically it's possible that we could restrict the required input by the
    // desired output, but that cropping should not adjust the zoom region or inset. This is non
    // trivial to separate and is an unlikely use case so for now just require fLensBounds.
    skif::LayerSpace<SkIRect> requiredInput = mapping.paramToLayer(fLensBounds).roundOut();
    if (recurse == VisitChildren::kNo) {
        return requiredInput;
    } else {
        // Our required input is the desired output for our child image filter.
        return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
    }
}

skif::LayerSpace<SkIRect> SkMagnifierImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // The output of this filter is fLensBounds intersected with its child's output.
    skif::LayerSpace<SkIRect> output = this->getChildOutputLayerBounds(0, mapping, contentBounds);
    if (output.intersect(mapping.paramToLayer(fLensBounds).roundOut())) {
        return output;
    } else {
        // Nothing to magnify
        return skif::LayerSpace<SkIRect>::Empty();
    }
}

SkRect SkMagnifierImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    if (bounds.intersect(SkRect(fLensBounds))) {
        return bounds;
    } else {
        return SkRect::MakeEmpty();
    }
}
