/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/SpecialImage_Graphite.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "src/core/SkSpecialImage.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/TextureUtils.h"

namespace skgpu::graphite {

class SpecialImage final : public SkSpecialImage {
public:
    SpecialImage(const SkIRect& subset, sk_sp<SkImage> image, const SkSurfaceProps& props)
            : SkSpecialImage(subset, image->uniqueID(), image->imageInfo().colorInfo(), props)
            , fImage(std::move(image)) {
        SkASSERT(as_IB(fImage)->isGraphiteBacked());
    }

    size_t getSize() const override {
        return fImage->textureSize();
    }

    bool isGraphiteBacked() const override { return true; }

    TextureProxyView textureProxyView() const {
        if (as_IB(fImage)->type() == SkImage_Base::Type::kGraphite) {
            return static_cast<const Image*>(fImage.get())->textureProxyView();
        } else {
            // A YUV[A] graphite-backed image cannot be represented as a single texture proxy view.
            return {};
        }
    }

    SkISize backingStoreDimensions() const override {
        return fImage->dimensions();
    }

    sk_sp<SkSpecialImage> onMakeBackingStoreSubset(const SkIRect& subset) const override {
        SkASSERT(fImage->bounds().contains(subset));
        return sk_make_sp<skgpu::graphite::SpecialImage>(subset, fImage, this->props());
    }

    sk_sp<SkImage> asImage() const override { return fImage; }

private:
    // TODO(b/299474380): SkSpecialImage is intended to go away in favor of just using SkImages
    // and tracking the intended srcRect explicitly in skif::FilterResult. Since Graphite tracks
    // device-linked textures via Images, the graphite special image just wraps an image.
    sk_sp<SkImage> fImage;
};

} // namespace skgpu::graphite

namespace SkSpecialImages {

sk_sp<SkSpecialImage> MakeGraphite(skgpu::graphite::Recorder* recorder,
                                   const SkIRect& subset,
                                   sk_sp<SkImage> image,
                                   const SkSurfaceProps& props) {
    if (!recorder || !image || subset.isEmpty()) {
        return nullptr;
    }

    SkASSERT(image->bounds().contains(subset));

    // This will work even if the image is a raster-backed image and the Recorder's
    // client ImageProvider does a valid upload.
    if (!as_IB(image)->isGraphiteBacked()) {
        auto [graphiteImage, _] =
                skgpu::graphite::GetGraphiteBacked(recorder, image.get(), {});
        if (!graphiteImage) {
            return nullptr;
        }

        image = graphiteImage;
    }

    return sk_make_sp<skgpu::graphite::SpecialImage>(subset, std::move(image), props);
}

}  // namespace SkSpecialImages
