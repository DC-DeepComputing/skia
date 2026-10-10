/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/d3d11/D3D11CommandBuffer.h"

#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/d3d11/D3D11Buffer.h"
#include "src/gpu/graphite/d3d11/D3D11Caps.h"
#include "src/gpu/graphite/d3d11/D3D11GraphicsPipeline.h"
#include "src/gpu/graphite/d3d11/D3D11ResourceProvider.h"
#include "src/gpu/graphite/d3d11/D3D11Sampler.h"
#include "src/gpu/graphite/d3d11/D3D11SharedContext.h"
#include "src/gpu/graphite/d3d11/D3D11Texture.h"

#include <algorithm>

namespace skgpu::graphite {

std::unique_ptr<D3D11CommandBuffer> D3D11CommandBuffer::Make(
        const D3D11SharedContext* sharedContext,
        D3D11ResourceProvider* resourceProvider) {
    if (!sharedContext || !sharedContext->deviceContext()) {
        return nullptr;
    }

    return std::unique_ptr<D3D11CommandBuffer>(
            new D3D11CommandBuffer(sharedContext, resourceProvider, sharedContext->deviceContext()));
}

D3D11CommandBuffer::D3D11CommandBuffer(
        const D3D11SharedContext* sharedContext,
        D3D11ResourceProvider* resourceProvider,
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1> immediateContext)
        : CommandBuffer(Protected::kNo)
        , fSharedContext(sharedContext)
        , fResourceProvider(resourceProvider)
        , fImmediateDeviceContext(std::move(immediateContext)) {}

D3D11CommandBuffer::~D3D11CommandBuffer() = default;

ResourceProvider* D3D11CommandBuffer::resourceProvider() const {
    return fResourceProvider;
}

bool D3D11CommandBuffer::setNewCommandBufferResources() {
    return true;
}

void D3D11CommandBuffer::onResetCommandBuffer() {
    if (fImmediateDeviceContext) {
        fImmediateDeviceContext->ClearState();
    }
    fActiveGraphicsPipeline = nullptr;
    fCurrentVertexBuffers[0] = nullptr;
    fCurrentVertexBuffers[1] = nullptr;
    fCurrentVertexOffsets[0] = 0;
    fCurrentVertexOffsets[1] = 0;
    fCurrentVertexStrides[0] = 0;
    fCurrentVertexStrides[1] = 0;
    fCurrentIndexBuffer = nullptr;
    fCurrentIndexOffset = 0;
    fCurrentIndirectBuffer = nullptr;
    fCurrentIndirectBufferOffset = 0;

    for (auto& slot : fBoundUniforms) {
        slot = {};
    }
    fBoundUniformBuffersDirty = false;
    fBlendConstants = {1.0f, 1.0f, 1.0f, 1.0f};
    fCurrentIntrinsicBufferIndex = 0;
    fPendingTextureToBufferCopies.clear();
}

void D3D11CommandBuffer::processPendingTextureToBufferCopies() {
    if (!fImmediateDeviceContext || fPendingTextureToBufferCopies.empty()) {
        return;
    }
    for (const auto& copy : fPendingTextureToBufferCopies) {
        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT hr = fImmediateDeviceContext->Map(copy.fStagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
        if (SUCCEEDED(hr)) {
            if (copy.fDstBuffer && copy.fDstBuffer->cpuData()) {
                const uint8_t* srcRow = static_cast<const uint8_t*>(mapped.pData);
                uint8_t* dstRow = copy.fDstBuffer->cpuData() + copy.fBufferOffset;
                size_t copyBytesPerRow = std::min<size_t>(copy.fBufferRowBytes, mapped.RowPitch);
                for (uint32_t y = 0; y < copy.fHeight; ++y) {
                    memcpy(dstRow, srcRow, copyBytesPerRow);
                    srcRow += mapped.RowPitch;
                    dstRow += copy.fBufferRowBytes;
                }
            }
            fImmediateDeviceContext->Unmap(copy.fStagingTexture.Get(), 0);
        }
    }
    fPendingTextureToBufferCopies.clear();
}

bool D3D11CommandBuffer::onAddRenderPass(const RenderPassDesc& renderPassDesc,
                                         const Texture* colorTexture,
                                         const Texture* resolveTexture,
                                         const Texture* depthStencilTexture,
                                         SkIPoint resolveOffset,
                                         SkIRect viewport,
                                         const DrawPassList& drawPasses) {
    if (!fImmediateDeviceContext) {
        return false;
    }

    if (!SkIRect::Intersects(viewport, fRenderTargetBounds)) SK_UNLIKELY {
        return true;
    }

    // 1. Prepare intrinsic uniforms
    UniformManager intrinsicValues{Layout::kStd140};
    CollectIntrinsicUniforms(fSharedContext->caps(), viewport, fDstReadBounds, &intrinsicValues);
    const auto uniformData = UniformDataBlock::Wrap(&intrinsicValues);

    if (fCurrentIntrinsicBufferIndex >= fIntrinsicConstantBuffers.size()) {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = 256;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = 0;
        Microsoft::WRL::ComPtr<ID3D11Buffer> buf;
        HRESULT hr = fSharedContext->device()->CreateBuffer(&desc, nullptr, &buf);
        if (FAILED(hr)) {
            return false;
        }
        fIntrinsicConstantBuffers.push_back(std::move(buf));
    }
    ID3D11Buffer* intrinsicBuf = fIntrinsicConstantBuffers[fCurrentIntrinsicBufferIndex++].Get();
    uint8_t paddedUniformData[256] = {};
    if (uniformData.data() && uniformData.size() > 0) {
        memcpy(paddedUniformData, uniformData.data(), std::min<size_t>(uniformData.size(), 256));
    }
    fImmediateDeviceContext->UpdateSubresource(intrinsicBuf, 0, nullptr, paddedUniformData, 0, 0);

    fImmediateDeviceContext->VSSetConstantBuffers(D3D11GraphicsPipeline::kIntrinsicUniformBufferIndex, 1, &intrinsicBuf);
    fImmediateDeviceContext->PSSetConstantBuffers(D3D11GraphicsPipeline::kIntrinsicUniformBufferIndex, 1, &intrinsicBuf);

    // 2. Set Render Target & Depth Stencil views
    ID3D11RenderTargetView* rtv = nullptr;
    if (colorTexture) {
        rtv = static_cast<const D3D11Texture*>(colorTexture)->renderTargetView();
    }

    ID3D11DepthStencilView* dsv = nullptr;
    if (depthStencilTexture) {
        dsv = static_cast<const D3D11Texture*>(depthStencilTexture)->depthStencilView();
    }

    if (rtv || dsv) {
        fImmediateDeviceContext->OMSetRenderTargets(rtv ? 1 : 0, rtv ? &rtv : nullptr, dsv);
    }

    // 3. Clear if needed
    if (rtv && renderPassDesc.fColorAttachment.fLoadOp == LoadOp::kClear) {
        const std::array<float, 4>& c = renderPassDesc.fClearColor;
        fImmediateDeviceContext->ClearRenderTargetView(rtv, c.data());
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
            fImmediateDeviceContext->ClearDepthStencilView(
                    dsv, clearFlags, renderPassDesc.fClearDepth, renderPassDesc.fClearStencil);
        }
    }

    // 4. Load MSAA from resolve texture if needed
    const bool loadMSAAFromResolve = (colorTexture != nullptr &&
                                      resolveTexture != nullptr &&
                                      renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad);
    if (loadMSAAFromResolve) {
        auto loadPipeline = fResourceProvider->findOrCreateLoadMSAAPipeline(renderPassDesc);
        if (!loadPipeline) {
            SKIA_LOG_E("Unable to create pipeline to load resolve texture into MSAA attachment");
            return false;
        }

        D3D11_VIEWPORT fullVP = {
            0.0f,
            0.0f,
            static_cast<FLOAT>(colorTexture->dimensions().width()),
            static_cast<FLOAT>(colorTexture->dimensions().height()),
            0.0f,
            1.0f
        };
        fImmediateDeviceContext->RSSetViewports(1, &fullVP);

        this->bindGraphicsPipeline(loadPipeline.get());

        ID3D11Buffer* nullBuffers[2] = {nullptr, nullptr};
        UINT zeroStrides[2] = {0, 0};
        UINT zeroOffsets[2] = {0, 0};
        fImmediateDeviceContext->IASetVertexBuffers(0, 2, nullBuffers, zeroStrides, zeroOffsets);

        ID3D11ShaderResourceView* srv =
                static_cast<const D3D11Texture*>(resolveTexture)->shaderResourceView();
        fImmediateDeviceContext->PSSetShaderResources(0, 1, &srv);

        fImmediateDeviceContext->Draw(4, 0);

        ID3D11ShaderResourceView* nullSRV = nullptr;
        fImmediateDeviceContext->PSSetShaderResources(0, 1, &nullSRV);

        fActiveGraphicsPipeline = nullptr;
    }

    // 5. Set Viewport and Scissor
    this->setViewport(viewport);
    D3D11_RECT d3dScissor = {
        std::max<LONG>(0, viewport.fLeft),
        std::max<LONG>(0, viewport.fTop),
        std::max<LONG>(0, viewport.fRight),
        std::max<LONG>(0, viewport.fBottom),
    };
    fImmediateDeviceContext->RSSetScissorRects(1, &d3dScissor);

    // 6. Execute Draw Passes
    for (const auto& drawPass : drawPasses) {
        if (!this->addDrawPass(drawPass.get())) SK_UNLIKELY {
            return false;
        }
    }

    // 7. MSAA Resolve if needed
    if (resolveTexture && colorTexture) {
        const auto* srcTex = static_cast<const D3D11Texture*>(colorTexture);
        const auto* dstTex = static_cast<const D3D11Texture*>(resolveTexture);
        D3D11_TEXTURE2D_DESC desc = {};
        dstTex->d3d11Texture()->GetDesc(&desc);
        fImmediateDeviceContext->ResolveSubresource(
                dstTex->d3d11Texture(), 0, srcTex->d3d11Texture(), 0, desc.Format);
    }

    // 8. Unbind render target to prevent hazards
    ID3D11RenderTargetView* nullRTV = nullptr;
    fImmediateDeviceContext->OMSetRenderTargets(0, &nullRTV, nullptr);

    return true;
}

bool D3D11CommandBuffer::addDrawPass(DrawPass* drawPass) {
    if (!drawPass->addResourceRefs(fResourceProvider, this)) SK_UNLIKELY {
        return false;
    }

    for (auto [type, cmdPtr] : drawPass->commands()) {
        switch (type) {
            case DrawPassCommands::Type::kBindGraphicsPipeline: {
                auto bgp = static_cast<DrawPassCommands::BindGraphicsPipeline*>(cmdPtr);
                if (!this->bindGraphicsPipeline(drawPass->getPipeline(bgp->fPipelineIndex)))
                    SK_UNLIKELY { return false; }
                break;
            }
            case DrawPassCommands::Type::kSetBlendConstants: {
                auto sbc = static_cast<DrawPassCommands::SetBlendConstants*>(cmdPtr);
                this->setBlendConstants(sbc->fBlendConstants);
                break;
            }
            case DrawPassCommands::Type::kBindUniformBuffer: {
                auto bub = static_cast<DrawPassCommands::BindUniformBuffer*>(cmdPtr);
                this->bindUniformBuffer(bub->fInfo, bub->fSlot);
                break;
            }
            case DrawPassCommands::Type::kBindStaticDataBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindStaticDataBuffer*>(cmdPtr);
                this->bindInputBuffer(bdb->fStaticData.fBuffer, bdb->fStaticData.fOffset,
                                      D3D11GraphicsPipeline::kStaticDataBufferIndex);
                break;
            }
            case DrawPassCommands::Type::kBindAppendDataBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindAppendDataBuffer*>(cmdPtr);
                this->bindInputBuffer(bdb->fAppendData.fBuffer, bdb->fAppendData.fOffset,
                                      D3D11GraphicsPipeline::kAppendDataBufferIndex);
                break;
            }
            case DrawPassCommands::Type::kBindIndexBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindIndexBuffer*>(cmdPtr);
                this->bindIndexBuffer(
                        bdb->fIndices.fBuffer, bdb->fIndices.fOffset);
                break;
            }
            case DrawPassCommands::Type::kBindIndirectBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindIndirectBuffer*>(cmdPtr);
                this->bindIndirectBuffer(
                        bdb->fIndirect.fBuffer, bdb->fIndirect.fOffset);
                break;
            }
            case DrawPassCommands::Type::kBindTexturesAndSamplers: {
                auto bts = static_cast<DrawPassCommands::BindTexturesAndSamplers*>(cmdPtr);
                this->bindTextureAndSamplers(*drawPass, *bts);
                break;
            }
            case DrawPassCommands::Type::kSetScissor: {
                auto ss = static_cast<DrawPassCommands::SetScissor*>(cmdPtr);
                this->setScissor(ss->fScissor);
                break;
            }
            case DrawPassCommands::Type::kDraw: {
                auto draw = static_cast<DrawPassCommands::Draw*>(cmdPtr);
                this->draw(draw->fType, draw->fBaseVertex, draw->fVertexCount);
                break;
            }
            case DrawPassCommands::Type::kDrawIndexed: {
                auto draw = static_cast<DrawPassCommands::DrawIndexed*>(cmdPtr);
                this->drawIndexed(
                        draw->fType, draw->fBaseIndex, draw->fIndexCount, draw->fBaseVertex);
                break;
            }
            case DrawPassCommands::Type::kDrawInstanced: {
                auto draw = static_cast<DrawPassCommands::DrawInstanced*>(cmdPtr);
                this->drawInstanced(draw->fType,
                                    draw->fBaseVertex,
                                    draw->fVertexCount,
                                    draw->fBaseInstance,
                                    draw->fInstanceCount);
                break;
            }
            case DrawPassCommands::Type::kDrawIndexedInstanced: {
                auto draw = static_cast<DrawPassCommands::DrawIndexedInstanced*>(cmdPtr);
                this->drawIndexedInstanced(draw->fType,
                                           draw->fBaseIndex,
                                           draw->fIndexCount,
                                           draw->fBaseVertex,
                                           draw->fBaseInstance,
                                           draw->fInstanceCount);
                break;
            }
            case DrawPassCommands::Type::kDrawIndirect: {
                auto draw = static_cast<DrawPassCommands::DrawIndirect*>(cmdPtr);
                this->drawIndirect(draw->fType);
                break;
            }
            case DrawPassCommands::Type::kDrawIndexedIndirect: {
                auto draw = static_cast<DrawPassCommands::DrawIndexedIndirect*>(cmdPtr);
                this->drawIndexedIndirect(draw->fType);
                break;
            }
            case DrawPassCommands::Type::kAddBarrier: {
                break;
            }
        }
    }
    return true;
}

