/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/d3d11/D3D11Texture.h"

#include "src/core/SkMathPriv.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/MutableTextureState.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/d3d11/D3D11SharedContext.h"

#include <algorithm>

namespace skgpu::graphite {

static uint32_t num_mip_levels(SkISize dimensions) {
    return 32 - SkCLZ(std::max(dimensions.width(), dimensions.height()));
}

sk_sp<D3D11Texture> D3D11Texture::Make(const D3D11SharedContext* sharedContext,
                                       SkISize dimensions,
                                       const TextureInfo& info,
                                       std::string_view label) {
    const auto& d3dInfo = TextureInfoPriv::Get<D3D11TextureInfo>(info);

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = dimensions.width();
    desc.Height = dimensions.height();
    desc.MipLevels = (info.mipmapped() == Mipmapped::kYes) ? num_mip_levels(dimensions) : 1;
    desc.ArraySize = 1;
    desc.Format = d3dInfo.fFormat;
    desc.SampleDesc.Count = static_cast<UINT>(info.sampleCount());
    desc.SampleDesc.Quality = 0;
    desc.Usage = d3dInfo.fUsage;
    desc.BindFlags = d3dInfo.fBindFlags;
    desc.CPUAccessFlags = d3dInfo.fCPUAccessFlags;
    desc.MiscFlags = d3dInfo.fMiscFlags;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = sharedContext->device()->CreateTexture2D(&desc, nullptr, &texture);
    if (FAILED(hr)) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    if (d3dInfo.fBindFlags & D3D11_BIND_SHADER_RESOURCE) {
        sharedContext->device()->CreateShaderResourceView(texture.Get(), nullptr, &srv);
    }

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
    if (d3dInfo.fBindFlags & D3D11_BIND_RENDER_TARGET) {
        sharedContext->device()->CreateRenderTargetView(texture.Get(), nullptr, &rtv);
    }

    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;
    if (d3dInfo.fBindFlags & D3D11_BIND_DEPTH_STENCIL) {
        sharedContext->device()->CreateDepthStencilView(texture.Get(), nullptr, &dsv);
    }

    return sk_sp<D3D11Texture>(new D3D11Texture(sharedContext,
                                               dimensions,
                                               info,
                                               std::move(texture),
                                               std::move(srv),
                                               std::move(rtv),
                                               std::move(dsv),
                                               Ownership::kOwned,
                                               label));
}

sk_sp<D3D11Texture> D3D11Texture::MakeWrapped(const D3D11SharedContext* sharedContext,
                                              SkISize dimensions,
                                              const TextureInfo& info,
                                              Microsoft::WRL::ComPtr<ID3D11Texture2D> texture,
                                              std::string_view label) {
    if (!texture) {
        return nullptr;
    }

    const auto& d3dInfo = TextureInfoPriv::Get<D3D11TextureInfo>(info);

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    if (d3dInfo.fBindFlags & D3D11_BIND_SHADER_RESOURCE) {
        sharedContext->device()->CreateShaderResourceView(texture.Get(), nullptr, &srv);
    }

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
    if (d3dInfo.fBindFlags & D3D11_BIND_RENDER_TARGET) {
        sharedContext->device()->CreateRenderTargetView(texture.Get(), nullptr, &rtv);
    }

    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;
    if (d3dInfo.fBindFlags & D3D11_BIND_DEPTH_STENCIL) {
        sharedContext->device()->CreateDepthStencilView(texture.Get(), nullptr, &dsv);
    }

    return sk_sp<D3D11Texture>(new D3D11Texture(sharedContext,
                                               dimensions,
                                               info,
                                               std::move(texture),
                                               std::move(srv),
                                               std::move(rtv),
                                               std::move(dsv),
                                               Ownership::kWrapped,
                                               label));
}

D3D11Texture::D3D11Texture(const D3D11SharedContext* sharedContext,
                           SkISize dimensions,
                           const TextureInfo& info,
                           Microsoft::WRL::ComPtr<ID3D11Texture2D> texture,
                           Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv,
                           Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv,
                           Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv,
                           Ownership ownership,
                           std::string_view label)
        : Texture(sharedContext,
                  dimensions,
                  info,
                  /*isTransient=*/false,
                  /*mutableState=*/nullptr,
                  ownership,
                  label)
        , fTexture(std::move(texture))
        , fSRV(std::move(srv))
        , fRTV(std::move(rtv))
        , fDSV(std::move(dsv)) {}

D3D11Texture::~D3D11Texture() = default;

void D3D11Texture::freeGpuData() {
    fTexture.Reset();
    fSRV.Reset();
    fRTV.Reset();
    fDSV.Reset();
}

} // namespace skgpu::graphite
