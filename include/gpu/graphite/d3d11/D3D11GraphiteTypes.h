/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_D3D11GraphiteTypes_DEFINED
#define skgpu_graphite_D3D11GraphiteTypes_DEFINED

#include "include/core/SkSize.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/SkAPI.h"

#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <wrl/client.h>

namespace skgpu::graphite {

class BackendTexture;

class SK_API D3D11TextureInfo final : public TextureInfo::Data {
public:
    DXGI_FORMAT fFormat = DXGI_FORMAT_UNKNOWN;
    DXGI_FORMAT fViewFormat = DXGI_FORMAT_UNKNOWN;
    UINT fBindFlags = 0;
    UINT fCPUAccessFlags = 0;
    UINT fMiscFlags = 0;
    D3D11_USAGE fUsage = D3D11_USAGE_DEFAULT;

    D3D11TextureInfo() = default;

    D3D11TextureInfo(SampleCount sampleCount,
                     skgpu::Mipmapped mipmapped,
                     DXGI_FORMAT format,
                     UINT bindFlags,
                     D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
                     UINT cpuAccessFlags = 0,
                     UINT miscFlags = 0)
            : Data(sampleCount, mipmapped)
            , fFormat(format)
            , fViewFormat(format)
            , fBindFlags(bindFlags)
            , fCPUAccessFlags(cpuAccessFlags)
            , fMiscFlags(miscFlags)
            , fUsage(usage) {}

    D3D11TextureInfo(SampleCount sampleCount,
                     skgpu::Mipmapped mipmapped,
                     DXGI_FORMAT format,
                     DXGI_FORMAT viewFormat,
                     UINT bindFlags,
                     D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
                     UINT cpuAccessFlags = 0,
                     UINT miscFlags = 0)
            : Data(sampleCount, mipmapped)
            , fFormat(format)
            , fViewFormat(viewFormat)
            , fBindFlags(bindFlags)
            , fCPUAccessFlags(cpuAccessFlags)
            , fMiscFlags(miscFlags)
            , fUsage(usage) {}

    DXGI_FORMAT getViewFormat() const {
        return fViewFormat != DXGI_FORMAT_UNKNOWN ? fViewFormat : fFormat;
    }

private:
    friend class TextureInfo;
    friend class TextureInfoPriv;

    static constexpr skgpu::BackendApi kBackend = skgpu::BackendApi::kD3D11;

    Protected isProtected() const { return Protected::kNo; }
    TextureFormat viewFormat() const;

    SkString toBackendString() const override;

    void copyTo(TextureInfo::AnyTextureInfoData& dstData) const override {
        dstData.emplace<D3D11TextureInfo>(*this);
    }
    bool isCompatible(const TextureInfo& that, bool requireExact) const override;
};

namespace TextureInfos {
SK_API TextureInfo MakeD3D11(const D3D11TextureInfo& d3d11Info);
SK_API bool GetD3D11TextureInfo(const TextureInfo&, D3D11TextureInfo*);
}  // namespace TextureInfos

namespace BackendTextures {
SK_API BackendTexture MakeD3D11(SkISize dimensions,
                                const D3D11TextureInfo& info,
                                ID3D11Texture2D* texture);
SK_API ID3D11Texture2D* GetD3D11Texture2D(const BackendTexture&);
}  // namespace BackendTextures

}  // namespace skgpu::graphite

#endif // skgpu_graphite_D3D11GraphiteTypes_DEFINED
