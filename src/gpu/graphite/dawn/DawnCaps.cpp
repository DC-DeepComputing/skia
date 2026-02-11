/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnCaps.h"

#include <algorithm>
#include <string>

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/gpu/SwizzlePriv.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/sksl/SkSLUtil.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten/version.h>
#endif

namespace {

skgpu::UniqueKey::Domain get_pipeline_domain() {
    static const skgpu::UniqueKey::Domain kDawnGraphicsPipelineDomain =
            skgpu::UniqueKey::GenerateDomain();

    return kDawnGraphicsPipelineDomain;
}

#if !defined(__EMSCRIPTEN__)
bool IsMultiplanarFormat(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
            return true;
        default:
            return false;
    }
}
#endif
}  // anonymous namespace

namespace skgpu::graphite {

DawnCaps::DawnCaps(const DawnBackendContext& backendContext, const ContextOptions& options)
    : Caps() {
    this->initCaps(backendContext, options);
    this->initShaderCaps(backendContext.fDevice);
    this->finishInitialization(options);
}

DawnCaps::~DawnCaps() = default;

void DawnCaps::initCaps(const DawnBackendContext& backendContext, const ContextOptions& options) {
    // GetAdapter() is not available in WASM and there's no way to get AdapterInfo off of
    // the WGPUDevice directly.
#if !defined(__EMSCRIPTEN__)
    wgpu::AdapterInfo info;
    backendContext.fDevice.GetAdapter().GetInfo(&info);

#if defined(GPU_TEST_UTILS)
    this->setDeviceName(std::string(info.device));
#endif
#endif // defined(__EMSCRIPTEN__)

#if defined(__EMSCRIPTEN__)
    wgpu::SupportedLimits supportedLimits;
    // TODO(crbug.com/42241199): Update to use wgpu::Status when webgpu.h in Emscripten is updated.
    [[maybe_unused]] bool limitsSucceeded = backendContext.fDevice.GetLimits(&supportedLimits);
    SkASSERT(limitsSucceeded);
    wgpu::Limits& limits = supportedLimits.limits;
#else
    wgpu::CompatibilityModeLimits compatLimits;
    wgpu::Limits limits{.nextInChain = &compatLimits};
    wgpu::DawnTexelCopyBufferRowAlignmentLimits alignmentLimits{};
    if (backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment)) {
        compatLimits.nextInChain = &alignmentLimits;
    }
    [[maybe_unused]] wgpu::Status status = backendContext.fDevice.GetLimits(&limits);
    SkASSERT(status == wgpu::Status::Success);
#endif  // defined(__EMSCRIPTEN__)

    fMaxTextureSize = limits.maxTextureDimension2D;

    fRequiredTransferBufferAlignment = 4;
    fRequiredUniformBufferAlignment = limits.minUniformBufferOffsetAlignment;
    fRequiredStorageBufferAlignment = limits.minStorageBufferOffsetAlignment;

    fMaxVaryings = limits.maxInterStageShaderVariables;

    fFullCompressedUploadSizeMustAlignToBlockDims = true;
    // Dawn requires 256 bytes per row alignment for buffer texture copies.
    fTextureDataRowBytesAlignment = 256;
#if !defined(__EMSCRIPTEN__)
    // If the device supports the DawnTexelCopyBufferRowAlignment feature, the alignment can be
    // queried from its limits.
    if (backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment)) {
        fTextureDataRowBytesAlignment = alignmentLimits.minTexelCopyBufferRowAlignment;
    }
#endif

    fSupportsHalfPrecision = backendContext.fDevice.HasFeature(wgpu::FeatureName::ShaderF16);
    fResourceBindingReqs.fBackendApi = BackendApi::kDawn;
    // The WGSL generator assumes tightly packed std430 layout for SSBOs which is also the default
    // for all types outside the uniform address space in WGSL (which is emulated to 140 by SkSL's
    // WGSL generation). If ShaderF16 is supported, we switch the layout to upload half data.
    fResourceBindingReqs.fUniformBufferLayout = fSupportsHalfPrecision ? Layout::kStd140_F16
                                                                       : Layout::kStd140;
    fResourceBindingReqs.fStorageBufferLayout = fSupportsHalfPrecision ? Layout::kStd430_F16
                                                                       : Layout::kStd430;
    fResourceBindingReqs.fSeparateTextureAndSamplerBinding = true;

