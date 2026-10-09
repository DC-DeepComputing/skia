/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_D3D11CommandBuffer_DEFINED
#define skgpu_graphite_D3D11CommandBuffer_DEFINED

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/d3d11/D3D11GraphicsPipeline.h"

#include <array>
#include <vector>
#include <d3d11_1.h>
#include <wrl/client.h>

namespace skgpu::graphite {

class D3D11Buffer;
class D3D11ResourceProvider;
class D3D11SharedContext;

class D3D11CommandBuffer final : public CommandBuffer {
public:
    struct TextureToBufferCopy {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> fStagingTexture;
        D3D11Buffer* fDstBuffer;
        size_t fBufferOffset;
        size_t fBufferRowBytes;
        uint32_t fWidth;
        uint32_t fHeight;
    };

    static std::unique_ptr<D3D11CommandBuffer> Make(const D3D11SharedContext*,
                                                    D3D11ResourceProvider*);
    ~D3D11CommandBuffer() override;

    ID3D11DeviceContext1* deviceContext() const { return fImmediateDeviceContext.Get(); }

    bool setNewCommandBufferResources() override;
    void processPendingTextureToBufferCopies();

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

    bool addDrawPass(DrawPass*);

    bool bindGraphicsPipeline(const GraphicsPipeline*);
    void bindUniformBuffer(const BindBufferInfo&, UniformSlot);
    void bindInputBuffer(const Buffer*, size_t offset, uint32_t bindingIndex);
    void bindIndexBuffer(const Buffer*, size_t offset);
    void bindIndirectBuffer(const Buffer*, size_t offset);
    void bindTextureAndSamplers(const DrawPass&, const DrawPassCommands::BindTexturesAndSamplers&);
    void setScissor(const Scissor&);
    void setViewport(SkIRect);
    void setBlendConstants(std::array<float, 4>);

    void draw(PrimitiveType, unsigned int baseVertex, unsigned int vertexCount);
    void drawIndexed(PrimitiveType, unsigned int baseIndex, unsigned int indexCount, unsigned int baseVertex);
    void drawInstanced(PrimitiveType, unsigned int baseVertex, unsigned int vertexCount, unsigned int baseInstance, unsigned int instanceCount);
    void drawIndexedInstanced(PrimitiveType, unsigned int baseIndex, unsigned int indexCount, unsigned int baseVertex, unsigned int baseInstance, unsigned int instanceCount);
    void drawIndirect(PrimitiveType);
    void drawIndexedIndirect(PrimitiveType);

    void syncUniformBuffers();
    void syncVertexBuffers();
    const D3D11Sampler* getSampler(const DrawPassCommands::BindTexturesAndSamplers&, int32_t index);

    const D3D11SharedContext* fSharedContext;
    D3D11ResourceProvider* fResourceProvider;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> fImmediateDeviceContext;

    const D3D11GraphicsPipeline* fActiveGraphicsPipeline = nullptr;
    ID3D11Buffer* fCurrentVertexBuffers[2] = {nullptr, nullptr};
    UINT fCurrentVertexOffsets[2] = {0, 0};
    UINT fCurrentVertexStrides[2] = {0, 0};

    ID3D11Buffer* fCurrentIndexBuffer = nullptr;
    UINT fCurrentIndexOffset = 0;

    ID3D11Buffer* fCurrentIndirectBuffer = nullptr;
    UINT fCurrentIndirectBufferOffset = 0;

    std::array<BindBufferInfo, D3D11GraphicsPipeline::kMaxNumUniformBuffers> fBoundUniforms;
    bool fBoundUniformBuffersDirty = false;
    std::array<float, 4> fBlendConstants = {1.0f, 1.0f, 1.0f, 1.0f};

    std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> fIntrinsicConstantBuffers;
    size_t fCurrentIntrinsicBufferIndex = 0;

    std::vector<TextureToBufferCopy> fPendingTextureToBufferCopies;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_D3D11CommandBuffer_DEFINED
