/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_BackendTexture_DEFINED
#define skgpu_graphite_BackendTexture_DEFINED

#include "include/core/SkSize.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/TextureInfo.h"

#ifdef SK_DAWN
#include "include/gpu/graphite/dawn/DawnTypes.h"
#endif

#ifdef SK_METAL
#include "include/gpu/graphite/mtl/MtlTypes.h"
#endif

#ifdef SK_VULKAN
#include "include/private/gpu/vk/SkiaVulkan.h"
#endif

namespace skgpu::graphite {

class BackendTexture {
public:
    BackendTexture() {}
#ifdef SK_DAWN
    // Create a BackendTexture from a wgpu::Texture. Texture info will be
    // queried from the texture. Comparing to wgpu::TextureView,
    // SkImage::readPixels(), SkSurface::readPixels() and
    // SkSurface::writePixels() are implemented by direct buffer copy. They
    // should be more efficient. For wgpu::TextureView, those methods will use
    // create an intermediate wgpu::Texture, and use it to transfer pixels.
    BackendTexture(wgpu::Texture texture);
    // Create a BackendTexture from a wgpu::TextureView. Texture dimensions and
    // info have to be provided.
    BackendTexture(SkISize dimensions,
                   const DawnTextureInfo& info,
                   wgpu::TextureView textureView);
#endif
#ifdef SK_METAL
    // The BackendTexture will not call retain or release on the passed in MtlHandle. Thus the
    // client must keep the MtlHandle valid until they are no longer using the BackendTexture.
    BackendTexture(SkISize dimensions, MtlHandle mtlTexture);
#endif

    BackendTexture(const BackendTexture&);

    ~BackendTexture();

    BackendTexture& operator=(const BackendTexture&);

    bool operator==(const BackendTexture&) const;
    bool operator!=(const BackendTexture& that) const { return !(*this == that); }

    bool isValid() const { return fInfo.isValid(); }
    BackendApi backend() const { return fInfo.backend(); }

    SkISize dimensions() const { return fDimensions; }

    const TextureInfo& info() const { return fInfo; }

#ifdef SK_DAWN
    wgpu::Texture getDawnTexture() const;
    wgpu::TextureView getDawnTextureView() const;
#endif
#ifdef SK_METAL
    MtlHandle getMtlTexture() const;
#endif

private:
    SkISize fDimensions;
    TextureInfo fInfo;

    union {
#ifdef SK_DAWN
        struct {
            wgpu::Texture fDawnTexture;
            wgpu::TextureView fDawnTextureView;
        };
#endif
#ifdef SK_METAL
        MtlHandle fMtlTexture;
#endif
#ifdef SK_VULKAN
        VkImage fVkImage;
#endif
    };
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_BackendTexture_DEFINED