#if !defined(__EMSCRIPTEN__)
    // We need 32 bytes push constant for 2 vectors worth of intrinsic data.
    fResourceBindingReqs.fUsePushConstantsForIntrinsicConstants =
            limits.maxImmediateSize >= DawnGraphicsPipeline::kIntrinsicUniformSize;
#endif

    fResourceBindingReqs.fUniformsSetIdx = DawnGraphicsPipeline::kUniformBufferBindGroupIndex;
    fResourceBindingReqs.fTextureSamplerSetIdx = DawnGraphicsPipeline::kTextureBindGroupIndex;

    fResourceBindingReqs.fIntrinsicBufferBinding =
            DawnGraphicsPipeline::kIntrinsicUniformBufferIndex;
    fResourceBindingReqs.fCombinedUniformBufferBinding =
            DawnGraphicsPipeline::kCombinedUniformIndex;
    fResourceBindingReqs.fGradientBufferBinding = DawnGraphicsPipeline::kGradientBufferIndex;

#if !defined(__EMSCRIPTEN__)
    // We need at least 4 SSBOs for intrinsic, render step, paint & gradient buffers.
    // TODO(b/418235681): Enable SSBOs after fixing performance regressions for Dawn/Vulkan.
    fStorageBufferSupport = info.backendType != wgpu::BackendType::OpenGL &&
                            info.backendType != wgpu::BackendType::OpenGLES &&
                            info.backendType != wgpu::BackendType::Vulkan &&
                            compatLimits.maxStorageBuffersInVertexStage >= 4 &&
                            compatLimits.maxStorageBuffersInFragmentStage >= 4;
#else
    // WASM doesn't provide a way to query the backend, so can't tell if we are on a backend that
    // needs to have SSBOs disabled. Pessimistically assume we could be. Once the above conditions
    // go away in Dawn-native, then we can assume SSBOs are always supported in pure WebGPU too.
    fStorageBufferSupport = false;
#endif

    fDrawBufferCanBeMapped = false;

    fComputeSupport = true;

    // TODO: support clamp to border.
    fClampToBorderSupport = false;

#if defined(GPU_TEST_UTILS)
    fDrawBufferCanBeMappedForReadback = false;
#endif

#if defined(__EMSCRIPTEN__)
    // For wasm, we use async map.
    fBufferMapsAreAsync = true;
#else
    // For Dawn native, we use direct mapping.
    fBufferMapsAreAsync = false;
    fDrawBufferCanBeMapped =
            backendContext.fDevice.HasFeature(wgpu::FeatureName::BufferMapExtendedUsages);

    fMSAARenderToSingleSampledSupport =
            backendContext.fDevice.HasFeature(wgpu::FeatureName::MSAARenderToSingleSampled);

    if (backendContext.fDevice.HasFeature(wgpu::FeatureName::TransientAttachments)) {
        fSupportedTransientAttachmentUsage = wgpu::TextureUsage::TransientAttachment;
    }
#endif

#if !defined(__EMSCRIPTEN__)
    if (backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnLoadResolveTexture)) {
        fSupportedResolveTextureLoadOp = wgpu::LoadOp::ExpandResolveTexture;
        fSupportsPartialLoadResolve =
                backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnPartialLoadResolveTexture);
        fDifferentResolveAttachmentSizeSupport = fSupportsPartialLoadResolve;
    }
