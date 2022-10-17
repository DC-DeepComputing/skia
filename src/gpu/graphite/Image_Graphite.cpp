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
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureUtils.h"

#if SK_SUPPORT_GPU
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#endif

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

sk_sp<SkImage> Image::onMakeColorTypeAndColorSpace(SkColorType,
                                                   sk_sp<SkColorSpace>,
                                                   GrDirectContext*) const {
    return nullptr;
}

sk_sp<SkImage> Image::onReinterpretColorSpace(sk_sp<SkColorSpace>) const {
    return nullptr;
}

void Image::onAsyncReadPixels(const SkImageInfo& info,
                              SkIRect srcRect,
                              ReadPixelsCallback callback,
                              ReadPixelsContext context) const {
    // TODO
    callback(context, nullptr);
}

void Image::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                        SkIRect srcRect,
                                        RescaleGamma rescaleGamma,
                                        RescaleMode rescaleMode,
                                        ReadPixelsCallback callback,
                                        ReadPixelsContext context) const {
    // TODO
    callback(context, nullptr);
}

void Image::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                              sk_sp<SkColorSpace> dstColorSpace,
                                              const SkIRect srcRect,
                                              const SkISize dstSize,
                                              RescaleGamma rescaleGamma,
                                              RescaleMode rescaleMode,
                                              ReadPixelsCallback callback,
                                              ReadPixelsContext context) const {
    // TODO
    callback(context, nullptr);
}

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

sk_sp<SkImage> Image::onMakeTextureImage(Recorder*, RequiredImageProperties requiredProps) const {
    SkASSERT(requiredProps.fMipmapped == Mipmapped::kYes && !this->hasMipmaps());
    // TODO: copy the base layer into a new image that has mip levels. For now we just return
    // the un-mipmapped version and allow the sampling to be downgraded to linear
    SKGPU_LOG_W("Graphite does not yet allow explicit mipmap level addition");
    return sk_ref_sp(this);
}

} // namespace skgpu::graphite

using namespace skgpu::graphite;

namespace {

bool validate_backend_texture(const Caps* caps,
                              const BackendTexture& texture,
                              const SkColorInfo& info) {
    if (!texture.isValid() ||
        texture.dimensions().width() <= 0 ||
        texture.dimensions().height() <= 0) {
        return false;
    }

    if (!SkColorInfoIsValid(info)) {
        return false;
    }

    if (!caps->isTexturable(texture.info())) {
        return false;
    }

    return caps->areColorTypeAndTextureInfoCompatible(info.colorType(), texture.info());
}

} // anonymous namespace

using namespace skgpu::graphite;

sk_sp<SkImage> SkImage::makeTextureImage(Recorder* recorder,
                                         RequiredImageProperties requiredProps) const {
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
    return as_IB(this)->onMakeTextureImage(recorder, requiredProps);
}

