/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/d3d11/D3D11Caps.h"

#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/d3d11/D3D11BackendContext.h"
#include "src/gpu/graphite/d3d11/D3D11GraphicsPipeline.h"
#include "src/gpu/SwizzlePriv.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu::graphite {

#define D3D11_FORMAT_MAPPING(M)                                            \
    M(TextureFormat::kR8,             DXGI_FORMAT_R8_UNORM)                \
    M(TextureFormat::kR16,            DXGI_FORMAT_R16_UNORM)               \
    M(TextureFormat::kR16F,           DXGI_FORMAT_R16_FLOAT)               \
    M(TextureFormat::kR32F,           DXGI_FORMAT_R32_FLOAT)               \
    M(TextureFormat::kRG8,            DXGI_FORMAT_R8G8_UNORM)              \
    M(TextureFormat::kRG16,           DXGI_FORMAT_R16G16_UNORM)            \
    M(TextureFormat::kRG16F,          DXGI_FORMAT_R16G16_FLOAT)            \
    M(TextureFormat::kRG32F,          DXGI_FORMAT_R32G32_FLOAT)            \
    M(TextureFormat::kRGBA8,          DXGI_FORMAT_R8G8B8A8_UNORM)          \
    M(TextureFormat::kRGBA16,         DXGI_FORMAT_R16G16B16A16_UNORM)      \
    M(TextureFormat::kRGBA16F,        DXGI_FORMAT_R16G16B16A16_FLOAT)      \
    M(TextureFormat::kRGBA32F,        DXGI_FORMAT_R32G32B32A32_FLOAT)      \
    M(TextureFormat::kRGB10_A2,       DXGI_FORMAT_R10G10B10A2_UNORM)       \
    M(TextureFormat::kRGBA8_sRGB,     DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)     \
    M(TextureFormat::kBGRA8,          DXGI_FORMAT_B8G8R8A8_UNORM)          \
    M(TextureFormat::kBGRA8_sRGB,     DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)     \
    M(TextureFormat::kRGBA8_BC1,      DXGI_FORMAT_BC1_UNORM)               \
    M(TextureFormat::kRGBA8_BC1_sRGB, DXGI_FORMAT_BC1_UNORM_SRGB)          \
    M(TextureFormat::kD16,            DXGI_FORMAT_D16_UNORM)               \
    M(TextureFormat::kD24_S8,         DXGI_FORMAT_D24_UNORM_S8_UINT)       \
    M(TextureFormat::kD32F,           DXGI_FORMAT_D32_FLOAT)               \
    M(TextureFormat::kD32F_S8,        DXGI_FORMAT_D32_FLOAT_S8X24_UINT)

TextureFormat D3D11FormatToTextureFormat(DXGI_FORMAT format) {
#define M(TF, DXGI) case DXGI: return TF;
    switch (format) {
        D3D11_FORMAT_MAPPING(M)
        default: return TextureFormat::kUnsupported;
    }
#undef M
}

DXGI_FORMAT TextureFormatToD3D11Format(TextureFormat format) {
#define M(TF, DXGI) case TF: return DXGI;
    switch (format) {
        D3D11_FORMAT_MAPPING(M)
        default: return DXGI_FORMAT_UNKNOWN;
    }
#undef M
}

namespace {
static const skgpu::UniqueKey::Domain kD3D11GraphicsPipelineDomain =
        skgpu::UniqueKey::GenerateDomain();

constexpr int kFormatBits = 8;
constexpr int kDepthStencilNumSamplesOffset = 0;
constexpr int kDepthStencilFormatOffset = kDepthStencilNumSamplesOffset + kNumSampleKeyBits;
constexpr int kColorNumSamplesOffset = kDepthStencilFormatOffset + kFormatBits;
constexpr int kColorFormatOffset = kColorNumSamplesOffset + kNumSampleKeyBits;

constexpr uint32_t kFormatMask = (1 << kFormatBits) - 1;
constexpr uint32_t kNumSamplesMask = (1 << kNumSampleKeyBits) - 1;
} // anonymous namespace

