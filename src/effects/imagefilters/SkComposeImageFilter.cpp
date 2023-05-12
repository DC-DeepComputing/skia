/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkSpecialImage.h"

#include <utility>

class SkMatrix;
class SkReadBuffer;

namespace {

class SkComposeImageFilter final : public SkImageFilter_Base {
public:
    explicit SkComposeImageFilter(sk_sp<SkImageFilter> inputs[2])
            : SkImageFilter_Base(inputs, 2, nullptr) {
        SkASSERT(inputs[0]);
        SkASSERT(inputs[1]);
    }

    SkRect computeFastBounds(const SkRect& src) const override;

    // No need to override flatten() since there is no extra data to write beyond the children
    // handled by the parent class.

private:
    friend void ::SkRegisterComposeImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkComposeImageFilter)

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

    skif::FilterResult onFilterImage(const skif::Context& context) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds,
            VisitChildren recurse) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    static constexpr int kOuter = 0;
    static constexpr int kInner = 1;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Compose(sk_sp<SkImageFilter> outer,
                                             sk_sp<SkImageFilter> inner) {
    if (!outer) {
        return inner;
    }
    if (!inner) {
        return outer;
    }
    sk_sp<SkImageFilter> inputs[2] = { std::move(outer), std::move(inner) };
    return sk_sp<SkImageFilter>(new SkComposeImageFilter(inputs));
}

void SkRegisterComposeImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkComposeImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkComposeImageFilterImpl", SkComposeImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkComposeImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    return SkImageFilters::Compose(common.getInput(0), common.getInput(1));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkComposeImageFilter::onFilterImage(const skif::Context& ctx) const {
    // TODO: Consolidate with onGetInputLayerBounds() when dry-runs are a thing

    // The expected output of the inner image filter is the content bounds--or equivalently, the
    // expected layer bounds of the outer filter's Context's source image.
    const skif::LayerSpace<SkIRect> innerOutputBounds = this->getChildOutputLayerBounds(kInner, ctx.mapping(), ctx.source().layerBounds());
    // The required input of the outer filter will be the desired output of the inner's Context.
    const skif::LayerSpace<SkIRect> outerRequiredInput = this->getChildInputLayerBounds(kOuter, ctx.mapping(), ctx.desiredOutput(), innerOutputBounds);

    // The actual output of the inner filter becomes the "source" image seen by the outer filter.
    skif::FilterResult innerResult = as_IFB(this->getInput(kInner))->filterImage(ctx.withNewDesiredOutput(outerRequiredInput));
    return as_IFB(this->getInput(kOuter))->filterImage(ctx.withNewSource(innerResult));
}

skif::LayerSpace<SkIRect> SkComposeImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds,
        VisitChildren recurse) const {
    SkASSERT(recurse == VisitChildren::kYes);
    // The output of the inner filter operating on 'contentBounds' is the content available to the
    // outer image filter.
    const skif::LayerSpace<SkIRect> innerOutputBounds = this->getChildOutputLayerBounds(kInner, mapping, contentBounds);
    // The required input of the outer filter will be the desired output of the inner filter.
    const skif::LayerSpace<SkIRect> outerRequiredInput = this->getChildInputLayerBounds(kOuter, mapping, desiredOutput, innerOutputBounds);
    return this->getChildInputLayerBounds(kInner, mapping, outerRequiredInput, contentBounds);
}

skif::LayerSpace<SkIRect> SkComposeImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // The inner image filter operates on 'contentBounds'. Its output bounds becomes the
    // 'contentBounds' of the outer image filter.
    const skif::LayerSpace<SkIRect> innerOutputBounds =
            this->getChildOutputLayerBounds(kInner, mapping, contentBounds);
    return this->getChildOutputLayerBounds(kOuter, mapping, innerOutputBounds);
}

SkRect SkComposeImageFilter::computeFastBounds(const SkRect& src) const {
    const SkRect innerOutputBounds = this->getInput(kInner)->computeFastBounds(src);
    return this->getInput(kOuter)->computeFastBounds(innerOutputBounds);
}
