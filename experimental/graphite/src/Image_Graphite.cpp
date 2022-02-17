/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Image_Graphite.h"

#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/RecorderPriv.h"

#if SK_SUPPORT_GPU
// TODO: move onAsFragmentProcessor off of SkImage_Base and remove this include
#include "src/gpu/GrFragmentProcessor.h"
#endif

namespace skgpu {

Image::Image(Recorder*,
             TextureProxyView view,
             SkColorInfo info)
    : SkImage_Base(SkImageInfo::Make(view.proxy()->dimensions(), std::move(info)),
                   kNeedNewImageUniqueID)
    , fSwizzle(view.swizzle())
    , fOrigin(view.origin()) {
}

Image::~Image() {}

#if SK_SUPPORT_GPU
sk_sp<SkImage> Image::makeTextureImage(Recorder* recorder,
                                       Mipmapped mipmapped,
                                       SkBudgeted budgeted) const {
    if (!recorder) {
        return nullptr;
    }
    if (this->dimensions().area() <= 1) {
        mipmapped = Mipmapped::kNo;
    }

    if (this->isTextureBacked()) {
//        if (!as_IB(this)->context()->priv().matches(dContext)) {
//            return nullptr;
//        }

        if (mipmapped == Mipmapped::kNo || this->hasMipmaps()) {
            const SkImage* image = this;
            return sk_ref_sp(const_cast<SkImage*>(image));
        }
    }
    auto [view, ct] = as_IB(this)->asView(recorder, mipmapped);
    if (!view) {
        return nullptr;
    }
    SkASSERT(view.proxy());
    SkASSERT(mipmapped == Mipmapped::kNo || view.proxy()->mipmapped() == Mipmapped::kYes);
    SkColorInfo colorInfo(ct, this->alphaType(), this->refColorSpace());
    return sk_make_sp<Image>(recorder,
                             std::move(view),
                             std::move(colorInfo));
}

std::unique_ptr<GrFragmentProcessor> Image::onAsFragmentProcessor(
        GrRecordingContext*,
        SkSamplingOptions,
        const SkTileMode[2],
        const SkMatrix&,
        const SkRect* subset,
        const SkRect* domain) const {
    return nullptr;
}
#endif

} // namespace skgpu

#if SK_SUPPORT_GPU
#endif


