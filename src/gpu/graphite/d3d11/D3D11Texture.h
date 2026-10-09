/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_D3D11Texture_DEFINED
#define skgpu_graphite_D3D11Texture_DEFINED

#include "src/gpu/graphite/Texture.h"

#include <d3d11_1.h>
#include <wrl/client.h>

namespace skgpu::graphite {

class D3D11SharedContext;

class D3D11Texture final : public Texture {
public:
    static sk_sp<D3D11Texture> Make(const D3D11SharedContext*,
                                    SkISize dimensions,
                                    const TextureInfo&,
                                    std::string_view label);

    static sk_sp<D3D11Texture> MakeWrapped(const D3D11SharedContext*,
                                           SkISize dimensions,
                                           const TextureInfo&,
                                           Microsoft::WRL::ComPtr<ID3D11Texture2D>,
                                           std::string_view label);

    ~D3D11Texture() override;

    ID3D11Texture2D* d3d11Texture() const { return fTexture.Get(); }
    ID3D11ShaderResourceView* shaderResourceView() const { return fSRV.Get(); }
    ID3D11RenderTargetView* renderTargetView() const { return fRTV.Get(); }
    ID3D11DepthStencilView* depthStencilView() const { return fDSV.Get(); }

private:
    D3D11Texture(const D3D11SharedContext*,
                 SkISize dimensions,
                 const TextureInfo&,
                 Microsoft::WRL::ComPtr<ID3D11Texture2D>,
                 Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>,
                 Microsoft::WRL::ComPtr<ID3D11RenderTargetView>,
                 Microsoft::WRL::ComPtr<ID3D11DepthStencilView>,
                 Ownership,
                 std::string_view label);

    void freeGpuData() override;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> fTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> fSRV;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> fRTV;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> fDSV;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_D3D11Texture_DEFINED