#endif

    if (!fSupportsPartialLoadResolve &&
        fSupportedTransientAttachmentUsage == wgpu::TextureUsage::None) {
        // If the device doesn't support partial resolve nor transient attachments, we will emulate
        // load/resolve using separate render passes. This helps reuse MSAA textures better to
        // reduce memory usage.
        fEmulateLoadStoreResolve = true;
        fDifferentResolveAttachmentSizeSupport = true;

        // On hardware that doesn't support transient attachments or partial resolve, we
        // force-disable the ExpandResolveTexture loadOp. This is done because, under emulation,
        // ExpandResolveTexture isn't used, and fully disabling it prevents the precompilation API
        // from generating duplicate pipeline permutations (one for load resolve texture, one for
        // others).
        fSupportedResolveTextureLoadOp = std::nullopt;
    }

    if (backendContext.fDevice.HasFeature(wgpu::FeatureName::TimestampQuery)) {
        // Native Dawn has an API for writing timestamps on command buffers. WebGPU only supports
        // begin and end timestamps on render and compute passes.
#if !defined(__EMSCRIPTEN__)
        // TODO(b/42240559): On Apple silicon, the timer queries don't have the correct dependencies
        // to measure all the encoders that the start/end commands encapsulate in the commandbuffer.
        // We would prefer to keep this API as it lets us measure our texture uploads. If either
        // this is fixed in Dawn, we can unconditionally take this approach for dawn-native; or
        // the WebGPU API can hopefully be extended to capture blit passes.
        fSupportsCommandBufferTimestamps = info.backendType != wgpu::BackendType::Metal;
#endif

        // The emscripten C/C++ interface before 3.1.48 for timestamp query writes on render and
        // compute passes is different than on current emsdk. The older API isn't correctly
        // translated to the current JS WebGPU API in emsdk. So we require 3.1.48+.
#if !defined(__EMSCRIPTEN__)                                                                   \
        || (__EMSCRIPTEN_major__ > 3)                                                          \
        || (__EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ > 1)                             \
        || (__EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ == 1 && __EMSCRIPTEN_tiny__ >= 48)
        fSupportedGpuStats |= GpuStatsFlags::kElapsedTime;
#endif
    }

    if (!backendContext.fTick) {
        fAllowCpuSync = false;
        // This seems paradoxical. However, if we use the async pipeline creation methods (e.g
        // Device::CreateRenderPipelineAsync) then we may have to synchronize before a submit that
        // uses the pipeline. If we use the methods that look synchronous (e.g.
        // Device::CreateRenderPipeline) they actually operate asynchronously on WebGPU but the
        // browser becomes responsible for synchronizing when we call submit.
        fUseAsyncPipelineCreation = false;

        // The implementation busy waits after popping.
        fAllowScopedErrorChecks = false;
    }
}

void DawnCaps::initShaderCaps(const wgpu::Device& device) {
    SkSL::ShaderCaps* shaderCaps = fShaderCaps.get();

    // WGSL does not support infinities regardless of hardware support. There are discussions around
    // enabling it using an extension in the future.
    shaderCaps->fInfinitySupport = false;

    // WGSL supports shader derivatives in the fragment shader
    shaderCaps->fShaderDerivativeSupport = true;

#if !defined(__EMSCRIPTEN__)
    if (device.HasFeature(wgpu::FeatureName::DualSourceBlending)) {
        shaderCaps->fDualSourceBlendingSupport = true;
    }
    if (device.HasFeature(wgpu::FeatureName::FramebufferFetch)) {
        shaderCaps->fFBFetchSupport = true;
    }
#endif
}

TextureFormatInfo DawnCaps::getFormatInfo(TextureFormat format) const {
    wgpu::TextureFormat dawnFormat = TextureFormatToDawnFormat(format);
    if (dawnFormat == wgpu::TextureFormat::Undefined) {
        return {};
    }

    SkEnumBitMask<SampleCount> sampleCounts = SampleCount::k1;
    SkEnumBitMask<FormatSupport> supports;
    if (TextureFormatIsDepthOrStencil(format)) {
        // Depth/Stencil formats are renderable and support MSAA
        // FIXME sample count seems redundant with both of these support flags for formats?
        //   e.g. empty bitmask for not renderable, k1 for renderable but no msaa, k1 + others for msaa
        // well not really, since we still want k1 to be reported for sample count for non-renderable formats
        supports = FormatSupport::kMSAA | FormatSupport::kRender;
    } else {
        // Check extensions for some wgpu::TextureFormats
        switch (format) {
            case TextureFormat::kRGBA8_BC1:
            case TextureFormat::kRGBA8_BC1_sRGB:
                if (!device.HasFeature(wgpu::FeatureName::TextureCompressionBC)) {
                    return {};
                }
                break;
            case TextureFormat::kRGB8_ETC2:
            case TextureFormat::kRGB8_ETC2_sRGB:
                if (!device.HasFeature(wgpu::FeatureName::TextureCompressionETC2)) {
                    return {};
                }
                break;
                // wasm guard for unorm16?
            case TextureFormat::kR16:
            case TextureFormat::kRG16:
            case TextureFormat::kRGB16:
            case TextureFormat::kRGBA16:
                if (!device.HasFeature(wgpu::FeatureName::Unorm16TextureFormats)) {
                    return {};
                }
                break;
        }
        // kExternal WASM guard?

            // TODO(crbug.com/dawn/1856): Support storage binding for compute shader in Dawn.



        // Color formats always support CopyDst, CopySrc, and Sample. Uncompressed and non
        // multiplanar formats can be rendered into (with MSAA).
        supports = FormatSupport::kCopyDst | FormatSupport::kCopySrc;
        if (!TextureFormatIsMultiplanar(format) &&
            TextureFormatCompressionType(format) == SkTextureCompressionType::kNone) {
            supports |= FormatSupport::kMSAA | FormatSupport::kRender;
        }
    }

    if (supports & FormatSupport::kMSAA) {
        sampleCounts |= SampleCount::k4;
    }

    return TextureFormatInfo(format,
                             /*optimalSupport=*/{supports, sampleCounts},
                             /*linearSupport=*/{{}, {}});
}

