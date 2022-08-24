/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/TextureProxyView.h"

//--
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"

namespace skgpu::graphite {

class SpecialSurface_Graphite : public SkSpecialSurface {
public:
    SpecialSurface_Graphite(sk_sp<Device> device, SkIRect subset)
            : SkSpecialSurface(subset, device->surfaceProps()) {
//            , fView(device->readSurfaceView()) {

        {
            Recorder* recorder = device->recorder();
            const skgpu::graphite::Caps* caps = recorder->priv().caps();

            TextureProxy* proxy = device->proxy();
            Swizzle swizzle = caps->getReadSwizzle(device->imageInfo().colorType(),
                                                   proxy->textureInfo());

            fView = TextureProxyView(sk_ref_sp(proxy), swizzle);
        }

        fCanvas = std::make_unique<SkCanvas>(std::move(device));
        fCanvas->clipRect(SkRect::Make(subset));
    }

    sk_sp<SkSpecialImage> onMakeImageSnapshot() override {
        if (!fView.proxy()) {
            return nullptr;
        }

        // Note: SkSpecialImages can only be snapShotted once, so this call is destructive and we
        // move fView.
        return SkSpecialImage::MakeGraphite(fCanvas->recorder(),
                                            this->subset(),
                                            kNeedNewImageUniqueID_SpecialImage,
                                            std::move(fView),
                                            fCanvas->imageInfo().colorInfo(),
                                            this->props());
    }

private:
    TextureProxyView fView;
};

} // namespace skgpu::graphite

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeGraphite(skgpu::graphite::Recorder* recorder,
                                                       const SkImageInfo& ii,
                                                       const SkSurfaceProps& props) {
    if (!recorder) {
        return nullptr;
    }

    sk_sp<skgpu::graphite::Device> device = skgpu::graphite::Device::Make(recorder, ii,
                                                                          SkBudgeted::kYes,
                                                                          props);
    if (!device) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeSize(ii.dimensions());

    return sk_make_sp<skgpu::graphite::SpecialSurface_Graphite>(std::move(device), subset);
}
