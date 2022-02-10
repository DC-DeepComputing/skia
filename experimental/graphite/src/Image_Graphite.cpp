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

Image::Image(const SkImageInfo& ii)
    : SkImage_Base(ii, kNeedNewImageUniqueID) {
}

Image::~Image() {}

#if SK_SUPPORT_GPU
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
sk_sp<SkImage> SkImage::makeTextureImage(skgpu::Recorder* recorder,
                                         skgpu::Mipmapped mipmapped,
                                         SkBudgeted budgeted) const {
    if (!recorder) {
        return nullptr;
    }
    if (this->dimensions().area() <= 1) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

//    if (this->isTextureBacked()) {
//        if (!as_IB(this)->context()->priv().matches(dContext)) {
//            return nullptr;
//        }
//
//        if (this->isTextureBacked() && (mipmapped == GrMipmapped::kNo || this->hasMipmaps())) {
//            return sk_ref_sp(const_cast<SkImage*>(this));
//        }
//    }
//    GrImageTexGenPolicy policy = budgeted == SkBudgeted::kYes
//                                         ? GrImageTexGenPolicy::kNew_Uncached_Budgeted
//                                         : GrImageTexGenPolicy::kNew_Uncached_Unbudgeted;
//    // TODO: Don't flatten YUVA images here. Add mips to the planes instead.
//    auto [view, ct] = as_IB(this)->asView(dContext, mipmapped, policy);
//    if (!view) {
//        return nullptr;
//    }
//    SkASSERT(view.asTextureProxy());
//    SkASSERT(mipmapped == GrMipmapped::kNo ||
//             view.asTextureProxy()->mipmapped() == GrMipmapped::kYes);
//    SkColorInfo colorInfo(GrColorTypeToSkColorType(ct), this->alphaType(), this->refColorSpace());
//    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(dContext),
//                                   this->uniqueID(),
//                                   std::move(view),
//                                   std::move(colorInfo));

    return nullptr;
}
#endif