SkEnumBitMask<FormatSupport> DawnCaps::supportedOperations(const TextureInfo& textureInfo) const {
    const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(textureInfo);

    SkEnumBitMask<FormatSupport> supports;
    if (dawnInfo.fUsage & wgpu::TextureUsage::CopyDst) {
        supports |= FormatSupport::kCopyDst;
    }
    if (dawnInfo.fUsage & wgpu::TextureUsage::CopySrc) {
        supports |= FormatSupport::kCopySrc;
    }
    if (dawnInfo.fUsage & wgpu::TextureUsage::TextureBinding) {
        supports |= FormatSupport::kSample;

        // FIXME needed?
#if !defined(__EMSCRIPTEN__)
    switch (dawnInfo.fFormat) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm: {
            if (dawnInfo.fAspect == wgpu::TextureAspect::Plane0Only &&
                dawnInfo.getViewFormat() != wgpu::TextureFormat::R8Unorm) {
                return false;
            }
            if (dawnInfo.fAspect == wgpu::TextureAspect::Plane1Only &&
                dawnInfo.getViewFormat() != wgpu::TextureFormat::RG8Unorm) {
                return false;
            }
            break;
        }
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm: {
            if (dawnInfo.fAspect == wgpu::TextureAspect::Plane0Only &&
                dawnInfo.getViewFormat() != wgpu::TextureFormat::R16Unorm) {
                return false;
            }
            if (dawnInfo.fAspect == wgpu::TextureAspect::Plane1Only &&
                dawnInfo.getViewFormat() != wgpu::TextureFormat::RG16Unorm) {
                return false;
            }
            break;
        }
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm: {
            if (dawnInfo.fAspect == wgpu::TextureAspect::Plane0Only &&
                dawnInfo.getViewFormat() != wgpu::TextureFormat::R8Unorm) {
                return false;
            }
            if (dawnInfo.fAspect == wgpu::TextureAspect::Plane1Only &&
                dawnInfo.getViewFormat() != wgpu::TextureFormat::RG8Unorm) {
                return false;
            }
            if (dawnInfo.fAspect == wgpu::TextureAspect::Plane2Only &&
                dawnInfo.getViewFormat() != wgpu::TextureFormat::R8Unorm) {
                return false;
            }
            break;
        }
        default:
            break;
    }
#endif

    }
    if (dawnInfo.fUsage & wgpu::TextureUsage::StorageBinding) {
        // FIXME require StorageAttachment? it wasn't historically
        supports |= FormatSupport::kStorage;
    }
    if (dawnInfo.fUsage & wgpu::TextureUsage::RenderAttachment) {
        supports |= FormatSupport::kRender | FormatSupport::kMSAA;
    }

    return supports;
}