D3D11Caps::D3D11Caps(const D3D11BackendContext& backendContext, const ContextOptions& options)
        : Caps() {
    this->initCaps(backendContext, options);
    this->initShaderCaps();
    this->initFormatTable(backendContext.fDevice.Get());
    this->finishInitialization(options);
}

D3D11Caps::~D3D11Caps() = default;

void D3D11Caps::initCaps(const D3D11BackendContext& backendContext, const ContextOptions& options) {
    fResourceBindingReqs.fBackendApi = BackendApi::kD3D11;
    fResourceBindingReqs.fUniformBufferLayout = Layout::kStd140;
    fResourceBindingReqs.fStorageBufferLayout = Layout::kStd430;
    fResourceBindingReqs.fSeparateTextureAndSamplerBinding = false;
    fResourceBindingReqs.fUniformsSetIdx = D3D11GraphicsPipeline::kUniformBufferBindGroupIndex;
    fResourceBindingReqs.fTextureSamplerSetIdx = D3D11GraphicsPipeline::kTextureBindGroupIndex;
    fResourceBindingReqs.fIntrinsicBufferBinding = D3D11GraphicsPipeline::kIntrinsicUniformBufferIndex;
    fResourceBindingReqs.fCombinedUniformBufferBinding = D3D11GraphicsPipeline::kCombinedUniformIndex;
    fResourceBindingReqs.fStorageBufferBinding = D3D11GraphicsPipeline::kStorageBufferIndex;
    fResourceBindingReqs.fMaxFallbackTextureSize = 8192;
    fResourceBindingReqs.fMaxFallbackTextureBytes = 8192 * 8192 * 16;

    fDrawBufferCanBeMapped = false;
    fBufferMapsAreAsync = false;
    fStorageBufferSupport = true;
    fComputeSupport = false;

    fRequiredUniformBufferAlignment = 256;
    fRequiredStorageBufferAlignment = 16;
    fRequiredTransferBufferAlignment = 4;
    fMaxTextureSize = 16384;
}

void D3D11Caps::initShaderCaps() {
    SkSL::ShaderCaps* shaderCaps = fShaderCaps.get();
    shaderCaps->fDualSourceBlendingSupport = true;
    shaderCaps->fShaderDerivativeSupport = true;
    shaderCaps->fIntegerSupport = true;
    shaderCaps->fInfinitySupport = true;
    shaderCaps->fFlatInterpolationSupport = true;
}

void D3D11Caps::initFormatTable(ID3D11Device* device) {
    for (int i = 0; i < kTextureFormatCount; ++i) {
        TextureFormat tf = static_cast<TextureFormat>(i);
        DXGI_FORMAT format = TextureFormatToD3D11Format(tf);
        if (format == DXGI_FORMAT_UNKNOWN || !device) {
            continue;
        }

        UINT formatSupport = 0;
        HRESULT hr = device->CheckFormatSupport(format, &formatSupport);
        if (FAILED(hr) || !(formatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D)) {
            continue;
        }

        auto& [supportedUsage, supportedSampleCounts] = fFormatSupport[(int)Tiling::kOptimal][i];
        supportedUsage |= TextureUsage::kRead | TextureUsage::kCopySrc | TextureUsage::kCopyDst;
        supportedSampleCounts = SampleCount::k1;

        if (formatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) {
            supportedUsage |= TextureUsage::kSample;
        }
        if (formatSupport & (D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)) {
            supportedUsage |= TextureUsage::kRender;
        }

        if (formatSupport & (D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET |
                             D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)) {
            UINT quality = 0;
            if (SUCCEEDED(device->CheckMultisampleQualityLevels(format, 2, &quality)) && quality > 0) {
                supportedSampleCounts |= SampleCount::k2;
            }
            if (SUCCEEDED(device->CheckMultisampleQualityLevels(format, 4, &quality)) && quality > 0) {
                supportedSampleCounts |= SampleCount::k4;
            }
            if (SUCCEEDED(device->CheckMultisampleQualityLevels(format, 8, &quality)) && quality > 0) {
                supportedSampleCounts |= SampleCount::k8;
            }
        }
    }
}

