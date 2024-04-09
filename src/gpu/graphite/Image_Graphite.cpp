/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_Graphite.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/task/CopyTask.h"

#if defined(GRAPHITE_TEST_UTILS)
#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/ContextPriv.h"
#endif

namespace skgpu::graphite {

Image::Image(uint32_t uniqueID,
             TextureProxyView view,
             const SkColorInfo& info)
        : Image_Base(SkImageInfo::Make(view.proxy()->dimensions(), info), uniqueID)
        , fTextureProxyView(std::move(view)) {
    SkASSERT(fTextureProxyView.proxy());
}

Image::~Image() = default;

sk_sp<Image> Image::Make(TextureProxyView view, const SkColorInfo& info) {
    if (!view) {
        return nullptr;
    }
    // Graphite doesn't use SkImage's unique ID for caching so always request a new one.
    return sk_sp<Image>(new Image(kNeedNewImageUniqueID, std::move(view), info));
}

sk_sp<Image> Image::MakeView(sk_sp<Device> device) {
    if (!device) {
        return nullptr;
    }
    // NOTE: If the device was created with an approx backing fit, it's SkImageInfo reports the
    // logical dimensions, but its proxy has the approximate fit. These larger dimensions are
    // propagated to the SkImageInfo of this image view.
    sk_sp<Image> view = Make(device->readSurfaceView(), device->imageInfo().colorInfo());
    if (view) {
        view->linkDevice(std::move(device));
    }
    return view;
}

sk_sp<Image> Image::MakeCopy(Recorder* recorder,
                             const TextureProxyView& srcView,
                             const SkColorInfo& srcColorInfo,
                             const SkIRect& subset,
                             Budgeted budgeted,
                             Mipmapped mipmapped,
                             SkBackingFit backingFit) {
    SkASSERT(!(mipmapped == Mipmapped::kYes && backingFit == SkBackingFit::kApprox));
    if (!srcView) {
        return nullptr;
    }

    SkASSERT(srcView.proxy()->isFullyLazy() ||
             SkIRect::MakeSize(srcView.proxy()->dimensions()).contains(subset));

    if (!recorder->priv().caps()->supportsReadPixels(srcView.proxy()->textureInfo())) {
        if (!recorder->priv().caps()->isTexturable(srcView.proxy()->textureInfo())) {
            // The texture is not blittable nor texturable so copying cannot be done.
            return nullptr;
        }
        // Copy-as-draw
        sk_sp<Image> srcImage = Image::Make(srcView, srcColorInfo);
        return CopyAsDraw(recorder, srcImage.get(), subset, srcColorInfo,
                          budgeted, mipmapped, backingFit);
    }


    skgpu::graphite::TextureInfo textureInfo =
            recorder->priv().caps()->getTextureInfoForSampledCopy(srcView.proxy()->textureInfo(),
                                                                  mipmapped);

    sk_sp<TextureProxy> dst = TextureProxy::Make(
            recorder->priv().caps(),
            backingFit == SkBackingFit::kApprox ? GetApproxSize(subset.size()) : subset.size(),
            textureInfo,
            budgeted);
    if (!dst) {
        return nullptr;
    }

    auto copyTask = CopyTextureToTextureTask::Make(srcView.refProxy(), subset, dst, {0, 0});
    if (!copyTask) {
        return nullptr;
    }

    recorder->priv().add(std::move(copyTask));

    if (mipmapped == Mipmapped::kYes) {
        if (!GenerateMipmaps(recorder, dst, srcColorInfo)) {
            SKGPU_LOG_W("Image::MakeCopy failed to generate mipmaps");
            return nullptr;
        }
    }

    return Image::Make({std::move(dst), srcView.swizzle()}, srcColorInfo);
}

size_t Image::textureSize() const {
    if (!fTextureProxyView.proxy()->texture()) {
        return fTextureProxyView.proxy()->uninstantiatedGpuMemorySize();
    }

    return fTextureProxyView.proxy()->texture()->gpuMemorySize();
}

sk_sp<Image> Image::copyImage(Recorder* recorder,
                              const SkIRect& subset,
                              Budgeted budgeted,
                              Mipmapped mipmapped,
                              SkBackingFit backingFit) const {
    this->notifyInUse(recorder);
    return Image::MakeCopy(recorder, fTextureProxyView, this->imageInfo().colorInfo(),
                           subset, budgeted, mipmapped, backingFit);
}

sk_sp<SkImage> Image::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    sk_sp<Image> view = Make(fTextureProxyView,
                             this->imageInfo().colorInfo().makeColorSpace(std::move(newCS)));
    // The new Image object shares the same texture proxy, so it should also share linked Devices
    view->linkDevices(this);
    return view;
}

#if defined(GRAPHITE_TEST_UTILS)
bool Image::onReadPixelsGraphite(Recorder* recorder,
                                 const SkPixmap& dst,
                                 int srcX,
                                 int srcY) const {
    if (Context* context = recorder->priv().context()) {
        // Add all previous commands generated to the command buffer.
        // If the client snaps later they'll only get post-read commands in their Recording,
        // but since they're doing a readPixels in the middle that shouldn't be unexpected.
        std::unique_ptr<Recording> recording = recorder->snap();
        if (!recording) {
            return false;
        }
        InsertRecordingInfo info;
        info.fRecording = recording.get();
        if (!context->insertRecording(info)) {
            return false;
        }
        return context->priv().readPixels(dst,
                                          fTextureProxyView.proxy(),
                                          this->imageInfo(),
                                          srcX,
                                          srcY);
    }
    return false;
}
#endif

} // namespace skgpu::graphite
