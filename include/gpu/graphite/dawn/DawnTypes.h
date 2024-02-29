/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnTypes_DEFINED
#define skgpu_graphite_DawnTypes_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"
#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skgpu::graphite {

struct DawnTextureInfo {
    uint32_t fSampleCount = 1;
    Mipmapped fMipmapped = Mipmapped::kNo;

    // wgpu::TextureDescriptor properties
    // `fFormat` for multiplanar formats corresponds to the plane TextureView's format.
    wgpu::TextureFormat fFormat = wgpu::TextureFormat::Undefined;
    // For non-multiplaner textures leave this as Undefined. For multiplaner formats set this to
    // the full multiplaner format.
    wgpu::TextureFormat fFullYUVFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureUsage fUsage = wgpu::TextureUsage::None;
    // TODO(b/308944094): Migrate aspect information to BackendTextureViews.
    wgpu::TextureAspect fAspect = wgpu::TextureAspect::All;

    DawnTextureInfo() = default;
    DawnTextureInfo(const wgpu::Texture& texture);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnTypes_DEFINED