TextureInfo D3D11Caps::onGetDefaultTextureInfo(SkEnumBitMask<TextureUsage> usage,
                                               TextureFormat format,
                                               SampleCount sampleCount,
                                               Mipmapped mipmapped,
                                               Protected isProtected,
                                               Discardable discardable) const {
    DXGI_FORMAT dxgiFormat = TextureFormatToD3D11Format(format);
    if (dxgiFormat == DXGI_FORMAT_UNKNOWN) {
        return {};
    }

    UINT bindFlags = 0;
    if (usage & (TextureUsage::kSample | TextureUsage::kRead)) {
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }
    if (usage & TextureUsage::kRender) {
        if (TextureFormatIsDepthOrStencil(format)) {
            bindFlags |= D3D11_BIND_DEPTH_STENCIL;
        } else {
            bindFlags |= D3D11_BIND_RENDER_TARGET;
        }
    }
    if (usage & TextureUsage::kStorage) {
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }

    D3D11TextureInfo info;
    info.fSampleCount = sampleCount;
    info.fMipmapped = mipmapped;
    info.fFormat = dxgiFormat;
    info.fViewFormat = dxgiFormat;
    info.fBindFlags = bindFlags;
    info.fUsage = D3D11_USAGE_DEFAULT;

    return TextureInfos::MakeD3D11(info);
}

std::pair<SkEnumBitMask<TextureUsage>, Tiling> D3D11Caps::getTextureUsage(
        const TextureInfo& info) const {
    const auto& d3dInfo = TextureInfoPriv::Get<D3D11TextureInfo>(info);
    SkEnumBitMask<TextureUsage> usage;
    if (d3dInfo.fBindFlags & D3D11_BIND_SHADER_RESOURCE) {
        usage |= TextureUsage::kSample | TextureUsage::kRead;
    }
    if (d3dInfo.fBindFlags & D3D11_BIND_RENDER_TARGET) {
        usage |= TextureUsage::kRender;
    }
    if (d3dInfo.fBindFlags & D3D11_BIND_DEPTH_STENCIL) {
        usage |= TextureUsage::kRender;
    }
    if (d3dInfo.fBindFlags & D3D11_BIND_UNORDERED_ACCESS) {
        usage |= TextureUsage::kStorage;
    }
    usage |= TextureUsage::kCopySrc | TextureUsage::kCopyDst;
    return {usage, Tiling::kOptimal};
}

void D3D11Caps::buildKeyForTexture(SkISize dimensions,
                                   const TextureInfo& info,
                                   ResourceType type,
                                   GraphiteResourceKey* key) const {
    const auto& d3dInfo = TextureInfoPriv::Get<D3D11TextureInfo>(info);
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(d3dInfo.getViewFormat() != DXGI_FORMAT_UNKNOWN);

    uint32_t formatKey = static_cast<uint32_t>(d3dInfo.getViewFormat());
    uint32_t samplesKey = SamplesToKey(info.sampleCount());
    bool isMipped = info.mipmapped() == Mipmapped::kYes;

    GraphiteResourceKey::Builder builder(key, type, 5);
    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = formatKey;
    builder[3] = (samplesKey << 0) | (static_cast<uint32_t>(isMipped) << 3) |
                 (static_cast<uint32_t>(d3dInfo.fUsage) << 4);
    builder[4] = d3dInfo.fBindFlags;
    builder.finish();
}

uint32_t D3D11Caps::getRenderPassDescKeyForPipeline(const RenderPassDesc& renderPassDesc) const {
    const auto& color = renderPassDesc.fColorAttachment;
    const auto& depthStencil = renderPassDesc.fDepthStencilAttachment;
    SkASSERT(color.fFormat != TextureFormat::kUnsupported);

    uint32_t colorKey =
            (static_cast<uint32_t>(color.fFormat) << kColorFormatOffset) |
            (SamplesToKey(color.fSampleCount) << kColorNumSamplesOffset);

    uint32_t depthStencilKey = 0;
    if (depthStencil.fFormat != TextureFormat::kUnsupported) {
        depthStencilKey =
                (static_cast<uint32_t>(depthStencil.fFormat) << kDepthStencilFormatOffset) |
                (SamplesToKey(depthStencil.fSampleCount) << kDepthStencilNumSamplesOffset);
    }

    return colorKey | depthStencilKey;
}

