/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/d3d11/D3D11CommandBuffer.h"

#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/d3d11/D3D11Buffer.h"
#include "src/gpu/graphite/d3d11/D3D11ResourceProvider.h"
#include "src/gpu/graphite/d3d11/D3D11SharedContext.h"
#include "src/gpu/graphite/d3d11/D3D11Texture.h"

namespace skgpu::graphite {

std::unique_ptr<D3D11CommandBuffer> D3D11CommandBuffer::Make(
        const D3D11SharedContext* sharedContext,
        D3D11ResourceProvider* resourceProvider) {
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> deferredContext;
    HRESULT hr = sharedContext->device()->CreateDeferredContext1(0, &deferredContext);
    if (FAILED(hr)) {
        return nullptr;
    }

    return std::unique_ptr<D3D11CommandBuffer>(
            new D3D11CommandBuffer(sharedContext, resourceProvider, std::move(deferredContext)));
}

D3D11CommandBuffer::D3D11CommandBuffer(
        const D3D11SharedContext* sharedContext,
        D3D11ResourceProvider* resourceProvider,
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1> deferredContext)
        : CommandBuffer(Protected::kNo)
        , fSharedContext(sharedContext)
        , fResourceProvider(resourceProvider)
        , fDeferredContext(std::move(deferredContext)) {}

D3D11CommandBuffer::~D3D11CommandBuffer() = default;

ResourceProvider* D3D11CommandBuffer::resourceProvider() const {
    return fResourceProvider;
}

bool D3D11CommandBuffer::setNewCommandBufferResources() {
    if (!fDeferredContext) {
        HRESULT hr = fSharedContext->device()->CreateDeferredContext1(0, &fDeferredContext);
        if (FAILED(hr)) {
            return false;
        }
    }
    return true;
}

Microsoft::WRL::ComPtr<ID3D11CommandList> D3D11CommandBuffer::finishEncoding() {
    if (fDeferredContext) {
        fDeferredContext->FinishCommandList(FALSE, &fCommandList);
    }
    return fCommandList;
}

void D3D11CommandBuffer::onResetCommandBuffer() {
    fCommandList.Reset();
    if (fDeferredContext) {
        fDeferredContext->ClearState();
    }
}

bool D3D11CommandBuffer::onAddRenderPass(const RenderPassDesc& renderPassDesc,
                                         const Texture* colorTexture,
                                         const Texture* resolveTexture,
                                         const Texture* depthStencilTexture,
                                         SkIPoint resolveOffset,
                                         SkIRect viewport,
                                         const DrawPassList& drawPasses) {
    if (!fDeferredContext) {
        return false;
    }

    ID3D11RenderTargetView* rtv = nullptr;
    if (colorTexture) {
        rtv = static_cast<const D3D11Texture*>(colorTexture)->renderTargetView();
    }

    ID3D11DepthStencilView* dsv = nullptr;
    if (depthStencilTexture) {
        dsv = static_cast<const D3D11Texture*>(depthStencilTexture)->depthStencilView();
    }

    if (rtv || dsv) {
        fDeferredContext->OMSetRenderTargets(rtv ? 1 : 0, rtv ? &rtv : nullptr, dsv);
    }

    if (rtv && renderPassDesc.fColorAttachment.fLoadOp == LoadOp::kClear) {
        const std::array<float, 4>& c = renderPassDesc.fClearColor;
        fDeferredContext->ClearRenderTargetView(rtv, c.data());
    }

    if (dsv && renderPassDesc.fDepthStencilAttachment.fLoadOp == LoadOp::kClear) {
        UINT clearFlags = 0;
        if (TextureFormatHasDepth(renderPassDesc.fDepthStencilAttachment.fFormat)) {
            clearFlags |= D3D11_CLEAR_DEPTH;
        }
        if (TextureFormatHasStencil(renderPassDesc.fDepthStencilAttachment.fFormat)) {
            clearFlags |= D3D11_CLEAR_STENCIL;
        }
        if (clearFlags != 0) {
            fDeferredContext->ClearDepthStencilView(
                    dsv, clearFlags, renderPassDesc.fClearDepth, 0);
        }
    }

    return true;
}

bool D3D11CommandBuffer::onAddComputePass(DispatchGroupSpan) {
    return false;
}

bool D3D11CommandBuffer::onCopyBufferToBuffer(const Buffer* srcBuffer,
                                             size_t srcOffset,
                                             const Buffer* dstBuffer,
                                             size_t dstOffset,
                                             size_t size) {
    if (!fDeferredContext || !srcBuffer || !dstBuffer || size == 0) {
        return false;
    }

    const auto* srcD3D = static_cast<const D3D11Buffer*>(srcBuffer);
    const auto* dstD3D = static_cast<const D3D11Buffer*>(dstBuffer);

    if (srcD3D->cpuData()) {
        if (!dstD3D->d3d11Buffer()) {
            return false;
        }
        D3D11_BOX box = {
                static_cast<UINT>(dstOffset),
                0,
                0,
                static_cast<UINT>(dstOffset + size),
                1,
                1,
        };
        fDeferredContext->UpdateSubresource1(dstD3D->d3d11Buffer(),
                                             0,
                                             &box,
                                             srcD3D->cpuData() + srcOffset,
                                             0,
                                             0,
                                             D3D11_COPY_NO_OVERWRITE);
        return true;
    }

    if (srcD3D->d3d11Buffer() && dstD3D->d3d11Buffer()) {
        D3D11_BOX srcBox = {
                static_cast<UINT>(srcOffset),
                0,
                0,
                static_cast<UINT>(srcOffset + size),
                1,
                1,
        };
        fDeferredContext->CopySubresourceRegion(dstD3D->d3d11Buffer(),
                                                0,
                                                static_cast<UINT>(dstOffset),
                                                0,
                                                0,
                                                srcD3D->d3d11Buffer(),
                                                0,
                                                &srcBox);
        return true;
    }

    return false;
}

bool D3D11CommandBuffer::onCopyTextureToBuffer(const Texture*,
                                              SkIRect,
                                              const Buffer*,
                                              size_t,
                                              size_t) {
    return true;
}

bool D3D11CommandBuffer::onCopyBufferToTexture(const Buffer*,
                                              const Texture*,
                                              const BufferTextureCopyData*,
                                              int) {
    return true;
}

bool D3D11CommandBuffer::onCopyTextureToTexture(const Texture*,
                                               SkIRect,
                                               const Texture*,
                                               SkIPoint,
                                               int) {
    return true;
}

bool D3D11CommandBuffer::onSynchronizeBufferToCpu(const Buffer*, bool* outDidResultInWork) {
    if (outDidResultInWork) {
        *outDidResultInWork = false;
    }
    return true;
}

bool D3D11CommandBuffer::onClearBuffer(const Buffer*, size_t, size_t) {
    return true;
}

} // namespace skgpu::graphite