TextureInfo DawnCaps::getDefaultTextureInfo(TextureFormat format,
                                            Mipmapped mipmapped,
                                            SampleCount sampleCount,
                                            SkEnumBitMask<FormatSupport> supports,
                                            Protected,
                                            Discardable discardable) {
    // if (!this->isSampleCountSupported(desc.fFormat, desc.fSampleCount)) {
    //     return {};
    // }


    // FIXME what about MSAARTSS?
    wgpu::TextureUsage usage = wgpu::TextureUsage::None;
    if (supports & FormatSupport::kRender) {
        usage |= wgpu::TextureUsage::RenderAttachment;
        if (fSupportedTransientAttachmentUsage != wgpu::TextureUsage::None &&
            discardable == Discardable::kYes) {
            usage |= fSupportedTransientAttachmentUsage;
        }
        if (fEmulateLoadStoreResolve && !TextureFormatIsDepthOrStencil(format)) {
            // When emulating the store, the color attachment is sampled into the resolve.
            usage |= wgpu::TextureUsage::TextureBinding;
        }
    }
    if (supports & FormatSupport::kCopyDst) {
        usage |= wgpu::TextureUsage::CopyDst;
    }
    if (supports & FormatSupport::kCopySrc) {
        usage |= wgpu::TextureUsage::CopySrc;
    }
    if (supports & FormatSupport::kSample) {
        usage |= wgpu::TextureUsage::TextureBinding;
    }
    if (supports & FormatSupport::kStorage) {
        usage |= wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding;
    }

    DawnTextureInfo info;
    info.fSampleCount = sampleCount;
    info.fMipmapped   = mipmapped;
    info.fFormat      = TextureFormatToDawnFormat(format);
    info.fViewFormat = info.fFormat; // FIXME?
    info.fUsage       = usage;



    return TextureInfos::MakeDawn(info);
}

SkISize DawnCaps::getDepthAttachmentDimensions(const TextureInfo& textureInfo,
                                               const SkISize colorAttachmentDimensions) const {
#if !defined(__EMSCRIPTEN__)
    // For multiplanar textures, texture->textureInfo() uses the format of planes instead of
    // textures (R8, R8G8, vs R8BG8Biplanar420Unorm), so we have to query texture format from
    // wgpu::Texture object, and then use it reconstruct the full dimensions.
    const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(textureInfo);
    wgpu::TextureFormat format = dawnInfo.fFormat;
    if (IsMultiplanarFormat(format) && dawnInfo.fAspect == wgpu::TextureAspect::Plane1Only) {
        // Dawn requires depth attachment to match the size of Y plane (texture size).
        return SkISize::Make(colorAttachmentDimensions.width() * 2,
                             colorAttachmentDimensions.height() * 2);
    }
#endif

    return colorAttachmentDimensions;
}

// TextureFormat is backed by a uint8_t, so 8 bits are always sufficient (including using
// kUnsupported) to represent an unused attachment. To make room for the load-from-resolve bit, we
// reduce the uint8_t of fSampleCount to 3 bits with the SampleToKey function (x2 attachments)
static constexpr int kFormatBits = 8; // x2 attachments (color & depthStencil formats)
static constexpr int kResolveBits = 1;

static_assert(2*(kFormatBits + kNumSampleKeyBits) + kResolveBits <= 32);
static_assert(kTextureFormatCount < 1 << kFormatBits);

static constexpr int kDepthStencilNumSamplesOffset = /*loadResolveOffset=0 + */   kResolveBits;
static constexpr int kDepthStencilFormatOffset  = kDepthStencilNumSamplesOffset + kNumSampleKeyBits;
static constexpr int kColorNumSamplesOffset     = kDepthStencilFormatOffset     + kFormatBits;
static constexpr int kColorFormatOffset         = kColorNumSamplesOffset        + kNumSampleKeyBits;
static constexpr int kAdditionalFlagOffset      = kColorFormatOffset            + kFormatBits;
static_assert(kAdditionalFlagOffset <= 31);

static constexpr uint32_t kFormatMask     = (1 << kFormatBits) - 1;
static constexpr uint32_t kNumSamplesMask = (1 << kNumSampleKeyBits) - 1;
static constexpr uint32_t kResolveMask    = (1 << kResolveBits) - 1;

