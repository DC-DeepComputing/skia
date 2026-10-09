/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_D3D11CommandBuffer_DEFINED
#define skgpu_graphite_D3D11CommandBuffer_DEFINED

#include "src/gpu/graphite/CommandBuffer.h"

#include <d3d11_1.h>
#include <wrl/client.h>

namespace skgpu::graphite {

class D3D11ResourceProvider;
class D3D11SharedContext;

class D3D11CommandBuffer final : public CommandBuffer {
public:
    static std::unique_ptr<D3D11CommandBuffer> Make(const D3D11SharedContext*,
                                                    D3D11ResourceProvider*);
    ~D3D11CommandBuffer() override;

    Microsoft::WRL::ComPtr<ID3D11CommandList> finishEncoding();
    ID3D11DeviceContext1* deferredContext() const { return fDeferredContext.Get(); }

    bool setNewCommandBufferResources() override;

private:
    D3D11CommandBuffer(const D3D11SharedContext*,
                       D3D11ResourceProvider*,
                       Microsoft::WRL::ComPtr<ID3D11DeviceContext1>);

    ResourceProvider* resourceProvider() const override;

    void onResetCommandBuffer() override;

    bool onAddRenderPass(const RenderPassDesc&,
                         const Texture* colorTexture,
                         const Texture* resolveTexture,
                         const Texture* depthStencilTexture,
                         SkIPoint resolveOffset,
                         SkIRect viewport,
                         const DrawPassList& drawPasses) override;

    bool onAddComputePass(DispatchGroupSpan dispatchGroups) override;

    bool onCopyBufferToBuffer(const Buffer* srcBuffer,
                              size_t srcOffset,
                              const Buffer* dstBuffer,
                              size_t dstOffset,
                              size_t size) override;

    bool onCopyTextureToBuffer(const Texture*,
                               SkIRect srcRect,
                               const Buffer*,
                               size_t bufferOffset,
                               size_t bufferRowBytes) override;

    bool onCopyBufferToTexture(const Buffer*,
                               const Texture*,
                               const BufferTextureCopyData*,
                               int count) override;

    bool onCopyTextureToTexture(const Texture* src,
                                SkIRect srcRect,
                                const Texture* dst,
                                SkIPoint dstPoint,
                                int mipLevel) override;

    bool onSynchronizeBufferToCpu(const Buffer*, bool* outDidResultInWork) override;
    bool onClearBuffer(const Buffer*, size_t offset, size_t size) override;

    const D3D11SharedContext* fSharedContext;
    D3D11ResourceProvider* fResourceProvider;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> fDeferredContext;
    Microsoft::WRL::ComPtr<ID3D11CommandList> fCommandList;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_D3D11CommandBuffer_DEFINED
