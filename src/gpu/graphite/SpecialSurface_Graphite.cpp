/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"

#if SK_GRAPHITE_ENABLED

#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/TextureProxyView.h"

namespace skgpu::graphite {

class SpecialSurface_Graphite : public SkSpecialSurface_Base {
public:
    SpecialSurface_Graphite(sk_sp<Device> device, SkIRect subset)
            : SkSpecialSurface_Base(subset, device->surfaceProps()) {
//            , fView(device->readSurfaceView()) {

        fCanvas = std::make_unique<SkCanvas>(std::move(device));
        fCanvas->clipRect(SkRect::Make(subset));
    }

    sk_sp<SkSpecialImage> onMakeImageSnapshot() override {
        if (!fView.proxy()) {
            return nullptr;
        }

        // Note: SkSpecialImages can only be snapShotted once, so this call is destructive and we
        // move fReadMove.
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
                                                                          SkBudgeted::kYes);
    if (!device) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeSize(ii.dimensions());

    return sk_make_sp<skgpu::graphite::SpecialSurface_Graphite>(std::move(device), subset);
}

#endif // SK_GRAPHITE_ENABLED
