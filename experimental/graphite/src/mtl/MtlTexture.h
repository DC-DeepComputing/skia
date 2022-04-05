/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlTexture_DEFINED
#define skgpu_graphite_MtlTexture_DEFINED

#include "experimental/graphite/src/Texture.h"
#include "include/core/SkRefCnt.h"

#import <Metal/Metal.h>

namespace skgpu {
namespace mtl {
class Gpu;
}
}

namespace skgpu::graphite {

class MtlTexture : public skgpu::Texture {
public:
    static sk_cfp<id<MTLTexture>> MakeMtlTexture(const mtl::Gpu*,
                                                 SkISize dimensions,
                                                 const skgpu::TextureInfo&);

    static sk_sp<Texture> Make(const mtl::Gpu*,
                               SkISize dimensions,
                               const skgpu::TextureInfo&);

    static sk_sp<Texture> MakeWrapped(const mtl::Gpu*,
                                      SkISize dimensions,
                                      const skgpu::TextureInfo&,
                                      sk_cfp<id<MTLTexture>>);

    ~MtlTexture() override {}

    id<MTLTexture> mtlTexture() const { return fTexture.get(); }

private:
    MtlTexture(const Gpu* gpu,
               SkISize dimensions,
               const skgpu::TextureInfo& info,
               sk_cfp<id<MTLTexture>>,
               Ownership);

    void freeGpuData() override;

    sk_cfp<id<MTLTexture>> fTexture;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_MtlTexture_DEFINED
