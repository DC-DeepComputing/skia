/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_D3D11Buffer_DEFINED
#define skgpu_graphite_D3D11Buffer_DEFINED

#include "src/gpu/graphite/Buffer.h"

#include <d3d11_1.h>
#include <wrl/client.h>
#include <memory>

namespace skgpu::graphite {

class D3D11SharedContext;

class D3D11Buffer final : public Buffer {
public:
    static sk_sp<D3D11Buffer> Make(const D3D11SharedContext*,
                                   size_t size,
                                   BufferType,
                                   AccessPattern,
                                   std::string_view label);

    ID3D11Buffer* d3d11Buffer() const { return fD3D11Buffer.Get(); }
    const uint8_t* cpuData() const { return fCpuData.get(); }
    uint8_t* cpuData() { return fCpuData.get(); }
    BufferType bufferType() const { return fBufferType; }

    void setStagingTexture(Microsoft::WRL::ComPtr<ID3D11Texture2D> tex) {
        fStagingTexture = std::move(tex);
    }
    ID3D11Texture2D* stagingTexture() const { return fStagingTexture.Get(); }

private:
    D3D11Buffer(const D3D11SharedContext*,
                size_t size,
                BufferType,
                AccessPattern,
                std::string_view label,
                Microsoft::WRL::ComPtr<ID3D11Buffer> buffer,
                std::unique_ptr<uint8_t[]> cpuData);

    void onMap() override;
    void onUnmap() override;
    void freeGpuData() override;

    const D3D11SharedContext* d3d11SharedContext() const;

    BufferType fBufferType;
    Microsoft::WRL::ComPtr<ID3D11Buffer> fD3D11Buffer;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> fStagingTexture;
    std::unique_ptr<uint8_t[]> fCpuData;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_D3D11Buffer_DEFINED