bool D3D11CommandBuffer::bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) {
    auto* d3dPipeline = static_cast<const D3D11GraphicsPipeline*>(graphicsPipeline);
    if (!d3dPipeline) {
        return false;
    }
    fActiveGraphicsPipeline = d3dPipeline;

    fImmediateDeviceContext->IASetInputLayout(d3dPipeline->inputLayout());
    fImmediateDeviceContext->IASetPrimitiveTopology(d3dPipeline->primitiveTopology());
    fImmediateDeviceContext->VSSetShader(d3dPipeline->vertexShader(), nullptr, 0);
    fImmediateDeviceContext->PSSetShader(d3dPipeline->pixelShader(), nullptr, 0);
    fImmediateDeviceContext->RSSetState(d3dPipeline->rasterizerState());
    fImmediateDeviceContext->OMSetBlendState(d3dPipeline->blendState(), fBlendConstants.data(), 0xffffffff);
    fImmediateDeviceContext->OMSetDepthStencilState(d3dPipeline->depthStencilState(), d3dPipeline->stencilRef());

    if (fActiveGraphicsPipeline->dstReadStrategy() == DstReadStrategy::kTextureCopy &&
        fActiveGraphicsPipeline->numFragTexturesAndSamplers() == 2) {
        const auto* texture = static_cast<const D3D11Texture*>(fDstCopy.first);
        const auto* sampler = static_cast<const D3D11Sampler*>(fDstCopy.second);
        ID3D11ShaderResourceView* srv = texture ? texture->shaderResourceView() : nullptr;
        ID3D11SamplerState* samp = sampler ? sampler->d3d11SamplerState() : nullptr;
        fImmediateDeviceContext->PSSetShaderResources(0, 1, &srv);
        fImmediateDeviceContext->PSSetSamplers(0, 1, &samp);
    }

    fBoundUniformBuffersDirty = true;
    return true;
}