uint32_t DawnCaps::getRenderPassDescKeyForPipeline(const RenderPassDesc& renderPassDesc,
                                                   bool additionalFlag) const {
    // The color attachment should be valid; the depth-stencil attachment may not be if it's not
    // being used. The full resolve attachment (if present) does not need to be included.
    const auto& color = renderPassDesc.fColorAttachment;
    const auto& depthStencil = renderPassDesc.fDepthStencilAttachment;
    SkASSERT(color.fFormat != TextureFormat::kUnsupported);

    // Note: if Dawn supports ExpandResolveTexture load op and the render pass uses it to load the
    // resolve texture, a render pipeline will need to be created with
    // wgpu::ColorTargetStateExpandResolveTextureDawn chained struct in order to be compatible.
    // Hence a render pipeline created for a render pass using ExpandResolveTexture load op will be
    // different from the one created for a render pass not using that load op. So we need to
    // include a bit flag to differentiate the two kinds of pipelines.
    const bool shouldIncludeLoadResolveAttachmentBit = this->loadOpAffectsMSAAPipelines();
    uint32_t loadResolveAttachmentKey = 0;
    if (shouldIncludeLoadResolveAttachmentBit &&
        renderPassDesc.fColorResolveAttachment.fFormat != TextureFormat::kUnsupported &&
        renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad) {
        loadResolveAttachmentKey = 1;
    }

    SkASSERT(SamplesToKey(color.fSampleCount) < (1 << kNumSampleKeyBits));
    SkASSERT(SamplesToKey(depthStencil.fSampleCount) < (1 << kNumSampleKeyBits));
    SkASSERT(loadResolveAttachmentKey < (1 << kResolveBits));
    uint32_t additionalFlagKey = additionalFlag ? 1 : 0;

    return (additionalFlagKey                           << kAdditionalFlagOffset)         |
           (static_cast<uint32_t>(color.fFormat)        << kColorFormatOffset)            |
           (SamplesToKey(color.fSampleCount)            << kColorNumSamplesOffset)        |
           (static_cast<uint32_t>(depthStencil.fFormat) << kDepthStencilFormatOffset)     |
           (SamplesToKey(depthStencil.fSampleCount)     << kDepthStencilNumSamplesOffset) |
           loadResolveAttachmentKey;
}

static constexpr int kDawnGraphicsPipelineKeyData32Count = 4;

UniqueKey DawnCaps::makeGraphicsPipelineKey(const GraphicsPipelineDesc& pipelineDesc,
                                            const RenderPassDesc& renderPassDesc) const {
    UniqueKey pipelineKey;
    {
        // 4 uint32_t's (render step id, paint id, uint32 RenderPassDesc, uint16 write swizzle key)
        UniqueKey::Builder builder(&pipelineKey, get_pipeline_domain(),
                                   kDawnGraphicsPipelineKeyData32Count, "DawnGraphicsPipeline");
        // Add GraphicsPipelineDesc key.
        builder[0] = static_cast<uint32_t>(pipelineDesc.renderStepID());
        builder[1] = pipelineDesc.paintParamsID().asUInt();

        // Add RenderPassDesc key and write swizzle (which is separate from the RenderPassDescKey
        // because it is applied in the program writing to the target, and is not actually part of
        // the underlying GPU render pass config).
        builder[2] = this->getRenderPassDescKeyForPipeline(renderPassDesc);
        builder[3] = renderPassDesc.fWriteSwizzle.asKey();
        builder.finish();
    }

    return pipelineKey;
}

bool DawnCaps::extractGraphicsDescs(const UniqueKey& key,
                                    GraphicsPipelineDesc* pipelineDesc,
                                    RenderPassDesc* renderPassDesc,
                                    const RendererProvider* rendererProvider) const {
    SkASSERT(key.domain() == get_pipeline_domain());
    SkASSERT(key.dataSize() == 4 * kDawnGraphicsPipelineKeyData32Count);

    const uint32_t* rawKeyData = key.data();

    SkASSERT(RenderStep::IsValidRenderStepID(rawKeyData[0]));
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

    const bool loadFromResolve = (rpDescBits & kResolveMask) != 0;
    // This bit should only be set if Dawn supports ExpandResolveTexture load op
    SkASSERT(!loadFromResolve || this->loadOpAffectsMSAAPipelines());

    // Recreate the RenderPassDesc, assuming that if there is MSAA on the color attachment that
    // a resolve attachment is also required. The resolve attachment's load op will match the key.
    SkASSERT(colorSamples == depthStencilSamples ||
             depthStencilFormat == TextureFormat::kUnsupported);
    *renderPassDesc = {};
    renderPassDesc->fColorAttachment = {colorFormat,
                                        LoadOp::kClear,
                                        StoreOp::kStore,
                                        colorSamples};
    renderPassDesc->fDepthStencilAttachment = {depthStencilFormat,
                                               LoadOp::kClear,
                                               StoreOp::kDiscard,
                                               depthStencilSamples};
    if (colorSamples > SampleCount::k1) {
        renderPassDesc->fColorResolveAttachment = {colorFormat,
                                                   loadFromResolve ? LoadOp::kLoad : LoadOp::kClear,
                                                   StoreOp::kStore,
                                                   SampleCount::k1};
        renderPassDesc->fColorAttachment.fStoreOp = StoreOp::kDiscard;
    }

    renderPassDesc->fSampleCount = colorSamples;
    renderPassDesc->fWriteSwizzle = SwizzleCtorAccessor::Make(rawKeyData[3]);
    renderPassDesc->fDstReadStrategy = this->getDstReadStrategy();

    return true;
}

