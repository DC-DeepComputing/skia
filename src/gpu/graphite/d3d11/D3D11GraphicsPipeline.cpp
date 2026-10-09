/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/d3d11/D3D11GraphicsPipeline.h"

#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/d3d11/D3D11GraphiteTypes.h"
#include "src/gpu/Blend.h"
#include "src/gpu/SkSLToBackend.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "include/private/SkLog.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ShaderInfo.h"
#include "src/gpu/graphite/ThreadSafeResourceProvider.h"
#include "src/gpu/graphite/d3d11/D3D11Caps.h"
#include "src/gpu/graphite/d3d11/D3D11SharedContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/codegen/SkSLSPIRVCodeGenerator.h"
#include "src/sksl/codegen/SkSLSPIRVtoHLSL.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <d3dcompiler.h>

namespace skgpu::graphite {

namespace {

inline DXGI_FORMAT vertex_attrib_type_to_dxgi_format(VertexAttribType type) {
    switch (type) {
        case VertexAttribType::kFloat:
            return DXGI_FORMAT_R32_FLOAT;
        case VertexAttribType::kFloat2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case VertexAttribType::kFloat3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case VertexAttribType::kFloat4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case VertexAttribType::kHalf2:
            return DXGI_FORMAT_R16G16_FLOAT;
        case VertexAttribType::kHalf4:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case VertexAttribType::kInt2:
            return DXGI_FORMAT_R32G32_SINT;
        case VertexAttribType::kInt3:
            return DXGI_FORMAT_R32G32B32_SINT;
        case VertexAttribType::kInt4:
            return DXGI_FORMAT_R32G32B32A32_SINT;
        case VertexAttribType::kUInt2:
            return DXGI_FORMAT_R32G32_UINT;
        case VertexAttribType::kByte2:
            return DXGI_FORMAT_R8G8_SINT;
        case VertexAttribType::kByte4:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case VertexAttribType::kUByte2:
            return DXGI_FORMAT_R8G8_UINT;
        case VertexAttribType::kUByte4:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case VertexAttribType::kUByte4_norm:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case VertexAttribType::kShort2:
            return DXGI_FORMAT_R16G16_SINT;
        case VertexAttribType::kShort4:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case VertexAttribType::kUShort2:
            return DXGI_FORMAT_R16G16_UINT;
        case VertexAttribType::kUShort2_norm:
            return DXGI_FORMAT_R16G16_UNORM;
        case VertexAttribType::kInt:
            return DXGI_FORMAT_R32_SINT;
        case VertexAttribType::kUInt:
            return DXGI_FORMAT_R32_UINT;
        case VertexAttribType::kUShort4_norm:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case VertexAttribType::kHalf:
            return DXGI_FORMAT_R16_FLOAT;
        case VertexAttribType::kByte:
            return DXGI_FORMAT_R8_SINT;
        case VertexAttribType::kUByte:
            return DXGI_FORMAT_R8_UINT;
        case VertexAttribType::kUByte_norm:
            return DXGI_FORMAT_R8_UNORM;
        case VertexAttribType::kUShort_norm:
            return DXGI_FORMAT_R16_UNORM;
    }
    SkUNREACHABLE;
}

inline D3D11_COMPARISON_FUNC compare_op_to_d3d11(CompareOp op) {
    switch (op) {
        case CompareOp::kAlways:
            return D3D11_COMPARISON_ALWAYS;
        case CompareOp::kNever:
            return D3D11_COMPARISON_NEVER;
        case CompareOp::kGreater:
            return D3D11_COMPARISON_GREATER;
        case CompareOp::kGEqual:
            return D3D11_COMPARISON_GREATER_EQUAL;
        case CompareOp::kLess:
            return D3D11_COMPARISON_LESS;
        case CompareOp::kLEqual:
            return D3D11_COMPARISON_LESS_EQUAL;
        case CompareOp::kEqual:
            return D3D11_COMPARISON_EQUAL;
        case CompareOp::kNotEqual:
            return D3D11_COMPARISON_NOT_EQUAL;
    }
    SkUNREACHABLE;
}

inline D3D11_STENCIL_OP stencil_op_to_d3d11(StencilOp op) {
    switch (op) {
        case StencilOp::kKeep:
            return D3D11_STENCIL_OP_KEEP;
        case StencilOp::kZero:
            return D3D11_STENCIL_OP_ZERO;
        case StencilOp::kReplace:
            return D3D11_STENCIL_OP_REPLACE;
        case StencilOp::kInvert:
            return D3D11_STENCIL_OP_INVERT;
        case StencilOp::kIncWrap:
            return D3D11_STENCIL_OP_INCR;
        case StencilOp::kDecWrap:
            return D3D11_STENCIL_OP_DECR;
        case StencilOp::kIncClamp:
            return D3D11_STENCIL_OP_INCR_SAT;
        case StencilOp::kDecClamp:
            return D3D11_STENCIL_OP_DECR_SAT;
    }
    SkUNREACHABLE;
}

inline D3D11_BLEND blend_coeff_to_d3d11(skgpu::BlendCoeff coeff) {
    switch (coeff) {
        case skgpu::BlendCoeff::kZero:
            return D3D11_BLEND_ZERO;
        case skgpu::BlendCoeff::kOne:
            return D3D11_BLEND_ONE;
        case skgpu::BlendCoeff::kSC:
            return D3D11_BLEND_SRC_COLOR;
        case skgpu::BlendCoeff::kISC:
            return D3D11_BLEND_INV_SRC_COLOR;
        case skgpu::BlendCoeff::kDC:
            return D3D11_BLEND_DEST_COLOR;
        case skgpu::BlendCoeff::kIDC:
            return D3D11_BLEND_INV_DEST_COLOR;
        case skgpu::BlendCoeff::kSA:
            return D3D11_BLEND_SRC_ALPHA;
        case skgpu::BlendCoeff::kISA:
            return D3D11_BLEND_INV_SRC_ALPHA;
        case skgpu::BlendCoeff::kDA:
            return D3D11_BLEND_DEST_ALPHA;
        case skgpu::BlendCoeff::kIDA:
            return D3D11_BLEND_INV_DEST_ALPHA;
        case skgpu::BlendCoeff::kConstC:
            return D3D11_BLEND_BLEND_FACTOR;
        case skgpu::BlendCoeff::kIConstC:
            return D3D11_BLEND_INV_BLEND_FACTOR;
        case skgpu::BlendCoeff::kS2C:
            return D3D11_BLEND_SRC1_COLOR;
        case skgpu::BlendCoeff::kIS2C:
            return D3D11_BLEND_INV_SRC1_COLOR;
        case skgpu::BlendCoeff::kS2A:
            return D3D11_BLEND_SRC1_ALPHA;
        case skgpu::BlendCoeff::kIS2A:
            return D3D11_BLEND_INV_SRC1_ALPHA;
        case skgpu::BlendCoeff::kIllegal:
            return D3D11_BLEND_ONE;
    }
    SkUNREACHABLE;
}

inline D3D11_BLEND_OP blend_equation_to_d3d11(skgpu::BlendEquation eq) {
    switch (eq) {
        case skgpu::BlendEquation::kAdd:
            return D3D11_BLEND_OP_ADD;
        case skgpu::BlendEquation::kSubtract:
            return D3D11_BLEND_OP_SUBTRACT;
        case skgpu::BlendEquation::kReverseSubtract:
            return D3D11_BLEND_OP_REV_SUBTRACT;
        default:
            return D3D11_BLEND_OP_ADD;
    }
}

inline D3D11_PRIMITIVE_TOPOLOGY primitive_type_to_d3d11(PrimitiveType type) {
    switch (type) {
        case PrimitiveType::kTriangles:
            return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PrimitiveType::kTriangleStrip:
            return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case PrimitiveType::kPoints:
            return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    }
    SkUNREACHABLE;
}

} // anonymous namespace

sk_sp<D3D11GraphicsPipeline> D3D11GraphicsPipeline::Make(
        const D3D11SharedContext* sharedContext,
        const RuntimeEffectDictionary* runtimeDict,
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t compilationID) {
    const D3D11Caps& caps = *sharedContext->d3d11Caps();
    ShaderErrorHandler* errorHandler = caps.shaderErrorHandler();
    const RenderStep* step = sharedContext->rendererProvider()->lookup(pipelineDesc.renderStepID());

    skia_private::TArray<SamplerDesc> samplerDescArr;
    std::unique_ptr<ShaderInfo> shaderInfo =
            ShaderInfo::Make(&caps,
                             sharedContext->shaderCodeDictionary(),
                             runtimeDict,
                             renderPassDesc,
                             step,
                             pipelineDesc.paintParamsID(),
                             &samplerDescArr);
    if (!shaderInfo) {
        return nullptr;
    }

    SkSL::ProgramSettings settings;
    settings.fSharpenTextures = true;
    settings.fForceNoRTFlip = true;
    settings.fForceHighPrecision = true;

    // 1. Compile Vertex Shader
    const std::string& vsSkSL = shaderInfo->vertexSkSL();
    SkSL::NativeShader vsSpirv;
    SkSL::ProgramInterface vsInterface;
    if (!skgpu::SkSLToBackend(caps.shaderCaps(),
                              &SkSL::ToSPIRV,
                              "SPIRV",
                              vsSkSL,
                              SkSL::ProgramKind::kGraphiteVertex,
                              settings,
                              &vsSpirv,
                              &vsInterface,
                              errorHandler)) {
        return nullptr;
    }

    std::string vsHLSL;
    SkSpan<const uint32_t> vsSpirvSpan(vsSpirv.fBinary.data(), vsSpirv.fBinary.size());
    SkSL::SPIRVtoHLSL(vsSpirvSpan, &vsHLSL, /*shaderModel=*/50);

    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(SK_DEBUG)
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> vsBytecode;
    Microsoft::WRL::ComPtr<ID3DBlob> vsErrorBlob;
    HRESULT hr = D3DCompile(vsHLSL.c_str(),
                            vsHLSL.size(),
                            shaderInfo->vsLabel().c_str(),
                            nullptr,
                            D3D_COMPILE_STANDARD_FILE_INCLUDE,
                            "main",
                            "vs_5_0",
                            compileFlags,
                            0,
                            &vsBytecode,
                            &vsErrorBlob);
    if (FAILED(hr)) {
        if (vsErrorBlob) {
            SkDebugf("D3DCompile VS error: %s\nHLSL:\n%s\n",
                     static_cast<const char*>(vsErrorBlob->GetBufferPointer()),
                     vsHLSL.c_str());
        }
        return nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
    hr = sharedContext->device()->CreateVertexShader(
            vsBytecode->GetBufferPointer(),
            vsBytecode->GetBufferSize(),
            nullptr,
            &vs);
    if (FAILED(hr)) {
        return nullptr;
    }

    // 2. Compile Pixel Shader (if present)
    const std::string& fsSkSL = shaderInfo->fragmentSkSL();
    const bool hasFragmentSkSL = !fsSkSL.empty();
    std::string fsHLSL;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
    if (hasFragmentSkSL) {
        SkSL::NativeShader fsSpirv;
        SkSL::ProgramInterface fsInterface;
        if (!skgpu::SkSLToBackend(caps.shaderCaps(),
                                  &SkSL::ToSPIRV,
                                  "SPIRV",
                                  fsSkSL,
                                  SkSL::ProgramKind::kGraphiteFragment,
                                  settings,
                                  &fsSpirv,
                                  &fsInterface,
                                  errorHandler)) {
            return nullptr;
        }

        SkSpan<const uint32_t> fsSpirvSpan(fsSpirv.fBinary.data(), fsSpirv.fBinary.size());
        SkSL::SPIRVtoHLSL(fsSpirvSpan, &fsHLSL, /*shaderModel=*/50);

        Microsoft::WRL::ComPtr<ID3DBlob> psBytecode;
        Microsoft::WRL::ComPtr<ID3DBlob> psErrorBlob;
        hr = D3DCompile(fsHLSL.c_str(),
                        fsHLSL.size(),
                        shaderInfo->fsLabel().c_str(),
                        nullptr,
                        D3D_COMPILE_STANDARD_FILE_INCLUDE,
                        "main",
                        "ps_5_0",
                        compileFlags,
                        0,
                        &psBytecode,
                        &psErrorBlob);
        if (FAILED(hr)) {
            if (psErrorBlob) {
                SkDebugf("D3DCompile PS error: %s\nHLSL:\n%s\n",
                         static_cast<const char*>(psErrorBlob->GetBufferPointer()),
                         fsHLSL.c_str());
            }
            return nullptr;
        }

        hr = sharedContext->device()->CreatePixelShader(
                psBytecode->GetBufferPointer(),
                psBytecode->GetBufferSize(),
                nullptr,
                &ps);
        if (FAILED(hr)) {
            return nullptr;
        }
    }

    // 3. Create Input Layout
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
    uint32_t location = 0;

    // Static attributes (Slot 0)
    uint32_t staticOffset = 0;
    for (const Attribute& attr : step->staticAttributes()) {
        D3D11_INPUT_ELEMENT_DESC elem = {};
        elem.SemanticName = "TEXCOORD";
        elem.SemanticIndex = location++;
        elem.Format = vertex_attrib_type_to_dxgi_format(attr.cpuType());
        elem.InputSlot = kStaticDataBufferIndex;
        elem.AlignedByteOffset = staticOffset;
        elem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        elem.InstanceDataStepRate = 0;
        inputElements.push_back(elem);
        staticOffset += attr.sizeAlign4();
    }

    // Append attributes (Slot 1)
    bool isInstance = !step->appendsVertices();
    uint32_t appendOffset = 0;
    for (const Attribute& attr : shaderInfo->appendAttributes()) {
        D3D11_INPUT_ELEMENT_DESC elem = {};
        elem.SemanticName = "TEXCOORD";
        elem.SemanticIndex = location++;
        elem.Format = vertex_attrib_type_to_dxgi_format(attr.cpuType());
        elem.InputSlot = kAppendDataBufferIndex;
        elem.AlignedByteOffset = appendOffset;
        elem.InputSlotClass = isInstance ? D3D11_INPUT_PER_INSTANCE_DATA
                                         : D3D11_INPUT_PER_VERTEX_DATA;
        elem.InstanceDataStepRate = isInstance ? 1 : 0;
        inputElements.push_back(elem);
        appendOffset += attr.sizeAlign4();
    }

    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    if (!inputElements.empty()) {
        hr = sharedContext->device()->CreateInputLayout(
                inputElements.data(),
                static_cast<UINT>(inputElements.size()),
                vsBytecode->GetBufferPointer(),
                vsBytecode->GetBufferSize(),
                &inputLayout);
        if (FAILED(hr)) {
            SkDebugf("Failed to CreateInputLayout: hr=0x%08x\n", static_cast<unsigned int>(hr));
            return nullptr;
        }
    }

    // 4. Create Rasterizer State
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = TRUE;
    rasterizerDesc.MultisampleEnable = renderPassDesc.fSampleCount > SampleCount::k1;
    rasterizerDesc.AntialiasedLineEnable = FALSE;

    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
    hr = sharedContext->device()->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
    if (FAILED(hr)) {
        return nullptr;
    }

    // 5. Create Blend State
    D3D11_BLEND_DESC1 blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;

    const BlendInfo& blendInfo = shaderInfo->blendInfo();
    bool blendOn = !skgpu::BlendShouldDisable(blendInfo.fEquation,
                                              blendInfo.fSrcBlend,
                                              blendInfo.fDstBlend);

    auto& rtDesc = blendDesc.RenderTarget[0];
    rtDesc.BlendEnable = blendOn;
    rtDesc.LogicOpEnable = FALSE;
    rtDesc.LogicOp = D3D11_LOGIC_OP_NOOP;
    if (blendOn) {
        rtDesc.SrcBlend = blend_coeff_to_d3d11(blendInfo.fSrcBlend);
        rtDesc.DestBlend = blend_coeff_to_d3d11(blendInfo.fDstBlend);
        rtDesc.BlendOp = blend_equation_to_d3d11(blendInfo.fEquation);
        rtDesc.SrcBlendAlpha = blend_coeff_to_d3d11(blendInfo.fSrcBlend);
        rtDesc.DestBlendAlpha = blend_coeff_to_d3d11(blendInfo.fDstBlend);
        rtDesc.BlendOpAlpha = blend_equation_to_d3d11(blendInfo.fEquation);
    } else {
        rtDesc.SrcBlend = D3D11_BLEND_ONE;
        rtDesc.DestBlend = D3D11_BLEND_ZERO;
        rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
        rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
        rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
        rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    }

    if (blendInfo.fWritesColor && hasFragmentSkSL) {
        rtDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    } else {
        rtDesc.RenderTargetWriteMask = 0;
    }

    Microsoft::WRL::ComPtr<ID3D11BlendState1> blendState;
    hr = sharedContext->device()->CreateBlendState1(&blendDesc, &blendState);
    if (FAILED(hr)) {
        return nullptr;
    }

    // 6. Create Depth-Stencil State
    const auto& depthStencilSettings = shaderInfo->depthStencilSettings();
    TextureFormat dsFormat = renderPassDesc.fDepthStencilAttachment.fFormat;

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    if (dsFormat != TextureFormat::kUnsupported) {
        if (depthStencilSettings.depthEnabled()) {
            depthStencilDesc.DepthEnable = TRUE;
            depthStencilDesc.DepthWriteMask = depthStencilSettings.fDepthWriteEnabled
                                                  ? D3D11_DEPTH_WRITE_MASK_ALL
                                                  : D3D11_DEPTH_WRITE_MASK_ZERO;
            depthStencilDesc.DepthFunc = compare_op_to_d3d11(depthStencilSettings.fDepthCompareOp);
        } else {
            depthStencilDesc.DepthEnable = FALSE;
            depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
            depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        }

        if (depthStencilSettings.stencilEnabled()) {
            depthStencilDesc.StencilEnable = TRUE;
            depthStencilDesc.StencilReadMask = depthStencilSettings.fStencilReadMask;
            depthStencilDesc.StencilWriteMask = depthStencilSettings.fStencilWriteMask;

            // Front
            depthStencilDesc.FrontFace.StencilFailOp =
                    stencil_op_to_d3d11(depthStencilSettings.fFrontStencil.fStencilFailOp);
            depthStencilDesc.FrontFace.StencilDepthFailOp =
                    stencil_op_to_d3d11(depthStencilSettings.fFrontStencil.fDepthFailOp);
            depthStencilDesc.FrontFace.StencilPassOp =
                    stencil_op_to_d3d11(depthStencilSettings.fFrontStencil.fDepthStencilPassOp);
            depthStencilDesc.FrontFace.StencilFunc =
                    compare_op_to_d3d11(depthStencilSettings.fFrontStencil.fCompareOp);

            // Back
            depthStencilDesc.BackFace.StencilFailOp =
                    stencil_op_to_d3d11(depthStencilSettings.fBackStencil.fStencilFailOp);
            depthStencilDesc.BackFace.StencilDepthFailOp =
                    stencil_op_to_d3d11(depthStencilSettings.fBackStencil.fDepthFailOp);
            depthStencilDesc.BackFace.StencilPassOp =
                    stencil_op_to_d3d11(depthStencilSettings.fBackStencil.fDepthStencilPassOp);
            depthStencilDesc.BackFace.StencilFunc =
                    compare_op_to_d3d11(depthStencilSettings.fBackStencil.fCompareOp);
        }
    }

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
    hr = sharedContext->device()->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
    if (FAILED(hr)) {
        return nullptr;
    }

    // 7. Immutable Samplers
    skia_private::TArray<sk_sp<D3D11Sampler>> immutableSamplers;
    if (!samplerDescArr.empty()) {
        immutableSamplers.reserve(samplerDescArr.size());
        ThreadSafeResourceProvider* resourceProvider = sharedContext->threadSafeResourceProvider();
        for (const SamplerDesc& samplerDesc : samplerDescArr) {
            if (samplerDesc.isImmutable()) {
                sk_sp<Sampler> sampler = resourceProvider->findOrCreateCompatibleSampler(samplerDesc);
                if (!sampler) {
                    return nullptr;
                }
                immutableSamplers.push_back(sk_ref_sp(static_cast<D3D11Sampler*>(sampler.get())));
            } else {
                immutableSamplers.push_back(nullptr);
            }
        }
    }

    D3D11_PRIMITIVE_TOPOLOGY topology = primitive_type_to_d3d11(step->primitiveType());

    PipelineInfo pipelineInfo{ *shaderInfo, pipelineCreationFlags,
                               pipelineKey.hash(), compilationID };
#if defined(GPU_TEST_UTILS)
    pipelineInfo.fSkSLVertexShader = vsSkSL;
    pipelineInfo.fSkSLFragmentShader = fsSkSL;
    pipelineInfo.fNativeVertexShader = std::move(vsHLSL);
    pipelineInfo.fNativeFragmentShader = std::move(fsHLSL);
#endif

    return sk_sp<D3D11GraphicsPipeline>(
            new D3D11GraphicsPipeline(sharedContext,
                                     pipelineInfo,
                                     shaderInfo->pipelineLabel(),
                                     std::move(vs),
                                     std::move(ps),
                                     std::move(inputLayout),
                                     std::move(rasterizerState),
                                     std::move(blendState),
                                     std::move(depthStencilState),
                                     depthStencilSettings.fStencilReferenceValue,
                                     topology,
                                     staticOffset,
                                     appendOffset,
                                     std::move(immutableSamplers)));
}

D3D11GraphicsPipeline::D3D11GraphicsPipeline(
        const SharedContext* sharedContext,
        const PipelineInfo& pipelineInfo,
        std::string_view label,
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vs,
        Microsoft::WRL::ComPtr<ID3D11PixelShader> ps,
        Microsoft::WRL::ComPtr<ID3D11InputLayout> layout,
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState,
        Microsoft::WRL::ComPtr<ID3D11BlendState1> blendState,
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState,
        uint32_t stencilRef,
        D3D11_PRIMITIVE_TOPOLOGY topology,
        uint32_t staticStride,
        uint32_t appendStride,
        skia_private::TArray<sk_sp<D3D11Sampler>> immutableSamplers)
        : GraphicsPipeline(sharedContext, pipelineInfo, label)
        , fVertexShader(std::move(vs))
        , fPixelShader(std::move(ps))
        , fInputLayout(std::move(layout))
        , fRasterizerState(std::move(rasterizerState))
        , fBlendState(std::move(blendState))
        , fDepthStencilState(std::move(depthStencilState))
        , fStencilRef(stencilRef)
        , fPrimitiveTopology(topology)
        , fStaticStride(staticStride)
        , fAppendStride(appendStride)
        , fImmutableSamplers(std::move(immutableSamplers)) {}

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
    fImmutableSamplers.clear();
}

} // namespace skgpu::graphite