void D3D11CommandBuffer::setBlendConstants(std::array<float, 4> blendConstants) {
    fBlendConstants = blendConstants;
    if (fActiveGraphicsPipeline) {
        fImmediateDeviceContext->OMSetBlendState(
                fActiveGraphicsPipeline->blendState(), fBlendConstants.data(), 0xffffffff);
    }
}

void D3D11CommandBuffer::bindUniformBuffer(const BindBufferInfo& info, UniformSlot slot) {
    unsigned int bufferIndex = 0;
    switch (slot) {
        case UniformSlot::kCombinedUniforms:
            bufferIndex = D3D11GraphicsPipeline::kCombinedUniformIndex;
            break;
        case UniformSlot::kStorage:
            return;
    }
    fBoundUniforms[bufferIndex] = info;
    fBoundUniformBuffersDirty = true;
}

void D3D11CommandBuffer::bindInputBuffer(const Buffer* buffer, size_t offset, uint32_t bindingIndex) {
    SkASSERT(bindingIndex < 2);
    if (buffer) {
        fCurrentVertexBuffers[bindingIndex] = static_cast<const D3D11Buffer*>(buffer)->d3d11Buffer();
        fCurrentVertexOffsets[bindingIndex] = static_cast<UINT>(offset);
    } else {
        fCurrentVertexBuffers[bindingIndex] = nullptr;
        fCurrentVertexOffsets[bindingIndex] = 0;
    }
}

