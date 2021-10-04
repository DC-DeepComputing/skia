/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlTexture_DEFINED
#define skgpu_MtlTexture_DEFINED

#include "experimental/graphite/src/Texture.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class Gpu;

class Texture : public skgpu::Texture {
public:
    static sk_sp<Texture> MakeDepthStencil(Gpu* gpu,
                                           SkISize dimensions,
                                           UsageFlags usage, // Must only be depth and/or stencil
                                           int sampleCnt,
                                           MTLPixelFormat format);

    ~Texture() override {}

    id<MTLTexture> mtlTexture() const { return fTexture; }

private:
    static sk_sp<Texture> Make(Gpu* gpu,
                               SkISize dimensions,
                               UsageFlags usages,
                               int sampleCnt,
                               MTLPixelFormat format,
                               uint32_t mipLevels,
                               int mtlTextureUsage,
                               int mtlStorageMode);

    Texture(SkISize dimensions,
            const skgpu::TextureInfo& info,
            UsageFlags supportedUsages,
            id<MTLTexture> texture);

    id<MTLTexture> fTexture;
};

} // namepsace skgpu::mtl

#endif // skgpu_MtlTexture_DEFINED
