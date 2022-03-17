/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Image_Graphite.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/include/Recording.h"
#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CopyTask.h"
#include "experimental/graphite/src/RecorderPriv.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/Texture.h"

namespace skgpu {

Image::Image(TextureProxyView view,
             const SkColorInfo& info)
    : SkImage_Base(SkImageInfo::Make(view.proxy()->dimensions(), info),
                   kNeedNewImageUniqueID)
    , fTextureProxyView(std::move(view)) {
}

Image::~Image() {}

bool Image::onReadPixels(Context* context,
                         Recorder* recorder,
                         const SkImageInfo& dstInfo,
                         void* dstPixels,
                         size_t dstRowBytes,
                         int srcX,
                         int srcY) {
    // TODO: Support more formats that we can read back into
    if (dstInfo.colorType() != kRGBA_8888_SkColorType) {
        return false;
    }

    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();

    TextureProxy* srcProxy = fTextureProxyView.proxy();
    if (!srcProxy->instantiate(resourceProvider)) {
        return false;
    }
    sk_sp<Texture> srcTexture = srcProxy->refTexture();
    SkASSERT(srcTexture);

    size_t size = dstRowBytes * dstInfo.height();
    sk_sp<Buffer> dstBuffer = resourceProvider->findOrCreateBuffer(size,
                                                                   BufferType::kXferGpuToCpu,
                                                                   PrioritizeGpuReads::kNo);
    if (!dstBuffer) {
        return false;
    }

    SkIRect srcRect = SkIRect::MakeXYWH(srcX, srcY, dstInfo.width(), dstInfo.height());
    sk_sp<CopyTextureToBufferTask> task =
            CopyTextureToBufferTask::Make(std::move(srcTexture),
                                          srcRect,
                                          dstBuffer,
                                          /*bufferOffset=*/0,
                                          dstRowBytes);
    if (!task) {
        return false;
    }

    recorder->priv().flushTrackedDevices();
    recorder->priv().add(std::move(task));

    // TODO: Can snapping ever fail?
    context->insertRecording(recorder->snap());
    context->submit(SyncToCpu::kYes);

    void* mappedMemory = dstBuffer->map();

    memcpy(dstPixels, mappedMemory, size);

    return true;

}

std::tuple<TextureProxyView, SkColorType> Image::onAsView(Recorder*,
                                                          Mipmapped mipmapped,
                                                          SkBudgeted) const {
    if (fTextureProxyView.proxy()->mipmapped() != mipmapped) {
        // We will not generate miplevels
        return {};
    }
    SkColorType ct = this->colorType();
    return {fTextureProxyView, ct};
}

} // namespace skgpu

sk_sp<SkImage> SkImage::makeTextureImage(skgpu::Recorder* recorder,
                                         skgpu::Mipmapped mipmapped,
                                         SkBudgeted budgeted) const {
    if (!recorder) {
        return nullptr;
    }
    if (this->dimensions().area() <= 1) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

    if (as_IB(this)->isGraphiteBacked()) {
        if (mipmapped == skgpu::Mipmapped::kNo || this->hasMipmaps()) {
            const SkImage* image = this;
            return sk_ref_sp(const_cast<SkImage*>(image));
        }
    }
    auto [view, ct] = as_IB(this)->asView(recorder, mipmapped, budgeted);
    if (!view) {
        return nullptr;
    }
    SkASSERT(view.proxy());
    SkASSERT(mipmapped == skgpu::Mipmapped::kNo ||
             view.proxy()->mipmapped() == skgpu::Mipmapped::kYes);
    SkColorInfo colorInfo(ct, this->alphaType(), this->refColorSpace());
    return sk_make_sp<skgpu::Image>(std::move(view),
                                    std::move(colorInfo));
}