UniqueKey DawnCaps::makeComputePipelineKey(const ComputePipelineDesc& pipelineDesc) const {
    UniqueKey pipelineKey;
    {
        static const skgpu::UniqueKey::Domain kComputePipelineDomain = UniqueKey::GenerateDomain();
        // The key is made up of a single uint32_t corresponding to the compute step ID.
        UniqueKey::Builder builder(&pipelineKey, kComputePipelineDomain, 1, "ComputePipeline");
        builder[0] = pipelineDesc.computeStep()->uniqueID();

        // TODO(b/240615224): The local work group size should factor into the key here since it is
        // specified in the shader text on Dawn/SPIR-V. This is not a problem right now since
        // ComputeSteps don't vary their workgroup size dynamically.

        builder.finish();
    }
    return pipelineKey;
}

ImmutableSamplerInfo DawnCaps::getImmutableSamplerInfo(const TextureInfo& textureInfo) const {
#if !defined(__EMSCRIPTEN__)
    const wgpu::YCbCrVkDescriptor& ycbcrConversionInfo =
            TextureInfoPriv::Get<DawnTextureInfo>(textureInfo).fYcbcrVkDescriptor;

    if (DawnDescriptorIsValid(ycbcrConversionInfo)) {
        return DawnDescriptorToImmutableSamplerInfo(ycbcrConversionInfo);
    }
#endif

    // If the YCbCr conversion for is invalid, then return a default ImmutableSamplerInfo struct.
    return {};
}

#if !defined(__EMSCRIPTEN__)
static constexpr const char* filter_mode_to_str(wgpu::FilterMode mode) {
    switch (mode) {
        case wgpu::FilterMode::Undefined: return "undefined";
        case wgpu::FilterMode::Nearest:   return "nearest";
        case wgpu::FilterMode::Linear:    return "linear";
    }
    SkUNREACHABLE;
}

static constexpr const char* model_to_str(uint32_t c) {
    switch (c) {
        case 0 /* VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY */:   return "RGB-I";
        case 1 /* VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY */: return "YCbCr-I";
        case 2 /* VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709 */:      return "709";
        case 3 /* VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601 */:      return "601";
        case 4 /* VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020 */:     return "2020";
        default:                                                       return "unknown";
    }
    SkUNREACHABLE;
}

static constexpr const char* range_to_str(uint32_t r) {
    switch (r) {
        case 0 /* VK_SAMPLER_YCBCR_RANGE_ITU_FULL */:   return "full";
        case 1 /* VK_SAMPLER_YCBCR_RANGE_ITU_NARROW */: return "narrow";
        default:                                        return "unknown";
    }
    SkUNREACHABLE;
}

static constexpr char swizzle_to_str(uint32_t c, char identityAnswer) {
    switch (c) {
        case 0 /* VK_COMPONENT_SWIZZLE_IDENTITY */: return identityAnswer;
        case 1 /* VK_COMPONENT_SWIZZLE_ZERO */:     return '0';
        case 2 /* VK_COMPONENT_SWIZZLE_ONE */:      return '1';
        case 3 /* VK_COMPONENT_SWIZZLE_R */:        return 'r';
        case 4 /* VK_COMPONENT_SWIZZLE_G */:        return 'g';
        case 5 /* VK_COMPONENT_SWIZZLE_B */:        return 'b';
        case 6 /* VK_COMPONENT_SWIZZLE_A */:        return 'a';
        default:                                    return '?';
    }
    SkUNREACHABLE;
}
#endif

