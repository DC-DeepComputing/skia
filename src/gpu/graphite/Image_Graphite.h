/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Image_Graphite_DEFINED
#define skgpu_graphite_Image_Graphite_DEFINED

#include "src/gpu/graphite/Image_GraphiteBase.h"

#include "src/gpu/graphite/TextureProxyView.h"

namespace skgpu::graphite {

class Context;
class Recorder;

class Image final : public ImageBase {
public:
    Image(uint32_t uniqueID, TextureProxyView, const SkColorInfo&);
    Image(TextureProxyView, const SkColorInfo&);
    ~Image() override;

    bool onHasMipmaps() const override {
        return fTextureProxyView.proxy()->mipmapped() == Mipmapped::kYes;
    }

    using ImageBase::onMakeSubset;
    sk_sp<SkImage> onMakeSubset(const SkIRect&,
                                Recorder*,
                                RequiredImageProperties) const override;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override;

    TextureProxyView textureProxyView() const { return fTextureProxyView; }

private:
    sk_sp<SkImage> onMakeTextureImage(Recorder*, RequiredImageProperties) const override;

    TextureProxyView fTextureProxyView;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Image_Graphite_DEFINED