void D3D11CommandBuffer::bindIndexBuffer(const Buffer* indexBuffer, size_t offset) {
    if (indexBuffer) {
        fCurrentIndexBuffer = static_cast<const D3D11Buffer*>(indexBuffer)->d3d11Buffer();
        fCurrentIndexOffset = static_cast<UINT>(offset);
        fImmediateDeviceContext->IASetIndexBuffer(fCurrentIndexBuffer, DXGI_FORMAT_R16_UINT, fCurrentIndexOffset);
    }
}

void D3D11CommandBuffer::bindIndirectBuffer(const Buffer* indirectBuffer, size_t offset) {
    if (indirectBuffer) {
        fCurrentIndirectBuffer = static_cast<const D3D11Buffer*>(indirectBuffer)->d3d11Buffer();
        fCurrentIndirectBufferOffset = static_cast<UINT>(offset);
    } else {
        fCurrentIndirectBuffer = nullptr;
        fCurrentIndirectBufferOffset = 0;
    }
}

const D3D11Sampler* D3D11CommandBuffer::getSampler(
        const DrawPassCommands::BindTexturesAndSamplers& command, int32_t index) {
    auto desc = command.fSamplers[index];
    if (desc.isImmutable()) {
        const D3D11Sampler* immutableSampler = fActiveGraphicsPipeline->immutableSampler(index);
        if (immutableSampler) {
            this->trackResource(sk_ref_sp<Sampler>(const_cast<D3D11Sampler*>(immutableSampler)));
        }
        return immutableSampler;
    } else {
        return static_cast<const D3D11Sampler*>(
                fSharedContext->globalCache()->getDynamicSampler(desc));
    }
}

