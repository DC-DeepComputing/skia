/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkBlenders.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/SkColorData.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/imagefilters/SkCropImageFilter.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

namespace {

class SkBlendImageFilter : public SkImageFilter_Base {
public:
    SkBlendImageFilter(sk_sp<SkBlender> blender,
                       const std::optional<SkV4>& coefficients,
                       bool enforcePremul,
                       sk_sp<SkImageFilter> inputs[2])
            : SkImageFilter_Base(inputs, 2, nullptr)
            , fBlender(std::move(blender))
            , fArithmeticCoefficients(coefficients)
            , fEnforcePremul(enforcePremul) {
        // A null blender represents src-over, which should have been filled in by the factory
        SkASSERT(fBlender);
    }

    SkRect computeFastBounds(const SkRect& bounds) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterBlendImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkBlendImageFilter)
    static sk_sp<SkFlattenable> ArithmeticCreateProc(SkReadBuffer& buffer);

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

    bool onAffectsTransparentBlack() const override {
        // A runtime blender or an arithmetic blender with k3 != 0 affect transparent black
        return !as_BB(fBlender)->asBlendMode().has_value() &&
               (!fArithmeticCoefficients.has_value() || (*fArithmeticCoefficients)[3] != 0.f);
    }

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds,
            VisitChildren recurse) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    static constexpr int kBackground = 0;
    static constexpr int kForeground = 1;

    sk_sp<SkBlender> fBlender;

    // Normally runtime SkBlenders are pessimistic about the bounds they affect. For Arithmetic,
    // we remember the coefficients so that bounds can be reasoned about.
    std::optional<SkV4> fArithmeticCoefficients;
    bool fEnforcePremul; // Remembered to serialize the Arithmetic variant correctly
};

static sk_sp<SkImageFilter> make_blend(sk_sp<SkBlender> blender,
                                       std::optional<SkV4> coefficients,
                                       sk_sp<SkImageFilter> background,
                                       sk_sp<SkImageFilter> foreground,
                                       const SkImageFilters::CropRect& cropRect) {
    if (!blender) {
        blender = SkBlender::Mode(SkBlendMode::kSrcOver);
    }
    sk_sp<SkImageFilter> inputs[2] = { std::move(background), std::move(foreground) };
    sk_sp<SkImageFilter> filter{new SkBlendImageFilter(blender, coefficients, inputs)};
    if (cropRect) {
        filter = SkMakeCropImageFilter(*cropRect, std::move(filter));
    }
    return filter;
}

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Blend(SkBlendMode mode,
                                           sk_sp<SkImageFilter> background,
                                           sk_sp<SkImageFilter> foreground,
                                           const CropRect& cropRect) {
    return make_blend(SkBlender::Mode(mode), {}, std::move(background), std::move(foreground), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Blend(sk_sp<SkBlender> blender,
                                           sk_sp<SkImageFilter> background,
                                           sk_sp<SkImageFilter> foreground,
                                           const CropRect& cropRect) {
    return make_blend(std::move(blender), {}, std::move(background), std::move(foreground), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Arithmetic(SkScalar k1,
                                                SkScalar k2,
                                                SkScalar k3,
                                                SkScalar k4,
                                                bool enforcePMColor,
                                                sk_sp<SkImageFilter> background,
                                                sk_sp<SkImageFilter> foreground,
                                                const CropRect& cropRect) {
    auto blender = SkBlenders::Arithmetic(k1, k2, k3, k4, enforcePMColor);
    if (!blender) {
        // Arithmetic() returns null on an error, not to optimize src-over
        return nullptr;
    }
    return make_blend(std::move(blender), SkV4{k1, k2, k3, k4}, std::move(background), std::move(foreground), cropRect);
}

void SkRegisterBlendImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkBlendImageFilter);
    // While implemented in the same class, the Arithmetic blend avoids serializing an SkBlender.

    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkXfermodeImageFilter_Base", SkBlendImageFilter::CreateProc);
    SkFlattenable::Register("SkXfermodeImageFilterImpl", SkBlendImageFilter::CreateProc);
    SkFlattenable::Register("ArithmeticImageFilterImpl", SkBlendImageFilter::ArithmeticCreateProc);
    SkFlattenable::Register("SkArithmeticImageFilter", SkBlendImageFilter::ArithmeticCreateProc);
}

sk_sp<SkFlattenable> SkBlendImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);

    sk_sp<SkBlender> blender;
    const uint32_t mode = buffer.read32();
    if (mode == kCustom_SkBlendMode) {
        blender = buffer.readBlender();
    } else {
        if (mode > (unsigned)SkBlendMode::kLastMode) {
            buffer.validate(false);
            return nullptr;
        }
        blender = SkBlender::Mode((SkBlendMode)mode);
    }
    return SkImageFilters::Blend(std::move(blender), common.getInput(0), common.getInput(1),
                                 common.cropRect());
}

