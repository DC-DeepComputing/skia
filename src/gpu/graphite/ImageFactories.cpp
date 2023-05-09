/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkMutex.h"
#include "src/gpu/graphite/Image_Base_Graphite.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Lazy.h"

namespace SkImages {

sk_sp<SkImage> SubsetTextureFrom(skgpu::graphite::Recorder* recorder,
                                 const SkImage* img,
                                 const SkIRect& subset,
                                 SkImage::RequiredProperties props) {
    if (!recorder || !img) {
        return nullptr;
    }
    auto subsetImg = img->makeSubset(recorder, subset, props);
    return SkImages::TextureFromImage(recorder, subsetImg, props);
}

/*
 *  We only have 2 ways to create a Graphite-backed image.
 *
 *  1. Ask the generator to natively create one
 *  2. Ask the generator to return RGB(A) data, which the GPU can convert
 */
static sk_sp<SkImage> make_texture_image_from_lazy(skgpu::graphite::Recorder* recorder,
                                                   const SkImage_Lazy* img,
                                                   SkImage::RequiredProperties requiredProps) {
    // 1. Ask the generator to natively create one.
    {
        // Disable mipmaps here bc Graphite doesn't currently support mipmap regeneration
        // In this case, we would allocate the mipmaps and fill in the base layer but the mipmap
        // levels would never be filled out - yielding incorrect draws. Please see: b/238754357.
        requiredProps.fMipmapped = false;

        auto sharedGenerator = img->generator();
        SkAutoMutexExclusive mutex(sharedGenerator->fMutex);
        sk_sp<SkImage> newImage = sharedGenerator->makeTextureImage(recorder,
                                                                    ib->imageInfo(),
                                                                    requiredProps.fMipmapped);
        if (newImage) {
            SkASSERT(img->isGraphiteBacked());
            return newImage;
        }
    }

    // 2. Ask the generator to return a bitmap, which the GPU can convert.
    if (SkBitmap bitmap; ib->getROPixels(nullptr, &bitmap, CachingHint::kDisallow_CachingHint)) {
        return skgpu::graphite::MakeFromBitmap(recorder,
                                               ib->imageInfo().colorInfo(),
                                               bitmap,
                                               nullptr,
                                               skgpu::Budgeted::kNo,
                                               requiredProps);
    }

    return nullptr;
}


sk_sp<SkImage> TextureFromImage(skgpu::graphite::Recorder* recorder,
                                const SkImage* image,
                                SkImage::RequiredProperties requiredProps) {
    if (!recorder || !image) {
        return nullptr;
    }
    if (image->dimensions().area() <= 1) {
        requiredProps.fMipmapped = false;
    }

    auto ib = as_IB(img);
    SkASSERT(!ib->isGaneshBacked());

    if (ib->isRaster()) {
        return skgpu::graphite::MakeFromBitmap(recorder,
                                               ib->imageInfo().colorInfo(),
                                               ib->bitmap(),
                                               ib->refMips(),
                                               skgpu::Budgeted::kNo,
                                               requiredProps);
    }
    if (ib->isLazyGenerated) {
        return make_texture_image_from_lazy(recorder, static_cast<const SkImage_Lazy*>(ib),
                                            requiredProps);
    }
    SkASSERT(ib->isGraphiteBacked());
    auto ig = static_cast<const skgpu::graphite::Image_Base*>(image);
    return ig->onMakeTextureImage(recorder, requiredProps);
}

} // namespace SkImages