std::string DawnCaps::toString(const ImmutableSamplerInfo& immutableSamplerInfo) const {
#if defined(__EMSCRIPTEN__)
    return "";
#else
    const wgpu::YCbCrVkDescriptor info =
                DawnDescriptorFromImmutableSamplerInfo(immutableSamplerInfo);
    if (!DawnDescriptorIsValid(info)) {
        return "";
    }

    std::string result;

    if (info.vkFormat == 0) {
        result += 'x';
        result += std::to_string(info.externalFormat);
    } else {
        result += std::to_string(info.vkFormat);
    }

    result += " ";
    result += model_to_str(info.vkYCbCrModel);
    result += "+";
    result += range_to_str(info.vkYCbCrRange);
    result += info.vkXChromaOffset ? " mid"  : " cos";  // midpoint or cosited-even
    result += info.vkYChromaOffset ? " mid " : " cos "; // midpoint or cosited-even
    result += filter_mode_to_str(info.vkChromaFilter);
    result += info.forceExplicitReconstruction ? " T " : " F ";
    result += swizzle_to_str(info.vkComponentSwizzleRed,   'r');
    result += swizzle_to_str(info.vkComponentSwizzleGreen, 'g');
    result += swizzle_to_str(info.vkComponentSwizzleBlue,  'b');
    result += swizzle_to_str(info.vkComponentSwizzleAlpha, 'a');
    return result;
#endif
}

void DawnCaps::buildKeyForTexture(SkISize dimensions,
                                  const TextureInfo& info,
                                  ResourceType type,
                                  GraphiteResourceKey* key) const {
    const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(info);

    SkASSERT(!dimensions.isEmpty());

    SkASSERT(dawnInfo.getViewFormat() != wgpu::TextureFormat::Undefined);
    // FIXME we can reduce this by packing format into samplesKey and then we're back down to 5 ints
    // we could go further if we said textures were likely to be under 65kx65kf...
    uint32_t formatKey = static_cast<uint32_t>(dawnInfo.getViewFormat());

    uint32_t samplesKey = SamplesToKey(info.sampleCount());
    // We don't have to key the number of mip levels because it is inherit in the combination of
    // isMipped and dimensions.
    bool isMipped = info.mipmapped() == Mipmapped::kYes;

    // Confirm all the below parts of the key can fit in a single uint32_t. The sum of the shift
    // amounts in the asserts must be less than or equal to 32.
    SkASSERT(samplesKey                             < (1u << 3));  // sample key is first 3 bits
    SkASSERT(static_cast<uint32_t>(isMipped)        < (1u << 1));  // isMapped is 4th bit
    SkASSERT(static_cast<uint32_t>(dawnInfo.fUsage) < (1u << 28)); // usage is remaining 28 bits

    // We need two uint32_ts for dimensions, 1 for format, and 1 for the rest of the key;
    int num32DataCnt = 2 + 1 + 1;
    bool hasYcbcrInfo = false;
#if !defined(__EMSCRIPTEN__)
    // If we are using ycbcr texture/sampling, more key information is needed.
    if ((hasYcbcrInfo = DawnDescriptorIsValid(dawnInfo.fYcbcrVkDescriptor))) {
        num32DataCnt += 3; // non-format flags and 64-bit format
    }
#endif
    GraphiteResourceKey::Builder builder(key, type, num32DataCnt);

    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = formatKey;
    builder[3] = (samplesKey                                   << 0) |
                 (static_cast<uint32_t>(isMipped)              << 3) |
                 (static_cast<uint32_t>(dawnInfo.fUsage)       << 4);

#if !defined(__EMSCRIPTEN__)
    if (hasYcbcrInfo) {
        ImmutableSamplerInfo packedInfo =
                DawnDescriptorToImmutableSamplerInfo(dawnInfo.fYcbcrVkDescriptor);
        builder[4] = packedInfo.fNonFormatYcbcrConversionInfo;
        // Even though we already have formatKey appended to the texture key, we still need to add
        // fYcbcrVkDescriptor's vkFormat or externalFormat. The latter two are distinct from
        // dawnSpec's wgpu::TextureFormat.
        builder[5] = (uint32_t) packedInfo.fFormat;
        builder[6] = (uint32_t) (packedInfo.fFormat >> 32);
    }
#endif
}

} // namespace skgpu::graphite