sk_sp<SkFlattenable> SkBlendImageFilter::ArithmeticCreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    float k[4];
    for (int i = 0; i < 4; ++i) {
        k[i] = buffer.readScalar();
    }
    const bool enforcePremul = buffer.readBool();
    return SkImageFilters::Arithmetic(k[0], k[1], k[2], k[3], enforcePremul,
                                      common.getInput(0), common.getInput(1), common.cropRect());
}

void SkBlendImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);

    // TODO: This isn't entirely valid, we need to differentiate within the buffer contents since
    // they have the same flattenable CreateProc now
    if (fArithmeticCoefficients.has_value()) {
        SkV4 k = *fArithmeticCoefficients;
        for (int i = 0; i < 4; ++i) {
            buffer.writeScalar(k[i]);
        }
        buffer.writeBool(fEnforcePremul);
    } else {
        if (auto bm = as_BB(fBlender)->asBlendMode()) {
            buffer.write32((unsigned)bm.value());
        } else {
            buffer.write32(kCustom_SkBlendMode);
            buffer.writeFlattenable(fBlender.get());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkBlendImageFilter::onFilterImage(const skif::Context& ctx) const {
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    // We skip using filterInput() because that would call onGetInputLayerBounds() multiple times
    // and we want to use the same required input for both foreground and background.
    skif::LayerSpace<SkIRect> requiredInput = this->onGetInputLayerBounds(
            ctx.mapping(), ctx.desiredOutput(), ctx.source().layerBounds(), VisitChildren::kNo);

    skif::Context inputCtx = ctx.withNewDesiredOutput(requiredInput);
    skif::FilterResult bgImage = this->getInput(kBackground) ? as_IFB(this->getInput(kBackground))->filterImage(inputCtx) : ctx.source();
    skif::FilterResult fgImage = this->getInput(kForeground) ? as_IFB(this->getInput(kForeground))->filterImage(inputCtx) : ctx.source();

    skif::FilterResult::Builder builder{ctx};
    builder.add(bgImage);
    builder.add(fgImage);
    return builder.eval([&](SkSpan<sk_sp<SkShader>> inputs) {
            return SkShaders::Blend(fBlender, inputs[kBackground], inputs[kForeground]);
        }, ShaderFlags::kExplicitOutputBounds, requiredInput);
}

skif::LayerSpace<SkIRect> SkBlendImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds,
        VisitChildren recurse) const {
    // We could just return 'desiredOutput' for the blend's required input size, since that's what
    // it is expected to fill. However, some blend modes restrict the output to something other
    // than the union of the foreground and background. To make this restriction available to both
    // children before evaluating them, we determine the maximum possible output the blend can
    // produce from the contentBounds and require that for both children to produce.
    skif::LayerSpace<SkIRect> possibleOutput = this->onGetOutputLayerBounds(mapping, contentBounds);
    if (!possibleOutput.intersect(desiredOutput)) {
        return skif::LayerSpace<SkIRect>::Empty();
    }

    if (recurse == VisitChildren::kNo) {
        return possibleOutput;
    } else {
        skif::LayerSpace<SkIRect> requiredInput = this->getChildInputLayerBounds(kBackground, mapping, possibleOutput, contentBounds);
        requiredInput.join(this->getChildInputLayerBounds(kForeground, mapping, possibleOutput, contentBounds));
        return requiredInput;
    }
}

skif::LayerSpace<SkIRect> SkBlendImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // Blending is (k0*FG*BG +       k1*FG +       k2*BG + k3) for arithmetic blenders OR
    //             ( 0*FG*BG + srcCoeff*FG + dstCoeff*BG + 0 ) for Porter-Duff blend modes OR
    //              un-inspectable(FG, BG) for advanced blend modes and runtime blenders.
    //
    // There are six possible output bounds that can be produced:
    //   1. No output: K = (0,0,0,0) or (srcCoeff,dstCoeff) = (kZero,kZero)
    //   2. intersect(FG,BG): K = (non-zero, 0,0,0) or (srcCoeff,dstCoeff) = (kZero|kDA, kZero|kSA)
    //   3. FG-only: K = (0, non-zero, 0,0) or (srcCoeff,dstCoeff) = (!kZero&!kDA, kZero|kSA)
    //   4. BG-only: K = (0,0, non-zero, 0) or (srcCoeff,dstCoeff) = (kZero|kDA, !kZero&!kSA)
    //   5. union(FG,BG): K = (*,*,*,0) or (srcCoeff,dstCoeff) = (!kZero&!kDA, !kZero&!kSA)
    //        or an advanced blend mode.
    //   6. infinite: K = (*,*,*, non-zero) or a runtime blender.
    // TODO: better name for these. It's not an only, it's that we don't care about content outside of them
    // so if both are true, we take intersection, if one is true we take that one, and if none are true
    // we do the union (because infinite and empty are handled internally)
    bool fgOnly = false;
    bool bgOnly = false;
    if (auto bm = as_BB(fBlender)->asBlendMode()) {
        if (*bm == SkBlendMode::kClear) {
            return skif::LayerSpace<SkIRect>::Empty();
        }
        SkBlendModeCoeff src, dst;
        if (SkBlendMode_AsCoeff(*bm, &src, &dst)) {
            fgOnly = dst == SkBlendModeCoeff::kZero || dst == SkBlendModeCoeff::kSA;
            bgOnly = src == SkBlendModeCoeff::kZero || src == SkBlendModeCoeff::kDA;
        }
    } else if (fArithmeticCoefficients.has_value()) {
        const SkV4& k = *fArithmeticCoefficients;
        SkASSERT(k != SkV4{0.f, 0.f, 0.f, 0.f}); // Should have been converted to a clear blender

        if (k[3] != 0.f) {
            // The arithmetic equation produces non-transparent black everywhere
            return skif::LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
        } else {
            fgOnly = k[0] != 0.f || k[1] != 0.f;
            bgOnly = k[0] != 0.f || k[2] != 0.f;
        }
    } else {
        // A non-arithmetic runtime blender, so pessimistically assume it can return non-transparent
        // black anywhere.
        return skif::LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
    }

    skif::LayerSpace<SkIRect> foregroundBounds =
            this->getChildOutputLayerBounds(kForeground, mapping, contentBounds);
    skif::LayerSpace<SkIRect> backgroundBounds =
            this->getChildOutputLayerBounds(kBackground, mapping, contentBounds);
    if (fgOnly) {
        if (bgOnly) {
            // Output is the intersection of both
            if (!foregroundBounds.intersect(backgroundBounds)) {
                return skif::LayerSpace<SkIRect>::Empty();
            }
        }
        return foregroundBounds;
    } else {
        if (!bgOnly) {
            // Output is the union of both
            backgroundBounds.join(foregroundBounds);
        }
        return backgroundBounds;
    }
}

SkRect SkBlendImageFilter::computeFastBounds(const SkRect& bounds) const {
    // FIXME update this to match getOutputLayerBounds ...
    // FIXME move background/foreground to utility members instead of accessing by index.
        auto getBackground = [&]() {
        return this->getInput(0) ? this->getInput(0)->filterBounds(src, ctm, dir, inputRect) : src;
    };
    auto getForeground = [&]() {
        return this->getInput(1) ? this->getInput(1)->filterBounds(src, ctm, dir, inputRect) : src;
    };

}