void D3D11CommandBuffer::bindTextureAndSamplers(
        const DrawPass& drawPass, const DrawPassCommands::BindTexturesAndSamplers& command) {
    SkASSERT(fActiveGraphicsPipeline);

    int numTexturesAndSamplers = command.fNumTexSamplers;
    ID3D11ShaderResourceView* srvs[16] = {nullptr};
    ID3D11SamplerState* samplers[16] = {nullptr};

    for (int i = 0; i < command.fNumTexSamplers && i < 16; ++i) {
        const auto* tex = static_cast<const D3D11Texture*>(command.fTextures[i]->texture());
        srvs[i] = tex ? tex->shaderResourceView() : nullptr;
        const auto* samp = this->getSampler(command, i);
        samplers[i] = samp ? samp->d3d11SamplerState() : nullptr;
    }

    if (fActiveGraphicsPipeline->dstReadStrategy() == DstReadStrategy::kTextureCopy) {
        int idx = command.fNumTexSamplers;
        if (idx < 16) {
            const auto* dstTex = static_cast<const D3D11Texture*>(fDstCopy.first);
            const auto* dstSamp = static_cast<const D3D11Sampler*>(fDstCopy.second);
            srvs[idx] = dstTex ? dstTex->shaderResourceView() : nullptr;
            samplers[idx] = dstSamp ? dstSamp->d3d11SamplerState() : nullptr;
            numTexturesAndSamplers++;
        }
    }

    if (numTexturesAndSamplers > 0) {
        fImmediateDeviceContext->PSSetShaderResources(0, numTexturesAndSamplers, srvs);
        fImmediateDeviceContext->PSSetSamplers(0, numTexturesAndSamplers, samplers);
    }
}

