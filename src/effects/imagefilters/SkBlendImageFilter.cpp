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

    static constexpr uint32_t kArithmetic_BlendMode = kCustom_SkBlendMode + 1;

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
                                       sk_sp<SkImageFilter> background,
                                       sk_sp<SkImageFilter> foreground,
                                       const SkImageFilters::CropRect& cropRect,
                                       std::optional<SkV4> coefficients = {},
                                       bool enforcePremul = false) {
    if (!blender) {
        blender = SkBlender::Mode(SkBlendMode::kSrcOver);
    }
    sk_sp<SkImageFilter> inputs[2] = { std::move(background), std::move(foreground) };
    sk_sp<SkImageFilter> filter{new SkBlendImageFilter(blender, coefficients,
                                                       enforcePremul, inputs)};
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
    return make_blend(SkBlender::Mode(mode),
                      std::move(background),
                      std::move(foreground),
                      cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Blend(sk_sp<SkBlender> blender,
                                           sk_sp<SkImageFilter> background,
                                           sk_sp<SkImageFilter> foreground,
                                           const CropRect& cropRect) {
    return make_blend(std::move(blender), std::move(background), std::move(foreground), cropRect);
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
    return make_blend(std::move(blender),
                      std::move(background),
                      std::move(foreground),
                      cropRect,
                      // Carry arithmetic coefficients and premul behavior into image filter for
                      // serialization and bounds analysis
                      SkV4{k1, k2, k3, k4},
                      enforcePMColor);
}

void SkRegisterBlendImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkBlendImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkXfermodeImageFilter_Base", SkBlendImageFilter::CreateProc);
    SkFlattenable::Register("SkXfermodeImageFilterImpl", SkBlendImageFilter::CreateProc);
    SkFlattenable::Register("ArithmeticImageFilterImpl", SkBlendImageFilter::ArithmeticCreateProc);
    SkFlattenable::Register("SkArithmeticImageFilter", SkBlendImageFilter::ArithmeticCreateProc);
}

sk_sp<SkFlattenable> SkBlendImageFilter::ArithmeticCreateProc(SkReadBuffer& buffer) {
    // Newer SKPs should be using the updated Blend CreateProc.
    if (!buffer.validate(buffer.isVersionLT(SkPicturePriv::kCombineBlendArithmeticFilters))) {
        return nullptr;
    }

    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    float k[4];
    for (int i = 0; i < 4; ++i) {
        k[i] = buffer.readScalar();
    }
    const bool enforcePremul = buffer.readBool();
    return SkImageFilters::Arithmetic(k[0], k[1], k[2], k[3], enforcePremul,
                                      common.getInput(0), common.getInput(1), common.cropRect());
}

sk_sp<SkFlattenable> SkBlendImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);

    sk_sp<SkBlender> blender;
    std::optional<SkV4> coefficients;
    bool enforcePremul = false;

    const uint32_t mode = buffer.read32();
    if (mode == kArithmetic_BlendMode) {
        // Should only see this sentinel value in newer SKPs
        if (buffer.validate(!buffer.isVersionLT(SkPicturePriv::kCombineBlendArithmeticFilters))) {
            SkV4 k;
            for (int i = 0; i < 4; ++i) {
                k[i] = buffer.readScalar();
            }
            coefficients = k;
            enforcePremul = buffer.readBool();
            blender = SkBlenders::Arithmetic(k.x, k.y, k.z, k.w, enforcePremul);
            if (!buffer.validate(SkToBool(blender))) {
                return nullptr; // Null arithmetic blender are an error condition
            }
        }
    } else if (mode == kCustom_SkBlendMode) {
        blender = buffer.readBlender();
    } else {
        if (!buffer.validate(mode <= (unsigned) SkBlendMode::kLastMode)) {
            return nullptr;
        }
        blender = SkBlender::Mode((SkBlendMode)mode);
    }

    return make_blend(std::move(blender),
                      common.getInput(kBackground),
                      common.getInput(kForeground),
                      common.cropRect(),
                      coefficients,
                      enforcePremul);
}

void SkBlendImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    if (fArithmeticCoefficients.has_value()) {
        buffer.write32(kArithmetic_BlendMode);
        buffer.writeScalarArray(fArithmeticCoefficients->ptr(), 4);
        buffer.writeBool(fEnforcePremul);
    } else if (auto bm = as_BB(fBlender)->asBlendMode()) {
        buffer.write32((unsigned)bm.value());
    } else {
        buffer.write32(kCustom_SkBlendMode);
        buffer.writeFlattenable(fBlender.get());
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

    skif::FilterResult bgImage = this->getInput(kBackground) ?
            as_IFB(this->getInput(kBackground))->filterImage(inputCtx) : ctx.source();
    skif::FilterResult fgImage = this->getInput(kForeground) ?
            as_IFB(this->getInput(kForeground))->filterImage(inputCtx) : ctx.source();

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

SkIRect SkBlendImageFilter::onFilterBounds(const SkIRect& src,
                                           const SkMatrix& ctm,
                                           MapDirection dir,
                                           const SkIRect* inputRect) const {
    if (kReverse_MapDirection == dir) {
        return INHERITED::onFilterBounds(src, ctm, dir, inputRect);
    }

    SkASSERT(!inputRect);
    SkASSERT(2 == this->countInputs());
    auto getBackground = [&]() {
        return this->getInput(0) ? this->getInput(0)->filterBounds(src, ctm, dir, inputRect) : src;
    };
    auto getForeground = [&]() {
        return this->getInput(1) ? this->getInput(1)->filterBounds(src, ctm, dir, inputRect) : src;
    };
    if (auto bm = as_BB(fBlender)->asBlendMode()) {
        switch (bm.value()) {
            case SkBlendMode::kClear:
                return SkIRect::MakeEmpty();

            case SkBlendMode::kSrc:
            case SkBlendMode::kDstATop:
                return getForeground();

            case SkBlendMode::kDst:
            case SkBlendMode::kSrcATop:
                return getBackground();

            case SkBlendMode::kSrcIn:
            case SkBlendMode::kDstIn: {
                auto result = getBackground();
                if (!result.intersect(getForeground())) {
                    return SkIRect::MakeEmpty();
                }
                return result;
            }
            default: break;
        }
    }
    auto result = getBackground();
    result.join(getForeground());
    return result;
}

void SkBlendImageFilter::drawForeground(SkCanvas* canvas, SkSpecialImage* img,
                                        const SkIRect& fgBounds) const {
    SkPaint paint;
    paint.setBlender(fBlender);
    if (img) {
        img->draw(canvas, SkIntToScalar(fgBounds.fLeft), SkIntToScalar(fgBounds.fTop),
                  SkSamplingOptions(), &paint);
    }

    SkAutoCanvasRestore acr(canvas, true);
    canvas->clipRect(SkRect::Make(fgBounds), SkClipOp::kDifference);
    paint.setColor(0);
    canvas->drawPaint(paint);
}

#if defined(SK_GANESH)

sk_sp<SkSpecialImage> SkBlendImageFilter::filterImageGPU(const Context& ctx,
                                                         sk_sp<SkSpecialImage> background,
                                                         const SkIPoint& backgroundOffset,
                                                         sk_sp<SkSpecialImage> foreground,
                                                         const SkIPoint& foregroundOffset,
                                                         const SkIRect& bounds) const {
    SkASSERT(ctx.gpuBacked());

    auto rContext = ctx.getContext();

    GrSurfaceProxyView backgroundView, foregroundView;

    if (background) {
        backgroundView = background->view(rContext);
    }

    if (foreground) {
        foregroundView = foreground->view(rContext);
    }

    std::unique_ptr<GrFragmentProcessor> fp;
    const auto& caps = *ctx.getContext()->priv().caps();
    GrSamplerState sampler(GrSamplerState::WrapMode::kClampToBorder,
                           GrSamplerState::Filter::kNearest);

    if (backgroundView.asTextureProxy()) {
        SkRect bgSubset = SkRect::Make(background->subset());
        SkMatrix bgMatrix = SkMatrix::Translate(
                SkIntToScalar(bgSubset.left() - backgroundOffset.fX),
                SkIntToScalar(bgSubset.top()  - backgroundOffset.fY));
        fp = GrTextureEffect::MakeSubset(std::move(backgroundView), background->alphaType(),
                                         bgMatrix, sampler, bgSubset, caps);
        fp = GrColorSpaceXformEffect::Make(std::move(fp), background->getColorSpace(),
                                           background->alphaType(), ctx.colorSpace(),
                                           kPremul_SkAlphaType);
    } else {
        fp = GrFragmentProcessor::MakeColor(SK_PMColor4fTRANSPARENT);
    }

    GrImageInfo info(ctx.grColorType(), kPremul_SkAlphaType, ctx.refColorSpace(), bounds.size());

    if (foregroundView.asTextureProxy()) {
        SkRect fgSubset = SkRect::Make(foreground->subset());
        SkMatrix fgMatrix = SkMatrix::Translate(
                SkIntToScalar(fgSubset.left() - foregroundOffset.fX),
                SkIntToScalar(fgSubset.top()  - foregroundOffset.fY));
        auto fgFP = GrTextureEffect::MakeSubset(std::move(foregroundView), foreground->alphaType(),
                                                fgMatrix, sampler, fgSubset, caps);
        fgFP = GrColorSpaceXformEffect::Make(std::move(fgFP), foreground->getColorSpace(),
                                             foreground->alphaType(), ctx.colorSpace(),
                                             kPremul_SkAlphaType);

        SkSurfaceProps props{}; // default OK; blend-image filters don't render text
        GrFPArgs args(rContext, &info.colorInfo(), props);

        fp = as_BB(fBlender)->asFragmentProcessor(std::move(fgFP), std::move(fp), args);
    }

    auto sfc = rContext->priv().makeSFC(
            info, "BlendImageFilter_FilterImageGPU", SkBackingFit::kApprox);
    if (!sfc) {
        return nullptr;
    }

    sfc->fillRectToRectWithFP(bounds, SkIRect::MakeSize(bounds.size()), std::move(fp));

    return SkSpecialImage::MakeDeferredFromGpu(rContext,
                                               SkIRect::MakeWH(bounds.width(), bounds.height()),
                                               kNeedNewImageUniqueID_SpecialImage,
                                               sfc->readSurfaceView(),
                                               sfc->colorInfo(),
                                               ctx.surfaceProps());
}

#endif
