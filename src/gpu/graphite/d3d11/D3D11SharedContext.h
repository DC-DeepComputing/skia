/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_D3D11SharedContext_DEFINED
#define skgpu_graphite_D3D11SharedContext_DEFINED

#include "include/gpu/graphite/d3d11/D3D11BackendContext.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/ThreadSafeResourceProvider.h"
#include "src/gpu/graphite/d3d11/D3D11Caps.h"

#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <wrl/client.h>

namespace skgpu::graphite {

class D3D11SharedContext final : public SharedContext {
public:
    static sk_sp<SharedContext> Make(const D3D11BackendContext&, const ContextOptions&);
    ~D3D11SharedContext() override;

    const D3D11Caps* d3d11Caps() const { return static_cast<const D3D11Caps*>(this->caps()); }
    ID3D11Device1* device() const { return fDevice.Get(); }
    ID3D11DeviceContext1* deviceContext() const { return fDeviceContext.Get(); }

    std::unique_ptr<ResourceProvider> makeResourceProvider(
            SingleOwner*, uint32_t recorderID, size_t resourceBudget) override;

private:
    D3D11SharedContext(const D3D11BackendContext&,
                       std::unique_ptr<const D3D11Caps>,
                       SkExecutor* executor,
                       SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects);

    sk_sp<GraphicsPipeline> createGraphicsPipeline(
            const RuntimeEffectDictionary*,
            const UniqueKey&,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>,
            uint32_t compilationID) override;

    Microsoft::WRL::ComPtr<IDXGIAdapter1> fAdapter;
    Microsoft::WRL::ComPtr<ID3D11Device1> fDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> fDeviceContext;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_D3D11SharedContext_DEFINED