void D3D11CommandBuffer::setScissor(const Scissor& scissor) {
    SkIRect rect = scissor.getRect(fReplayTranslation, fRenderAreaBounds);
    D3D11_RECT d3dRect = {
        std::max<LONG>(0, rect.fLeft),
        std::max<LONG>(0, rect.fTop),
        std::max<LONG>(0, rect.fRight),
        std::max<LONG>(0, rect.fBottom),
    };
    fImmediateDeviceContext->RSSetScissorRects(1, &d3dRect);
}

void D3D11CommandBuffer::setViewport(SkIRect viewport) {
    D3D11_VIEWPORT vp = {
        static_cast<FLOAT>(viewport.fLeft),
        static_cast<FLOAT>(viewport.fTop),
        static_cast<FLOAT>(viewport.width()),
        static_cast<FLOAT>(viewport.height()),
        0.0f,
        1.0f
    };
    fImmediateDeviceContext->RSSetViewports(1, &vp);
}

void D3D11CommandBuffer::syncVertexBuffers() {
    if (fActiveGraphicsPipeline) {
        fCurrentVertexStrides[0] = fActiveGraphicsPipeline->staticStride();
        fCurrentVertexStrides[1] = fActiveGraphicsPipeline->appendStride();
        fImmediateDeviceContext->IASetVertexBuffers(
                0, 2, fCurrentVertexBuffers, fCurrentVertexStrides, fCurrentVertexOffsets);
    }
}

void D3D11CommandBuffer::syncUniformBuffers() {
    if (!fBoundUniformBuffersDirty || !fActiveGraphicsPipeline) {
        return;
    }
    fBoundUniformBuffersDirty = false;

    if (fActiveGraphicsPipeline->hasCombinedUniforms()) {
        const BindBufferInfo& combinedInfo =
                fBoundUniforms[D3D11GraphicsPipeline::kCombinedUniformIndex];
        if (combinedInfo.fBuffer) {
            ID3D11Buffer* cb = static_cast<const D3D11Buffer*>(combinedInfo.fBuffer)->d3d11Buffer();
            if (cb) {
                // In D3D11.1, both firstConstant and numConstants are measured in 16-byte shader
                // constants, and both must be multiples of 16 (i.e. 256 bytes).
                UINT firstConstant = static_cast<UINT>(combinedInfo.fOffset / 16);
                UINT sizeInConstants = static_cast<UINT>(SkAlignTo(combinedInfo.fSize, 16u) / 16);
                UINT numConstants = static_cast<UINT>(SkAlignTo(std::max<UINT>(sizeInConstants, 16u), 16u));

                D3D11_BUFFER_DESC bufDesc = {};
                cb->GetDesc(&bufDesc);
                UINT maxConstants = bufDesc.ByteWidth / 16;
                if (firstConstant < maxConstants) {
                    numConstants = std::min(numConstants, maxConstants - firstConstant);
                }

                fImmediateDeviceContext->VSSetConstantBuffers1(
                        D3D11GraphicsPipeline::kCombinedUniformIndex, 1, &cb, &firstConstant, &numConstants);
                if (fActiveGraphicsPipeline->hasFragmentShader()) {
                    fImmediateDeviceContext->PSSetConstantBuffers1(
                            D3D11GraphicsPipeline::kCombinedUniformIndex, 1, &cb, &firstConstant, &numConstants);
                }
            }
        }
    }
}

void D3D11CommandBuffer::draw(PrimitiveType, unsigned int baseVertex, unsigned int vertexCount) {
    this->syncVertexBuffers();
    this->syncUniformBuffers();
    fImmediateDeviceContext->Draw(vertexCount, baseVertex);
}

void D3D11CommandBuffer::drawIndexed(PrimitiveType,
                                     unsigned int baseIndex,
                                     unsigned int indexCount,
                                     unsigned int baseVertex) {
    this->syncVertexBuffers();
    this->syncUniformBuffers();
    fImmediateDeviceContext->DrawIndexed(indexCount, baseIndex, baseVertex);
}

void D3D11CommandBuffer::drawInstanced(PrimitiveType,
                                       unsigned int baseVertex,
                                       unsigned int vertexCount,
                                       unsigned int baseInstance,
                                       unsigned int instanceCount) {
    this->syncVertexBuffers();
    this->syncUniformBuffers();
    fImmediateDeviceContext->DrawInstanced(vertexCount, instanceCount, baseVertex, baseInstance);
}

