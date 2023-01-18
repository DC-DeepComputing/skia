/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Image_YUVA_Graphite_DEFINED
#define skgpu_graphite_Image_YUVA_Graphite_DEFINED

#include "src/gpu/graphite/Image_Base_Graphite.h"

namespace skgpu::graphite {

class Recorder;

class Image_YUVA final : public Image_Base {
public:
    ~Image_YUVA() override {}

    bool isYUVA() const override { return true; }

    bool onHasMipmaps() const override {
        return false;
    }

    using Image_Base::onMakeSubset;
    sk_sp<SkImage> onMakeSubset(const SkIRect&, Recorder*, RequiredImageProperties) const override {
        return nullptr;
    }

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override {
        return nullptr;
    }

private:
    sk_sp<SkImage> onMakeTextureImage(Recorder*, RequiredImageProperties) const override {
        return nullptr;
    }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Image_YUVA_Graphite_DEFINED
