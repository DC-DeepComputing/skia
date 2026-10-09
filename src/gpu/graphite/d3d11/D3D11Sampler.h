/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_D3D11Sampler_DEFINED
#define skgpu_graphite_D3D11Sampler_DEFINED

#include "src/gpu/graphite/Sampler.h"

#include <d3d11_1.h>
#include <wrl/client.h>

namespace skgpu::graphite {

class D3D11SharedContext;
struct SamplerDesc;

class D3D11Sampler final : public Sampler {
public:
    static sk_sp<D3D11Sampler> Make(const D3D11SharedContext*, const SamplerDesc&);
    ~D3D11Sampler() override;

    ID3D11SamplerState* d3d11SamplerState() const { return fSamplerState.Get(); }

private:
    D3D11Sampler(const D3D11SharedContext*, Microsoft::WRL::ComPtr<ID3D11SamplerState>);

    void freeGpuData() override;

    Microsoft::WRL::ComPtr<ID3D11SamplerState> fSamplerState;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_D3D11Sampler_DEFINED
