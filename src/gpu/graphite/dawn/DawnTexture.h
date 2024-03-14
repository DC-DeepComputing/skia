/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnTexture_DEFINED
#define skgpu_graphite_DawnTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/Texture.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skgpu::graphite {
class DawnSampler;
class DawnSharedContext;

class DawnTexture : public Texture {
public:
    static wgpu::Texture MakeDawnTexture(const DawnSharedContext*,
                                         SkISize dimensions,
                                         const TextureInfo&);

    static sk_sp<Texture> Make(const DawnSharedContext*,
                               SkISize dimensions,
                               const TextureInfo&,
                               skgpu::Budgeted);

    static sk_sp<Texture> MakeWrapped(const DawnSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      wgpu::Texture);

    static sk_sp<Texture> MakeWrapped(const DawnSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      const wgpu::TextureView&);

    ~DawnTexture() override {}

    const wgpu::Texture& dawnTexture() const { return fTexture; }
    const wgpu::TextureView& sampleTextureView() const { return fSampleTextureView; }
    const wgpu::TextureView& renderTextureView() const { return fRenderTextureView; }

    // Get or create a bind group containing the given sampler & this texture.
    // Have to make this method const because usually it is called during comamnd enconding
    // and at that point, only const pointer to Texture is provided.
    const wgpu::BindGroup& getOrCreateSingleSamplerBindGroup(const DawnSampler* sampler) const;

private:
    DawnTexture(const DawnSharedContext*,
                SkISize dimensions,
                const TextureInfo&,
                wgpu::Texture,
                wgpu::TextureView sampleTextureView,
                wgpu::TextureView renderTextureView,
                Ownership,
                skgpu::Budgeted);

    void freeGpuData() override;

    static std::pair<wgpu::TextureView, wgpu::TextureView> CreateTextureViews(
            const wgpu::Texture& texture, const TextureInfo& info);

    wgpu::Texture     fTexture;
    wgpu::TextureView fSampleTextureView;
    wgpu::TextureView fRenderTextureView;

    mutable skia_private::THashMap<uint32_t, wgpu::BindGroup> fSingleTextureSamplerBindGroups;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_DawnTexture_DEFINED
