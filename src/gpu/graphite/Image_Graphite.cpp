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
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"

namespace skgpu::graphite {

Image::Image(uint32_t uniqueID,
             TextureProxyView view,
             const SkColorInfo& info)
    : Image_Base(SkImageInfo::Make(view.proxy()->dimensions(), info), uniqueID)
    , fTextureProxyView(std::move(view))
    , fSurfaceDevice(nullptr) {}

Image::~Image() {}

sk_sp<Image> Image::MakeView(sk_sp<Device> device) {
    TextureProxyView proxy = device->readSurfaceView();
    if (!proxy) {
        return nullptr;
    }
    sk_sp<Image> view = sk_make_sp<Image>(kNeedNewImageUniqueID,
                                          std::move(proxy),
                                          device->imageInfo().colorInfo());
    view->fSurfaceDevice = std::move(device);
    return view;
}

// FIXME: notifyInUse() rename? copies stemming from image sourc'ed textures ought to call this too
void Image::notifyUseInDraw(Recorder* recorder) {
    // Images, including those that wrap a Device or Surface, can be used in multiple Recorders
    // so this function must be thread safe. Hold a local ref on any attached surface device to
    // prevent it from being destroyed by another thread when the Image uniquely owns it but happens
    // to be being drawn to two Recorders simultaneously.
    // FIXME: Do we need to take a mutex on thi simage in order to read fSurfaceDevice in the first place?

    /**
     * So in theory, there are many references held on the Image object, and so notifyUse() gets
     * invoked from many threads simultaneously. But `fSurfaceDevice` is not necessarily held by
     * anything other than this image.
     *
     * while the ref adding on the Device object is atomic and thread safe, the access of the
     * fSurfaceDevice sk_sp here is not, so we can have one thread get() the pointer to the Device
     * and then get stalled, while another thread gets the pointer, processes through this function
     * and sees that the surface device is uniquely held (since the local ref in thread A hasn't
     * actuall been added yet). It deletes the surface and returns. A comes back and adds a ref
     * to deleted memory, womp womp)
     *
     *
    */
    sk_sp<Device> linkedDevice = fSurfaceDevice;
    if (!linkedDevice) {
        return; // nothing to flush
    }

    SkASSERT(!linkedDevice->unique()); // at minimum Image and 'linkedDevice' hold a ref.
    // Automatic flushing of image views only happens when mixing reads and writes on the
    // originating Recorder. Draws of the view on another Recorder will always see the texture
    // content dependent on how Recordings are inserted.
    if (linkedDevice->recorder() == recorder) {
        linkedDevice->flushPendingWorkToRecorder();
    }
    linkedDevice.reset();

    if (fSurfaceDevice->unique()) {
        // The Surface has been destroyed, so the linked device can't have any more pending work
        // added to it that might need to be flushed before drawing the image view.
        fSurfaceDevice.reset();
    }

    // So if Image were holding a weak-reference around the Device, what this would look like
    // instead is:
    // -> really what we want is for it to be WeakRef<Device> and every image that has it adds a
    // weak_ref() to the Device object. If try_ref() fails, the device "deleted" and the image drops
    // its weak ref. Eventually all the Images detect this and the weak references hit 0 and the
    // device is finally destroyed. This is valuable for the case where we create multiple image views
    // of the same device. However, it still suffers from the same problem that we have in the originla
    // code where there's no thread safety on the IMage itself accessing its pointer.
    //  -> could we actually delete the entire Device in weak_dispose()?
    //  -> maybe, if the weak reference count was held in the wrapper around Device
    //     so that those could still be accessed safely. Then some thread somewhere would get the
    //     last strong unref and destroy the device.
    //   all referencing images would have try_ref() fail safely so work is skipped.
    //     but we have the dangling wrapper that doesn't get cleaned up unitl the weak refs hit 0
    //     and if we try doing multithreaded weak unrefing via the same Image, we get into troubel again...
    // really want an atomic way to update a pointer and a counter at teh same time.

    // obviously for now a mutex is the simplex way to resolve all of these issues.
    // could probably do it with some relaxed/acquire loading of an atomic if that's faster than a mutex?
    //
    // but we do need to think about these image views from other images
    //   -> Surface caching an image view is fine
    //   -> if we had thread safe weak refs, I suppose we could not cache the image view and then
    //      not worry about unlinked the device -> already not worrying about that with this uniqueness business though.
    //   -> but makeWithCOlorSpace or whatever variant that shares the same texture data needs to be
    //      linked to the same device. And if that's called ever with strong references, we will keep
    //      the device alive until one of the images goes away.
    if (fSurfaceDevice->try_ref()) {
        // We have a strong reference now, so it won't be deleted on us
        fSurfaceDevice->flushPendingWorkToRecorder();
        fSurfaceDevice->unref(); // we need unref to signal if it was the last one...

        // Because ugh, we could have held multiple strong refs for the flushing (or at leas tto
        // keep the device alive and access it), which means then we don't have any guarantee about
        // how many threads are inside this conditional block and they all could take the weak_expired()
        // branch.
        if (fSurfaceDevice->weak_expired()) {

        }
    } else {
        // Didn't get the strong ref, presumably some other thread acquired it and then saw they
        // were unique and then deleted things, so don't do anythign anymore
        // Next time through should see that it's null
    }

    //
}

size_t Image::textureSize() const {
    if (!fTextureProxyView.proxy()) {
        return 0;
    }

    if (!fTextureProxyView.proxy()->texture()) {
        return fTextureProxyView.proxy()->uninstantiatedGpuMemorySize();
    }

    return fTextureProxyView.proxy()->texture()->gpuMemorySize();
}

sk_sp<SkImage> Image::onMakeSubset(Recorder* recorder,
                                   const SkIRect& subset,
                                   RequiredProperties requiredProps) const {
    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());

