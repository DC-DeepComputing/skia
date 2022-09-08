/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_Graphite.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureUtils.h"

namespace skgpu::graphite {

Image::Image(uint32_t uniqueID,
             TextureProxyView view,
             const SkColorInfo& info)
    : SkImage_Base(SkImageInfo::Make(view.proxy()->dimensions(), info), uniqueID)
    , fTextureProxyView(std::move(view)) {
}

Image::Image(TextureProxyView view,
             const SkColorInfo& info)
    : SkImage_Base(SkImageInfo::Make(view.proxy()->dimensions(), info), kNeedNewImageUniqueID)
    , fTextureProxyView(std::move(view)) {
}

Image::~Image() {}

bool Image::testingOnly_ReadPixels(Context* context,
                                   Recorder* recorder,
                                   const SkImageInfo& dstInfo,
                                   void* dstPixels,
                                   size_t dstRowBytes,
                                   int srcX,
                                   int srcY) {
    return ReadPixelsHelper([recorder]() {
                                recorder->priv().flushTrackedDevices();
                            },
                            context,
                            recorder,
                            fTextureProxyView.proxy(),
                            dstInfo,
                            dstPixels,
                            dstRowBytes,
                            srcX,
                            srcY);
}

sk_sp<SkImage> Image::onMakeTextureImage(Recorder*, RequiredImageProperties requiredProps) const {
    SkASSERT(requiredProps.fMipmapped == Mipmapped::kYes && !this->hasMipmaps());
    // TODO: copy the base layer into a new image that has mip levels
    SKGPU_LOG_W("Graphite does not yet allow explicit mipmap level addition");
    return nullptr;
}

} // namespace skgpu::graphite

namespace {

// We require that the user returns a Graphite-backed image that preserves the dimensions, number
// of channels and alpha type of the original image.
// The bit depth of the individual channels can change (e.g., 4444 -> 8888 is allowed).
// The client can return an image w/ different mipmap settings than were requested but
// the sampling can be impacted.
bool valid_client_provided_image(const SkImage* clientProvided,
                                 const SkImage* original,
                                 SkImage::RequiredImageProperties requiredProps) {
    if (!clientProvided ||
        !as_IB(clientProvided)->isGraphiteBacked() ||
        original->dimensions() != clientProvided->dimensions() ||
        original->alphaType() != clientProvided->alphaType()) {
        return false;
    }

    uint32_t origChannels = SkColorTypeChannelFlags(original->colorType());
    uint32_t clientChannels = SkColorTypeChannelFlags(clientProvided->colorType());
    if (origChannels != clientChannels) {
        return false;
    }

    return true;
}

} // anonymous namespace

sk_sp<SkImage> SkImage::makeTextureImage(skgpu::graphite::Recorder* recorder,
                                         RequiredImageProperties requiredProps) const {
    using namespace skgpu::graphite;

    if (!recorder) {
        return nullptr;
    }
    if (this->dimensions().area() <= 1) {
        requiredProps.fMipmapped = Mipmapped::kNo;
    }

    if (as_IB(this)->isGraphiteBacked()) {
        if (requiredProps.fMipmapped == Mipmapped::kNo || this->hasMipmaps()) {
            const SkImage* image = this;
            return sk_ref_sp(const_cast<SkImage*>(image));
        }
    }

    ImageProvider* clientImageProvider = recorder->clientImageProvider();
    ImageProvider::Key key(this, requiredProps.fMipmapped);

    sk_sp<SkImage> result = clientImageProvider->find(recorder, key, requiredProps);
    if (result && valid_client_provided_image(result.get(), this, requiredProps)) {
        return result;
    }

    result = as_IB(this)->onMakeTextureImage(recorder, requiredProps);
    if (!result) {
        return nullptr;
    }

    clientImageProvider->insert(recorder, key, result);
    return result;
}