sk_sp<TextureProxy> Image::MakePromiseImageLazyProxy(Recorder* recorder,
                                                     SkISize dimensions,
                                                     TextureInfo textureInfo,
                                                     Volatile isVolatile,
                                                     GraphitePromiseImageFulfillProc fulfillProc,
                                                     GraphitePromiseImageReleaseProc releaseProc,
                                                     GraphitePromiseImageFulfillContext fulfillContext) {
    SkASSERT(!dimensions.isEmpty());

    if (!fulfillProc) {
        return nullptr;
    }

    /**
     * This class is the lazy instantiation callback for promise images. It manages calling the
     * client's Fulfill, Release, and Done procs. It attempts to reuse a GrTexture instance in
     * cases where the client provides the same SkPromiseImageTexture as Fulfill results for
     * multiple SkImages. The created GrTexture is given a key based on a unique ID associated with
     * the SkPromiseImageTexture.
     *
     * A key invalidation message is installed on the SkPromiseImageTexture so that the GrTexture
     * is deleted once it can no longer be used to instantiate a proxy.
     */
    class PromiseLazyInstantiateCallback {
    public:
        PromiseLazyInstantiateCallback(GraphitePromiseImageFulfillProc fulfillProc,
                                       GraphitePromiseImageReleaseProc releaseProc,
                                       GraphitePromiseImageFulfillContext fulfillContext)
                : fFulfillProc(fulfillProc)
                , fReleaseProc(releaseProc)
                , fFulfillContext(fulfillContext) {
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

        sk_sp<Texture> operator()(ResourceProvider* resourceProvider,
                                  const TextureProxy::LazySurfaceDesc& desc) {

            if (fCalledFulfilledProc && desc.fIsVolatile == Volatile::kNo) {
                // In the non-volatile case we should only try to fulfill multiple times if the
                // first call failed.
                SkASSERT(fFulfillProcFailed);
                return nullptr;
            }

            fCalledFulfilledProc = true;
            auto [ backendTexture, releaseContext ] = fFulfillProc(fFulfillContext);
            if (!backendTexture.isValid()) {
                fFulfillProcFailed = true;
                return nullptr;
            }

            sk_sp<RefCntedCallback> releaseCB = RefCntedCallback::Make(fReleaseProc,
                                                                       releaseContext);

            sk_sp<Texture> texture = resourceProvider->createWrappedTexture(backendTexture);
            if (!texture) {
                return nullptr;
            }

            texture->setReleaseCallback(std::move(releaseCB));

            SkDebugf("-- creating texture ");
            texture->printRefs();
            SkDebugf("release CB %p %d\n",
                     texture->fReleaseCallback.get(),
                     texture->fReleaseCallback->getRefCnt());

            return texture;
        }

    private:
        GraphitePromiseImageFulfillProc fFulfillProc;
        GraphitePromiseImageReleaseProc fReleaseProc;
        GraphitePromiseImageFulfillContext fFulfillContext;
        bool fCalledFulfilledProc = false;
        bool fFulfillProcFailed = false;

    } callback(fulfillProc, releaseProc, fulfillContext);

    return TextureProxy::MakeLazy(dimensions,
                                  textureInfo,
                                  SkBudgeted::kNo,     // This is destined for a user's SkImage
                                  isVolatile,
                                  std::move(callback));
}

sk_sp<SkImage> SkImage::MakeGraphitePromiseTexture(Recorder* recorder,
                                                   SkISize dimensions,
                                                   const TextureInfo& textureInfo,
                                                   const SkColorInfo& colorInfo,
                                                   Volatile isVolatile,
                                                   GraphitePromiseImageFulfillProc fulfillProc,
                                                   GraphitePromiseImageReleaseProc releaseProc,
                                                   GraphitePromiseImageFulfillContext fulfillContext) {

    // Our contract is that we will always call the release proc even on failure.
//    releaseProc = releaseProc ? releaseProc : [](void*) {};
//    auto releaseHelper = skgpu::RefCntedCallback::Make(releaseProc, promiseContext);

    if (!recorder) {
        SKGPU_LOG_W("Null Recorder");
        return nullptr;
    }

    const Caps* caps = recorder->priv().caps();

    SkImageInfo info = SkImageInfo::Make(dimensions, colorInfo);
    if (!SkImageInfoIsValid(info)) {
        SKGPU_LOG_W("Invalid SkImageInfo");
        return nullptr;
    }

    if (!caps->areColorTypeAndTextureInfoCompatible(colorInfo.colorType(), textureInfo)) {
        SKGPU_LOG_W("Incompatible SkColorType and TextureInfo");
        return nullptr;
    }

    sk_sp<TextureProxy> proxy = Image::MakePromiseImageLazyProxy(recorder,
                                                                 dimensions,
                                                                 textureInfo,
                                                                 isVolatile,
                                                                 fulfillProc,
                                                                 releaseProc,
                                                                 fulfillContext);
    if (!proxy) {
        return nullptr;
    }

    skgpu::Swizzle swizzle = caps->getReadSwizzle(colorInfo.colorType(), textureInfo);
    TextureProxyView view(std::move(proxy), swizzle);
    return sk_make_sp<Image>(view, colorInfo);
}

sk_sp<SkImage> SkImage::MakeGraphiteFromBackendTexture(Recorder* recorder,
                                                       const BackendTexture& backendTex,
                                                       SkColorType ct,
                                                       SkAlphaType at,
                                                       sk_sp<SkColorSpace> cs) {
    if (!recorder) {
        return nullptr;
    }

    const Caps* caps = recorder->priv().caps();

    SkColorInfo info(ct, at, std::move(cs));

    if (!validate_backend_texture(caps, backendTex, info)) {
        return nullptr;
    }

    sk_sp<Texture> texture = recorder->priv().resourceProvider()->createWrappedTexture(backendTex);
    if (!texture) {
        return nullptr;
    }

    sk_sp<TextureProxy> proxy(new TextureProxy(std::move(texture)));

    skgpu::Swizzle swizzle = caps->getReadSwizzle(ct, backendTex.info());
    TextureProxyView view(std::move(proxy), swizzle);
    return sk_make_sp<Image>(view, info);
}

skgpu::graphite::TextureProxy* SkImage::proxy() const {
    return nullptr;
}
