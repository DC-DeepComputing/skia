/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

#include <utility>

namespace {

class SkPictureImageFilter final : public SkImageFilter_Base {
public:
    SkPictureImageFilter(sk_sp<SkPicture> picture, const SkRect& cullRect)
            : SkImageFilter_Base(nullptr, 0, nullptr)
            , fPicture(std::move(picture))
            , fCullRect(cullRect) {
        // The external cullrect should already have been intersected with the internal cull rect
        SkASSERT((!fPicture && cullRect.isEmpty()) ||
                 (fPicture && fPicture->cullRect().contains(cullRect)));
    }

    SkRect computeFastBounds(const SkRect&) const override { return SkRect(fCullRect); }

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterPictureImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkPictureImageFilter)

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

    skif::FilterResult onFilterImage(const skif::Context& ctx) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping&,
            const skif::LayerSpace<SkIRect>&,
            const skif::LayerSpace<SkIRect>&,
            VisitChildren) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>&) const override;

    sk_sp<SkPicture> fPicture;
    skif::ParameterSpace<SkRect> fCullRect;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Picture(sk_sp<SkPicture> pic, const SkRect& targetRect) {
    return sk_sp<SkImageFilter>(new SkPictureImageFilter(std::move(pic), targetRect));
}

void SkRegisterPictureImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkPictureImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkPictureImageFilterImpl", SkPictureImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkPictureImageFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkPicture> picture;
    if (buffer.readBool()) {
        picture = SkPicturePriv::MakeFromBuffer(buffer);
    }

    SkRect cullRect;
    buffer.readRect(&cullRect);
    return SkImageFilters::Picture(std::move(picture), cullRect);
}

void SkPictureImageFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeBool(SkToBool(fPicture));
    if (fPicture) {
        SkPicturePriv::Flatten(fPicture, buffer);
    }
    buffer.writeRect(SkRect(fCullRect));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkPictureImageFilter::onFilterImage(const skif::Context& ctx) const {
    return skif::FilterResult::MakeFromPicture(ctx, fPicture, fCullRect);
}

skif::LayerSpace<SkIRect> SkPictureImageFilter::onGetInputLayerBounds(
        const skif::Mapping&,
        const skif::LayerSpace<SkIRect>&,
        const skif::LayerSpace<SkIRect>&,
        VisitChildren) const {
    // This is a leaf filter, it requires no input and no further recursion
    return skif::LayerSpace<SkIRect>::Empty();
}

skif::LayerSpace<SkIRect> SkPictureImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>&) const {
    if (fPicture) {
        // The output is the transformed bounds of the picture.
        return mapping.paramToLayer(fCullRect).roundOut();
    } else {
        // An empty picture is fully transparent
        return skif::LayerSpace<SkIRect>::Empty();
    }
}
