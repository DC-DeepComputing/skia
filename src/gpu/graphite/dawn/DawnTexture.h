/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnTexture_DEFINED
#define skgpu_graphite_DawnTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skgpu::graphite {
class DawnSharedContext;

class DawnTexture : public Texture {
public:
    static wgpu::Texture MakeDawnTexture(const DawnSharedContext*,
                                         SkISize dimensions,
                                         const TextureInfo&);

    // Create and return a pair of renderView, sampleView.
    static std::pair<wgpu::TextureView, wgpu::TextureView> MakeDawnTextureViews(
            const wgpu::Texture& texture, const TextureInfo& info);

    static sk_sp<Texture> Make(const DawnSharedContext*,
                               SkISize dimensions,
                               const TextureInfo&);

    static sk_sp<Texture> MakeWrapped(const DawnSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      wgpu::Texture,
                                      const wgpu::TextureView& renderView,
                                      const wgpu::TextureView& sampleView);

    static sk_sp<Texture> MakeWrapped(const DawnSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      const wgpu::TextureView& renderView,
                                      const wgpu::TextureView& sampleView);

    ~DawnTexture() override {}

    const wgpu::Texture& dawnTexture() const { return fTexture; }
    const wgpu::TextureView& sampleTextureView() const { return fSampleTextureView; }
    const wgpu::TextureView& renderTextureView() const { return fRenderTextureView; }

    const DawnTextureInfo& dawnTextureInfo() const {
        return TextureInfoPriv::Get<DawnTextureInfo>(this->textureInfo());
    }

private:
    DawnTexture(const DawnSharedContext*,
                SkISize dimensions,
                const TextureInfo&,
                wgpu::Texture,
                wgpu::TextureView renderTextureView,
                wgpu::TextureView sampleTextureView,
                Ownership);

    void freeGpuData() override;

    void setBackendLabel(char const* label) override;

    wgpu::Texture     fTexture;
    wgpu::TextureView fRenderTextureView;
    wgpu::TextureView fSampleTextureView;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_DawnTexture_DEFINED