UniqueKey D3D11Caps::makeGraphicsPipelineKey(const GraphicsPipelineDesc& pipelineDesc,
                                             const RenderPassDesc& renderPassDesc) const {
    UniqueKey pipelineKey;
    {
        UniqueKey::Builder builder(&pipelineKey, kD3D11GraphicsPipelineDomain, 4, "GraphicsPipeline");
        builder[0] = static_cast<uint32_t>(pipelineDesc.renderStepID());
        builder[1] = pipelineDesc.paintParamsID().asUInt();
        builder[2] = this->getRenderPassDescKeyForPipeline(renderPassDesc);
        builder[3] = renderPassDesc.fWriteSwizzle.asKey();
        builder.finish();
    }
    return pipelineKey;
}

bool D3D11Caps::extractGraphicsDescs(const UniqueKey& key,
                                     GraphicsPipelineDesc* pipelineDesc,
                                     RenderPassDesc* renderPassDesc,
                                     const RendererProvider* rendererProvider) const {
    SkASSERT(key.domain() == kD3D11GraphicsPipelineDomain);
    const uint32_t* rawKeyData = key.data();

    RenderStep::RenderStepID renderStepID = static_cast<RenderStep::RenderStepID>(rawKeyData[0]);
    *pipelineDesc = GraphicsPipelineDesc(renderStepID, UniquePaintParamsID(rawKeyData[1]));

    const uint32_t rpDescBits = rawKeyData[2];
    TextureFormat colorFormat =
            static_cast<TextureFormat>((rpDescBits >> kColorFormatOffset) & kFormatMask);
    SampleCount colorSamples =
            KeyToSamples((rpDescBits >> kColorNumSamplesOffset) & kNumSamplesMask);

    TextureFormat depthStencilFormat =
            static_cast<TextureFormat>((rpDescBits >> kDepthStencilFormatOffset) & kFormatMask);
    SampleCount depthStencilSamples =
            KeyToSamples((rpDescBits >> kDepthStencilNumSamplesOffset) & kNumSamplesMask);

    *renderPassDesc = {};
    renderPassDesc->fColorAttachment = {colorFormat,
                                        LoadOp::kClear,
                                        StoreOp::kStore,
                                        colorSamples};
    if (depthStencilFormat != TextureFormat::kUnsupported) {
        renderPassDesc->fDepthStencilAttachment = {depthStencilFormat,
                                                   LoadOp::kClear,
                                                   StoreOp::kDiscard,
                                                   depthStencilSamples};
    }
    if (colorSamples > SampleCount::k1) {
        renderPassDesc->fColorResolveAttachment = {colorFormat,
                                                   LoadOp::kClear,
                                                   StoreOp::kStore,
                                                   SampleCount::k1};
        renderPassDesc->fColorAttachment.fStoreOp = StoreOp::kDiscard;
    }

    renderPassDesc->fSampleCount = colorSamples;
    renderPassDesc->fWriteSwizzle = SwizzleCtorAccessor::Make(rawKeyData[3]);
    renderPassDesc->fDstReadStrategy = this->getDstReadStrategy();

    return true;
}

UniqueKey D3D11Caps::makeComputePipelineKey(const ComputePipelineDesc& pipelineDesc) const {
    UniqueKey pipelineKey;
    {
        static const skgpu::UniqueKey::Domain kComputePipelineDomain = UniqueKey::GenerateDomain();
        UniqueKey::Builder builder(&pipelineKey, kComputePipelineDomain, 1, "ComputePipeline");
        builder[0] = 0;
        builder.finish();
    }
    return pipelineKey;
}

} // namespace skgpu::graphite
