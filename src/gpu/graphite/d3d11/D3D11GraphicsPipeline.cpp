/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/d3d11/D3D11GraphicsPipeline.h"

#include "src/gpu/graphite/d3d11/D3D11SharedContext.h"

namespace skgpu::graphite {

sk_sp<D3D11GraphicsPipeline> D3D11GraphicsPipeline::Make(
        const D3D11SharedContext* /*sharedContext*/,
        const RuntimeEffectDictionary* /*runtimeDict*/,
        const UniqueKey& /*pipelineKey*/,
        const GraphicsPipelineDesc& /*pipelineDesc*/,
        const RenderPassDesc& /*renderPassDesc*/,
        SkEnumBitMask<PipelineCreationFlags> /*flags*/,
        uint32_t /*compilationID*/) {
    // Pipeline compilation will be implemented in Phase 3.
    return nullptr;
}

D3D11GraphicsPipeline::D3D11GraphicsPipeline(
        const SharedContext* sharedContext,
        const PipelineInfo& pipelineInfo,
        std::string_view label,
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vs,
        Microsoft::WRL::ComPtr<ID3D11PixelShader> ps,
        Microsoft::WRL::ComPtr<ID3D11InputLayout> layout,
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState,
        Microsoft::WRL::ComPtr<ID3D11BlendState> blendState,
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState,
        uint32_t stencilRef,
        D3D11_PRIMITIVE_TOPOLOGY topology)
        : GraphicsPipeline(sharedContext, pipelineInfo, label)
        , fVertexShader(std::move(vs))
        , fPixelShader(std::move(ps))
        , fInputLayout(std::move(layout))
        , fRasterizerState(std::move(rasterizerState))
        , fBlendState(std::move(blendState))
        , fDepthStencilState(std::move(depthStencilState))
        , fStencilRef(stencilRef)
        , fPrimitiveTopology(topology) {}

D3D11GraphicsPipeline::~D3D11GraphicsPipeline() {
    this->freeGpuData();
}

void D3D11GraphicsPipeline::freeGpuData() {
    fVertexShader.Reset();
    fPixelShader.Reset();
    fInputLayout.Reset();
    fRasterizerState.Reset();
    fBlendState.Reset();
    fDepthStencilState.Reset();
}

} // namespace skgpu::graphite
