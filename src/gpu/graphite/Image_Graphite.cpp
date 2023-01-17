/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_Graphite.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureUtils.h"

namespace skgpu::graphite {

Image::Image(uint32_t uniqueID,
             TextureProxyView view,
             const SkColorInfo& info)
    : ImageBase(SkImageInfo::Make(view.proxy()->dimensions(), info), uniqueID)
    , fTextureProxyView(std::move(view)) {
}

Image::Image(TextureProxyView view,
             const SkColorInfo& info)
    : ImageBase(SkImageInfo::Make(view.proxy()->dimensions(), info), kNeedNewImageUniqueID)
    , fTextureProxyView(std::move(view)) {
}

Image::~Image() {}

sk_sp<SkImage> Image::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    return sk_make_sp<Image>(kNeedNewImageUniqueID,
                             fTextureProxyView,
                             this->imageInfo().colorInfo().makeColorSpace(std::move(newCS)));
}

sk_sp<SkImage> Image::onMakeTextureImage(Recorder* recorder,
                                         RequiredImageProperties requiredProps) const {
    SkASSERT(requiredProps.fMipmapped == Mipmapped::kYes && !this->hasMipmaps());

    TextureProxyView srcView = this->textureProxyView();
    if (!srcView) {
        return nullptr;
    }

    const SkIRect bounds = SkIRect::MakeSize(this->imageInfo().dimensions());
    TextureProxyView copiedView = TextureProxyView::Copy(recorder,
                                                         this->imageInfo().colorInfo(),
                                                         srcView,
                                                         bounds,
                                                         requiredProps.fMipmapped);
    if (!copiedView) {
        return nullptr;
    }

    return sk_sp<Image>(new Image(kNeedNewImageUniqueID,
                                  std::move(copiedView),
                                  this->imageInfo().colorInfo()));
}

} // namespace skgpu::graphite