    // optimization : return self if the subset == our bounds and requirements met
    if (bounds == subset && (!requiredProps.fMipmapped || this->hasMipmaps())) {
        const SkImage* image = this;
        return sk_ref_sp(const_cast<SkImage*>(image));
    }

    return this->copyImage(subset, recorder, requiredProps);
}

sk_sp<SkImage> Image::makeTextureImage(Recorder* recorder, RequiredProperties requiredProps) const {
    if (!requiredProps.fMipmapped || this->hasMipmaps()) {
        const SkImage* image = this;
        return sk_ref_sp(const_cast<SkImage*>(image));
    }

    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());
    return this->copyImage(bounds, recorder, requiredProps);
}

sk_sp<SkImage> Image::copyImage(const SkIRect& subset,
                                Recorder* recorder,
                                RequiredProperties requiredProps) const {
    const TextureProxyView& srcView = this->textureProxyView();
    if (!srcView) {
        return nullptr;
    }
    this->notifyUseInDraw(recorder); // FIXME: const'ness, should we just make it mutable?

    auto mm = requiredProps.fMipmapped ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
    TextureProxyView copiedView = TextureProxyView::Copy(
            recorder, this->imageInfo().colorInfo(), srcView, subset, mm, SkBackingFit::kExact);
    if (!copiedView) {
        return nullptr;
    }

    return sk_sp<Image>(new Image(kNeedNewImageUniqueID,
                                  std::move(copiedView),
                                  this->imageInfo().colorInfo()));
}

sk_sp<SkImage> Image::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    return sk_make_sp<Image>(kNeedNewImageUniqueID,
                             fTextureProxyView,
                             this->imageInfo().colorInfo().makeColorSpace(std::move(newCS)));
}

