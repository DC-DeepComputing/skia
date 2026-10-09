/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_D3D11GraphicsPipeline_DEFINED
#define skgpu_graphite_D3D11GraphicsPipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include <d3d11_1.h>
#include <wrl/client.h>

namespace skgpu::graphite {

class D3D11SharedContext;
class GraphicsPipelineDesc;
struct RenderPassDesc;
class RuntimeEffectDictionary;

class D3D11GraphicsPipeline final : public GraphicsPipeline {
public:
    inline static constexpr unsigned int kIntrinsicUniformBufferIndex = 0;
    inline static constexpr unsigned int kCombinedUniformIndex = 1;
    inline static constexpr unsigned int kMaxNumUniformBuffers = 2;

    static sk_sp<D3D11GraphicsPipeline> Make(const D3D11SharedContext* sharedContext,
                                             const RuntimeEffectDictionary* runtimeDict,
                                             const UniqueKey& pipelineKey,
                                             const GraphicsPipelineDesc& pipelineDesc,
                                             const RenderPassDesc& renderPassDesc,
                                             SkEnumBitMask<PipelineCreationFlags>,
                                             uint32_t compilationID);

    ~D3D11GraphicsPipeline() override;

    ID3D11VertexShader* vertexShader() const { return fVertexShader.Get(); }
    ID3D11PixelShader* pixelShader() const { return fPixelShader.Get(); }
    ID3D11InputLayout* inputLayout() const { return fInputLayout.Get(); }
    ID3D11RasterizerState* rasterizerState() const { return fRasterizerState.Get(); }
    ID3D11BlendState* blendState() const { return fBlendState.Get(); }
    ID3D11DepthStencilState* depthStencilState() const { return fDepthStencilState.Get(); }
    uint32_t stencilRef() const { return fStencilRef; }
    D3D11_PRIMITIVE_TOPOLOGY primitiveTopology() const { return fPrimitiveTopology; }

private:
    D3D11GraphicsPipeline(const SharedContext* sharedContext,
                          const PipelineInfo& pipelineInfo,
                          std::string_view label,
                          Microsoft::WRL::ComPtr<ID3D11VertexShader> vs,
                          Microsoft::WRL::ComPtr<ID3D11PixelShader> ps,
                          Microsoft::WRL::ComPtr<ID3D11InputLayout> layout,
                          Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState,
                          Microsoft::WRL::ComPtr<ID3D11BlendState> blendState,
                          Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState,
                          uint32_t stencilRef,
                          D3D11_PRIMITIVE_TOPOLOGY topology);

    void freeGpuData() override;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> fVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> fPixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> fInputLayout;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> fRasterizerState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> fBlendState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> fDepthStencilState;
    uint32_t fStencilRef = 0;
    D3D11_PRIMITIVE_TOPOLOGY fPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_D3D11GraphicsPipeline_DEFINED