void D3D11CommandBuffer::drawIndexedInstanced(PrimitiveType,
                                              unsigned int baseIndex,
                                              unsigned int indexCount,
                                              unsigned int baseVertex,
                                              unsigned int baseInstance,
                                              unsigned int instanceCount) {
    this->syncVertexBuffers();
    this->syncUniformBuffers();
    fImmediateDeviceContext->DrawIndexedInstanced(
            indexCount, instanceCount, baseIndex, baseVertex, baseInstance);
}

void D3D11CommandBuffer::drawIndirect(PrimitiveType) {
    this->syncVertexBuffers();
    this->syncUniformBuffers();
    fImmediateDeviceContext->DrawInstancedIndirect(fCurrentIndirectBuffer, fCurrentIndirectBufferOffset);
}

void D3D11CommandBuffer::drawIndexedIndirect(PrimitiveType) {
    this->syncVertexBuffers();
    this->syncUniformBuffers();
    fImmediateDeviceContext->DrawIndexedInstancedIndirect(
            fCurrentIndirectBuffer, fCurrentIndirectBufferOffset);
}

bool D3D11CommandBuffer::onAddComputePass(DispatchGroupSpan) {
    return false;
}

bool D3D11CommandBuffer::onCopyBufferToBuffer(const Buffer* srcBuffer,
                                              size_t srcOffset,
                                              const Buffer* dstBuffer,
                                              size_t dstOffset,
                                              size_t size) {
    if (!fImmediateDeviceContext || !srcBuffer || !dstBuffer || size == 0) {
        return false;
    }

    const auto* srcD3D = static_cast<const D3D11Buffer*>(srcBuffer);
    const auto* dstD3D = static_cast<const D3D11Buffer*>(dstBuffer);

    if (srcD3D->cpuData()) {
        if (dstD3D->cpuData()) {
            memcpy(const_cast<uint8_t*>(dstD3D->cpuData()) + dstOffset,
                   srcD3D->cpuData() + srcOffset,
                   size);
        }
        if (dstD3D->d3d11Buffer()) {
            D3D11_BUFFER_DESC desc = {};
            dstD3D->d3d11Buffer()->GetDesc(&desc);
            if (desc.BindFlags & D3D11_BIND_CONSTANT_BUFFER) {
                if (dstOffset == 0 && size >= desc.ByteWidth) {
                    fImmediateDeviceContext->UpdateSubresource1(
                            dstD3D->d3d11Buffer(), 0, nullptr, srcD3D->cpuData() + srcOffset, 0, 0, 0);
                } else {
                    UINT alignedStart = static_cast<UINT>(dstOffset);
                    UINT alignedEnd = static_cast<UINT>(SkAlignTo(dstOffset + size, static_cast<size_t>(16)));
                    alignedEnd = std::min(alignedEnd, desc.ByteWidth);
                    D3D11_BOX box = {
                            alignedStart,
                            0,
                            0,
                            alignedEnd,
                            1,
                            1,
                    };
                    fImmediateDeviceContext->UpdateSubresource1(
                            dstD3D->d3d11Buffer(), 0, &box, srcD3D->cpuData() + srcOffset, 0, 0, 0);
                }
            } else {
                D3D11_BOX box = {
                        static_cast<UINT>(dstOffset),
                        0,
                        0,
                        static_cast<UINT>(dstOffset + size),
                        1,
                        1,
                };
                fImmediateDeviceContext->UpdateSubresource(dstD3D->d3d11Buffer(),
                                                           0,
                                                           &box,
                                                           srcD3D->cpuData() + srcOffset,
                                                           0,
                                                           0);
            }
        }
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
        fImmediateDeviceContext->CopySubresourceRegion(dstD3D->d3d11Buffer(),
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

bool D3D11CommandBuffer::onCopyTextureToBuffer(const Texture* texture,
                                              SkIRect srcRect,
                                              const Buffer* buffer,
                                              size_t bufferOffset,
                                              size_t bufferRowBytes) {
    if (!fImmediateDeviceContext || !texture || !buffer) {
        return false;
    }

    const auto* srcTex = static_cast<const D3D11Texture*>(texture);
    auto* dstBuf = const_cast<D3D11Buffer*>(static_cast<const D3D11Buffer*>(buffer));

    D3D11_TEXTURE2D_DESC srcDesc = {};
    srcTex->d3d11Texture()->GetDesc(&srcDesc);

    D3D11_TEXTURE2D_DESC stagingDesc = {};
    stagingDesc.Width = srcRect.width();
    stagingDesc.Height = srcRect.height();
    stagingDesc.MipLevels = 1;
    stagingDesc.ArraySize = 1;
    stagingDesc.Format = srcDesc.Format;
    stagingDesc.SampleDesc.Count = 1;
    stagingDesc.SampleDesc.Quality = 0;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
    HRESULT hr = fSharedContext->device()->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
    if (FAILED(hr)) {
        return false;
    }

    D3D11_BOX srcBox = {
        static_cast<UINT>(srcRect.fLeft),
        static_cast<UINT>(srcRect.fTop),
        0,
        static_cast<UINT>(srcRect.fRight),
        static_cast<UINT>(srcRect.fBottom),
        1
    };

    fImmediateDeviceContext->CopySubresourceRegion(
            stagingTexture.Get(), 0, 0, 0, 0, srcTex->d3d11Texture(), 0, &srcBox);

    fPendingTextureToBufferCopies.push_back({
            std::move(stagingTexture),
            dstBuf,
            bufferOffset,
            bufferRowBytes,
            static_cast<uint32_t>(srcRect.width()),
            static_cast<uint32_t>(srcRect.height())
    });

    return true;
}

bool D3D11CommandBuffer::onCopyBufferToTexture(const Buffer* buffer,
                                              const Texture* texture,
                                              const BufferTextureCopyData* copyData,
                                              int count) {
    if (!fImmediateDeviceContext || !buffer || !texture || !copyData || count <= 0) {
        return false;
    }

    const auto* srcBuf = static_cast<const D3D11Buffer*>(buffer);
    const auto* dstTex = static_cast<const D3D11Texture*>(texture);

    if (srcBuf->cpuData()) {
        for (int i = 0; i < count; ++i) {
            const auto& cd = copyData[i];
            const void* pSrc = srcBuf->cpuData() + cd.fBufferOffset;
            D3D11_BOX box = {
                static_cast<UINT>(cd.fRect.fLeft),
                static_cast<UINT>(cd.fRect.fTop),
                0,
                static_cast<UINT>(cd.fRect.fRight),
                static_cast<UINT>(cd.fRect.fBottom),
                1
            };
            fImmediateDeviceContext->UpdateSubresource(
                dstTex->d3d11Texture(),
                cd.fMipLevel,
                &box,
                pSrc,
                static_cast<UINT>(cd.fBufferRowBytes),
                0);
        }
        return true;
    }
    return false;
}

bool D3D11CommandBuffer::onCopyTextureToTexture(const Texture* src,
                                                SkIRect srcRect,
                                                const Texture* dst,
                                                SkIPoint dstPoint,
                                                int mipLevel) {
    if (!fImmediateDeviceContext || !src || !dst) {
        return false;
    }
    const auto* srcTex = static_cast<const D3D11Texture*>(src);
    const auto* dstTex = static_cast<const D3D11Texture*>(dst);

    D3D11_BOX box = {
        static_cast<UINT>(srcRect.fLeft),
        static_cast<UINT>(srcRect.fTop),
        0,
        static_cast<UINT>(srcRect.fRight),
        static_cast<UINT>(srcRect.fBottom),
        1
    };
    fImmediateDeviceContext->CopySubresourceRegion(
        dstTex->d3d11Texture(),
        mipLevel,
        dstPoint.fX,
        dstPoint.fY,
        0,
        srcTex->d3d11Texture(),
        mipLevel,
        &box);
    return true;
}

bool D3D11CommandBuffer::onSynchronizeBufferToCpu(const Buffer*, bool* outDidResultInWork) {
    if (outDidResultInWork) {
        *outDidResultInWork = false;
    }
    return true;
}

bool D3D11CommandBuffer::onClearBuffer(const Buffer* buffer, size_t offset, size_t size) {
    if (!buffer || size == 0) {
        return false;
    }
    auto* d3dBuf = const_cast<D3D11Buffer*>(static_cast<const D3D11Buffer*>(buffer));
    if (d3dBuf->cpuData()) {
        memset(d3dBuf->cpuData() + offset, 0, size);
        return true;
    }
    if (d3dBuf->d3d11Buffer()) {
        std::vector<uint8_t> zeros(size, 0);
        D3D11_BOX box = {
            static_cast<UINT>(offset), 0, 0,
            static_cast<UINT>(offset + size), 1, 1
        };
        fImmediateDeviceContext->UpdateSubresource(d3dBuf->d3d11Buffer(), 0, &box, zeros.data(), 0, 0);
        return true;
    }
    return false;
}

} // namespace skgpu::graphite
