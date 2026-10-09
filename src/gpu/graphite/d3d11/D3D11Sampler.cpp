/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/d3d11/D3D11Sampler.h"

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTileMode.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/d3d11/D3D11SharedContext.h"

namespace skgpu::graphite {

static D3D11_TEXTURE_ADDRESS_MODE tile_mode_to_d3d_address_mode(SkTileMode tileMode) {
    switch (tileMode) {
        case SkTileMode::kClamp:
            return D3D11_TEXTURE_ADDRESS_CLAMP;
        case SkTileMode::kRepeat:
            return D3D11_TEXTURE_ADDRESS_WRAP;
        case SkTileMode::kMirror:
            return D3D11_TEXTURE_ADDRESS_MIRROR;
        case SkTileMode::kDecal:
            return D3D11_TEXTURE_ADDRESS_BORDER;
    }
    return D3D11_TEXTURE_ADDRESS_CLAMP;
}

sk_sp<D3D11Sampler> D3D11Sampler::Make(const D3D11SharedContext* sharedContext,
                                       const SamplerDesc& desc) {
    D3D11_SAMPLER_DESC samplerDesc = {};
    D3D11_FILTER_TYPE minFilter = (desc.samplingOptions().filter == SkFilterMode::kLinear)
                                          ? D3D11_FILTER_TYPE_LINEAR
                                          : D3D11_FILTER_TYPE_POINT;
    D3D11_FILTER_TYPE magFilter = (desc.samplingOptions().filter == SkFilterMode::kLinear)
                                          ? D3D11_FILTER_TYPE_LINEAR
                                          : D3D11_FILTER_TYPE_POINT;
    D3D11_FILTER_TYPE mipFilter = (desc.samplingOptions().mipmap == SkMipmapMode::kLinear)
                                          ? D3D11_FILTER_TYPE_LINEAR
                                          : D3D11_FILTER_TYPE_POINT;

    samplerDesc.Filter = D3D11_ENCODE_BASIC_FILTER(
            minFilter, magFilter, mipFilter, D3D11_FILTER_REDUCTION_TYPE_STANDARD);

    samplerDesc.AddressU = tile_mode_to_d3d_address_mode(desc.tileModeX());
    samplerDesc.AddressV = tile_mode_to_d3d_address_mode(desc.tileModeY());
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = (desc.samplingOptions().mipmap == SkMipmapMode::kNone)
                                 ? 0.0f
                                 : D3D11_FLOAT32_MAX;
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;

    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
    HRESULT hr = sharedContext->device()->CreateSamplerState(&samplerDesc, &samplerState);
    if (FAILED(hr)) {
        return nullptr;
    }

    return sk_sp<D3D11Sampler>(new D3D11Sampler(sharedContext, std::move(samplerState)));
}

D3D11Sampler::D3D11Sampler(const D3D11SharedContext* sharedContext,
                           Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState)
        : Sampler(sharedContext), fSamplerState(std::move(samplerState)) {}

D3D11Sampler::~D3D11Sampler() = default;

void D3D11Sampler::freeGpuData() {
    fSamplerState.Reset();
}

} // namespace skgpu::graphite