sk_sp<SkImage> Image::makeColorTypeAndColorSpace(Recorder* recorder,
                                                 SkColorType targetCT,
                                                 sk_sp<SkColorSpace> targetCS,
                                                 RequiredProperties requiredProps) const {
    SkAlphaType at = (this->alphaType() == kOpaque_SkAlphaType) ? kPremul_SkAlphaType
                                                                : this->alphaType();

    SkImageInfo ii = SkImageInfo::Make(this->dimensions(), targetCT, at, std::move(targetCS));

    auto mm = requiredProps.fMipmapped ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
    sk_sp<SkSurface> s = SkSurfaces::RenderTarget(recorder, ii, mm);
    if (!s) {
        return nullptr;
    }

    s->getCanvas()->drawImage(this, 0, 0);
    return SkSurfaces::AsImage(s);
}

} // namespace skgpu::graphite

using namespace skgpu::graphite;
using SkImages::GraphitePromiseImageFulfillProc;
using SkImages::GraphitePromiseTextureReleaseProc;

sk_sp<TextureProxy> Image::MakePromiseImageLazyProxy(
        const Caps* caps,
        SkISize dimensions,
        TextureInfo textureInfo,
        Volatile isVolatile,
        GraphitePromiseImageFulfillProc fulfillProc,
        sk_sp<skgpu::RefCntedCallback> releaseHelper,
        GraphitePromiseTextureReleaseProc textureReleaseProc) {
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(releaseHelper);

    if (!fulfillProc) {
        return nullptr;
    }

    /**
     * This class is the lazy instantiation callback for promise images. It manages calling the
     * client's Fulfill, ImageRelease, and TextureRelease procs.
     */
    class PromiseLazyInstantiateCallback {
    public:
        PromiseLazyInstantiateCallback(GraphitePromiseImageFulfillProc fulfillProc,
                                       sk_sp<skgpu::RefCntedCallback> releaseHelper,
                                       GraphitePromiseTextureReleaseProc textureReleaseProc)
                : fFulfillProc(fulfillProc)
                , fReleaseHelper(std::move(releaseHelper))
                , fTextureReleaseProc(textureReleaseProc) {
        }
        PromiseLazyInstantiateCallback(PromiseLazyInstantiateCallback&&) = default;
        PromiseLazyInstantiateCallback(const PromiseLazyInstantiateCallback&) {
            // Because we get wrapped in std::function we must be copyable. But we should never
            // be copied.
            SkASSERT(false);
        }
        PromiseLazyInstantiateCallback& operator=(PromiseLazyInstantiateCallback&&) = default;
        PromiseLazyInstantiateCallback& operator=(const PromiseLazyInstantiateCallback&) {
            SkASSERT(false);
            return *this;
        }

        sk_sp<Texture> operator()(ResourceProvider* resourceProvider) {

            auto [ backendTexture, textureReleaseCtx ] = fFulfillProc(fReleaseHelper->context());
            if (!backendTexture.isValid()) {
                SKGPU_LOG_W("FulFill Proc failed");
                return nullptr;
            }

            sk_sp<RefCntedCallback> textureReleaseCB = RefCntedCallback::Make(fTextureReleaseProc,
                                                                              textureReleaseCtx);

            sk_sp<Texture> texture = resourceProvider->createWrappedTexture(backendTexture);
            if (!texture) {
                SKGPU_LOG_W("Texture creation failed");
                return nullptr;
            }

            texture->setReleaseCallback(std::move(textureReleaseCB));
            return texture;
        }

    private:
        GraphitePromiseImageFulfillProc fFulfillProc;
        sk_sp<skgpu::RefCntedCallback> fReleaseHelper;
        GraphitePromiseTextureReleaseProc fTextureReleaseProc;

    } callback(fulfillProc, std::move(releaseHelper), textureReleaseProc);

    return TextureProxy::MakeLazy(caps,
                                  dimensions,
                                  textureInfo,
                                  skgpu::Budgeted::kNo,  // This is destined for a user's SkImage
                                  isVolatile,
                                  std::move(callback));
}
