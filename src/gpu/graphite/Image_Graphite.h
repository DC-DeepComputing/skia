/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Image_Graphite_DEFINED
#define skgpu_graphite_Image_Graphite_DEFINED

#include "src/gpu/graphite/Image_Base_Graphite.h"

#include "include/gpu/graphite/Image.h"
#include "src/gpu/graphite/TextureProxyView.h"

namespace skgpu {
    class RefCntedCallback;
}

namespace skgpu::graphite {

class Context;
class Device;
class Recorder;

class Image final : public Image_Base {
public:
    Image(uint32_t uniqueID, TextureProxyView, const SkColorInfo&);
    ~Image() override;

    // Create an Image that wraps the Device and automatically flushes or references the Device's
    // pending tasks when the Image is used in a draw to another canvas.
    static sk_sp<Image> MakeView(sk_sp<Device> device);

    const TextureProxyView& textureProxyView() const { return fTextureProxyView; }

    // Called when the Image is referenced in a draw call to flush pending work if this Image
    // is linked with a Device.
    void notifyUseInDraw(Recorder*);

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kGraphite; }

    bool onHasMipmaps() const override {
        return fTextureProxyView.proxy()->mipmapped() == skgpu::Mipmapped::kYes;
    }

    bool onIsProtected() const override {
        return fTextureProxyView.proxy()->isProtected();
    }

    size_t textureSize() const override;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override;

    sk_sp<SkImage> makeTextureImage(Recorder*, RequiredProperties) const override;

    static sk_sp<TextureProxy> MakePromiseImageLazyProxy(
            const Caps*,
            SkISize dimensions,
            TextureInfo,
            Volatile,
            SkImages::GraphitePromiseImageFulfillProc,
            sk_sp<RefCntedCallback>,
            SkImages::GraphitePromiseTextureReleaseProc);

private:
    sk_sp<SkImage> copyImage(const SkIRect& subset, Recorder*, RequiredProperties) const;
    using Image_Base::onMakeSubset;
    sk_sp<SkImage> onMakeSubset(Recorder*, const SkIRect&, RequiredProperties) const override;
    using Image_Base::onMakeColorTypeAndColorSpace;
    sk_sp<SkImage> makeColorTypeAndColorSpace(Recorder*,
                                              SkColorType targetCT,
                                              sk_sp<SkColorSpace> targetCS,
                                              RequiredProperties) const override;

    TextureProxyView fTextureProxyView;
    // If this Image was created as the read view of a Graphite Surface, this will point to the
    // surface's Device to automatically flush pending work before the image is sampled (within the
    // same Recorder). When the Surface is destroyed or unrefed by the client, the Image becomes the
    // unique holder of this pointer and can remove the Device next time it is sampled. This turns
    // it into a regular immutable image.
    sk_sp<Device> fSurfaceDevice;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Image_Graphite_DEFINED
