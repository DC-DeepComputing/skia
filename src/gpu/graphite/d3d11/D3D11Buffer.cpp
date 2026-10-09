/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/d3d11/D3D11Buffer.h"

#include "include/private/SkAlign.h"
#include "src/gpu/graphite/d3d11/D3D11SharedContext.h"

namespace skgpu::graphite {

sk_sp<D3D11Buffer> D3D11Buffer::Make(const D3D11SharedContext* sharedContext,
                                     size_t size,
                                     BufferType type,
                                     AccessPattern accessPattern,
                                     std::string_view label) {
    if (type == BufferType::kXferCpuToGpu) {
        auto cpuData = std::make_unique<uint8_t[]>(std::max(size, static_cast<size_t>(1)));
        return sk_sp<D3D11Buffer>(new D3D11Buffer(sharedContext,
                                                 size,
                                                 type,
                                                 accessPattern,
                                                 label,
                                                 nullptr,
                                                 std::move(cpuData)));
    }

    if (type == BufferType::kXferGpuToCpu) {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = static_cast<UINT>(
                SkAlignTo(std::max(size, static_cast<size_t>(16)), static_cast<size_t>(16)));
        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        HRESULT hr = sharedContext->device()->CreateBuffer(&desc, nullptr, &buffer);
        if (FAILED(hr)) {
            return nullptr;
        }

        return sk_sp<D3D11Buffer>(new D3D11Buffer(sharedContext,
                                                 size,
                                                 type,
                                                 accessPattern,
                                                 label,
                                                 std::move(buffer),
                                                 nullptr));
    }

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = static_cast<UINT>(
            SkAlignTo(std::max(size, static_cast<size_t>(16)), static_cast<size_t>(16)));
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;

    switch (type) {
        case BufferType::kVertex:
        case BufferType::kVertexStorage:
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            break;
        case BufferType::kIndex:
        case BufferType::kIndexStorage:
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            break;
        case BufferType::kUniform:
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            break;
        case BufferType::kStorage:
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
            desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
            break;
        case BufferType::kIndirect:
            desc.BindFlags = 0;
            desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
            break;
        default:
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            break;
    }

    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
    HRESULT hr = sharedContext->device()->CreateBuffer(&desc, nullptr, &buffer);
    if (FAILED(hr)) {
        SkDebugf("D3D11 CreateBuffer failed for type %d (size=%zu, ByteWidth=%u, Bind=0x%x, Misc=0x%x): hr=0x%08x\n",
                 static_cast<int>(type), size, desc.ByteWidth, desc.BindFlags, desc.MiscFlags, static_cast<unsigned int>(hr));
        return nullptr;
    }

    return sk_sp<D3D11Buffer>(new D3D11Buffer(sharedContext,
                                             size,
                                             type,
                                             accessPattern,
                                             label,
                                             std::move(buffer),
                                             nullptr));
}

D3D11Buffer::D3D11Buffer(const D3D11SharedContext* sharedContext,
                         size_t size,
                         BufferType type,
                         AccessPattern accessPattern,
                         std::string_view label,
                         Microsoft::WRL::ComPtr<ID3D11Buffer> buffer,
                         std::unique_ptr<uint8_t[]> cpuData)
        : Buffer(sharedContext, size, Protected::kNo, label)
        , fBufferType(type)
        , fD3D11Buffer(std::move(buffer))
        , fCpuData(std::move(cpuData)) {}

const D3D11SharedContext* D3D11Buffer::d3d11SharedContext() const {
    return static_cast<const D3D11SharedContext*>(this->sharedContext());
}

void D3D11Buffer::onMap() {
    if (fCpuData) {
        fMapPtr = fCpuData.get();
        return;
    }

    if (fStagingTexture) {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        HRESULT hr = d3d11SharedContext()->deviceContext()->Map(
                fStagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
        if (SUCCEEDED(hr)) {
            fMapPtr = mapped.pData;
        }
        return;
    }

    if (fD3D11Buffer) {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        HRESULT hr = d3d11SharedContext()->deviceContext()->Map(
                fD3D11Buffer.Get(), 0, D3D11_MAP_READ, 0, &mapped);
        if (SUCCEEDED(hr)) {
            fMapPtr = mapped.pData;
        }
    }
}

void D3D11Buffer::onUnmap() {
    if (fCpuData) {
        fMapPtr = nullptr;
        return;
    }

    if (fStagingTexture) {
        d3d11SharedContext()->deviceContext()->Unmap(fStagingTexture.Get(), 0);
        fMapPtr = nullptr;
        return;
    }

    if (fD3D11Buffer) {
        d3d11SharedContext()->deviceContext()->Unmap(fD3D11Buffer.Get(), 0);
        fMapPtr = nullptr;
    }
}

void D3D11Buffer::freeGpuData() {
    fD3D11Buffer.Reset();
    fStagingTexture.Reset();
    fCpuData.reset();
}

} // namespace skgpu::graphite
