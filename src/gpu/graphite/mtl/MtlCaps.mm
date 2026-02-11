/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlCaps.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "src/gpu/SwizzlePriv.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/mtl/MtlGraphicsPipeline.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu::graphite {

MtlCaps::MtlCaps(const id<MTLDevice> device, const ContextOptions& options)
        : Caps() {
    this->initGPUFamily(device);
    this->initCaps(device);
    this->initShaderCaps();
    this->finishInitialization(options);
}

bool MtlCaps::GetGPUFamily(id<MTLDevice> device, GPUFamily* gpuFamily, int* group) {
#if SKGPU_GRAPHITE_METAL_SDK_VERSION >= 220
    if (@available(macOS 10.15, iOS 13.0, tvOS 13.0, *)) {
        // Apple Silicon
#if SKGPU_GRAPHITE_METAL_SDK_VERSION >= 230
        if ([device supportsFamily:MTLGPUFamilyApple7]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 7;
            return true;
        }
#endif
#ifdef SK_BUILD_FOR_IOS
        if ([device supportsFamily:MTLGPUFamilyApple6]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 6;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple5]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 5;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple4]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 4;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple3]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 3;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple2]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 2;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple1]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 1;
            return true;
        }
#endif

        // Older Macs
        // MTLGPUFamilyMac1, MTLGPUFamilyMacCatalyst1, and MTLGPUFamilyMacCatalyst2 are deprecated.
        // However, some MTLGPUFamilyMac1 only hardware is still supported.
        // MacCatalyst families have the same features as Mac, so treat them the same
        //
        // Check if an Intel GPU is present; allow targeting issues specific to that hardware.
        bool isIntel = [device.name containsString:@"Intel"];
        if ([device supportsFamily:MTLGPUFamilyMac2] ||
            [device supportsFamily:(MTLGPUFamily)4002/*MTLGPUFamilyMacCatalyst2*/]) {
            *gpuFamily = isIntel ? GPUFamily::kMacIntel : GPUFamily::kMac;
            *group = 2;
            return true;
        }
        if ([device supportsFamily:(MTLGPUFamily)2001/*MTLGPUFamilyMac1*/] ||
            [device supportsFamily:(MTLGPUFamily)4001/*MTLGPUFamilyMacCatalyst1*/]) {
            *gpuFamily = isIntel ? GPUFamily::kMacIntel : GPUFamily::kMac;
            *group = 1;
            return true;
        }
    }
#endif

    // No supported GPU families were found
    return false;
}

void MtlCaps::initGPUFamily(id<MTLDevice> device) {
    if (@available(macOS 10.15, iOS 13.0, tvOS 13.0, *)) {
        if (GetGPUFamily(device, &fGPUFamily, &fFamilyGroup)) {
            return;
        }
    }

    // We don't know what this is, fall back to minimum defaults
#ifdef SK_BUILD_FOR_MAC
    fGPUFamily = GPUFamily::kMac;
    fFamilyGroup = 1;
#else
    fGPUFamily = GPUFamily::kApple;
    fFamilyGroup = 1;
#endif
}

void MtlCaps::initCaps(const id<MTLDevice> device) {
#if defined(GPU_TEST_UTILS)
    this->setDeviceName([[device name] UTF8String]);
#endif

    if (this->isMac() || fFamilyGroup >= 3) {
        fMaxTextureSize = 16384;
    } else {
        fMaxTextureSize = 8192;
    }

    // We use constant address space for our uniform buffers which has various alignment
    // requirements for the offset when binding the buffer. On MacOS Intel the offset must align
    // to 256. On iOS or Apple Silicon we must align to the max of the data type consumed by the
    // vertex function or 4 bytes, or we can ignore the data type and just use 16 bytes.
    //
    // On Mac, all copies must be aligned to at least 4 bytes; on iOS there is no alignment.
    if (this->isMac()) {
        fRequiredUniformBufferAlignment = 256;
        fRequiredTransferBufferAlignment = 4;
    } else {
        fRequiredUniformBufferAlignment = 16;
        fRequiredTransferBufferAlignment = 1;
    }

    fResourceBindingReqs.fBackendApi = BackendApi::kMetal;
    fResourceBindingReqs.fUniformBufferLayout = Layout::kMetal;
    fResourceBindingReqs.fStorageBufferLayout = Layout::kMetal;

    // Graphite/Metal does not group resources into different sets or bind groups at this time,
    // though ResourceBindingRequirements still expects valid assignments of these indices.
    // Assigning both to 0 conveys the usage of one single "set" for all resources.
    fResourceBindingReqs.fUniformsSetIdx = 0;
    fResourceBindingReqs.fTextureSamplerSetIdx = 0;

    fResourceBindingReqs.fComputeUsesDistinctIdxRangesForTextures = true;

    fResourceBindingReqs.fIntrinsicBufferBinding =
            MtlGraphicsPipeline::kIntrinsicUniformBufferIndex;
    fResourceBindingReqs.fCombinedUniformBufferBinding = MtlGraphicsPipeline::kCombinedUniformIndex;
    fResourceBindingReqs.fGradientBufferBinding = MtlGraphicsPipeline::kGradientBufferIndex;

    // Metal does not distinguish between uniform and storage buffers.
    fRequiredStorageBufferAlignment = fRequiredUniformBufferAlignment;

    fStorageBufferSupport = true;

    fComputeSupport = true;

    // See https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf, and what Dawn does at
    // https://crsrc.org/c/third_party/dawn/src/dawn/native/metal/PhysicalDeviceMTL.mm?q=maxInterStageShaderVariables
    if (this->isMac() || fFamilyGroup >= 4) {
        fMaxVaryings = 31;
    } else {
        fMaxVaryings = 15;
    }

    if (@available(macOS 10.12, iOS 14.0, tvOS 14.0, *)) {
        fClampToBorderSupport = (this->isMac() || fFamilyGroup >= 7);
    } else {
        fClampToBorderSupport = false;
    }

    // Init sample counts. All devices support 1
    fSupportedSampleCounts = SampleCount::k1;
    if (!this->isIntel()) {
        if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
            for (auto sampleCnt : {SampleCount::k2, SampleCount::k4, SampleCount::k8}) {
                if ([device supportsTextureSampleCount: (uint8_t) sampleCnt]) {
                    fSupportedSampleCounts |= sampleCnt;
                }
            }
        }
    } else {
        // All supported Intel Macs are of an old enough Intel generation that we can just assume
        // MSAA should be avoided instead of checking its generation. (>gen 11 FIXME?)
        fAvoidMSAA = true;
    }
}

void MtlCaps::initShaderCaps() {
    SkSL::ShaderCaps* shaderCaps = fShaderCaps.get();

    // Dual source blending requires Metal 1.2.
    if (@available(macOS 10.12, iOS 10.0, tvOS 10.0, *)) {
        shaderCaps->fDualSourceBlendingSupport = true;
    }

    shaderCaps->fFlatInterpolationSupport = true;
    shaderCaps->fShaderDerivativeSupport = true;
    shaderCaps->fInfinitySupport = true;

    if (@available(macOS 11.0, *)) {
        if (this->isApple()) {
            shaderCaps->fFBFetchSupport = true;
            shaderCaps->fFBFetchColorName = "sk_LastFragColor";
        }
    }

    if (this->isIntel()) {
        shaderCaps->fVectorClampMinMaxSupport = false;
    }

    shaderCaps->fIntegerSupport = true;
    shaderCaps->fNonsquareMatrixSupport = true;
    shaderCaps->fInverseHyperbolicSupport = true;

    // Metal uses IEEE floats so assuming those values here.
    shaderCaps->fFloatIs32Bits = true;
}

TextureFormatInfo MtlCaps::getFormatInfo(TextureFormat format) const {
    MTLPixelFormat mtlFormat = TextureFormatToMTLPixelFormat(format);
    if (mtlFormat == MTLPixelFormatInvalid) {
        return {};
    }
    SkEnumBitMask<FormatSupport> support = MTLPixelFormatSupport(fGPUFamily, mtlFormat);
    // FIXME need to check device support somehow for D24S8 support...
    // Maybe just store it as a boolean since it's the only thing that needs a device query?
    SkEnumBitMask<SampleCount> sampleCounts;
    if (support & FormatSupport::kMSAA) {
        sampleCounts = fSupportedSampleCounts;
    }

    // For now we only support optimal tiling, no linear tiling (e.g. TextureBuffers)
    return {format,
            /*optimalSupport=*/{support, sampleCounts},
            /*linearSupport=*/{{}, {}}};
}

SkEnumBitMask<FormatSupport> MtlCaps::supportedOperations(const TextureInfo& texInfo) const {
    const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(texInfo);

    SkEnumBitMask<FormatSupport> supports;
    if (!mtlInfo.fFramebufferOnly) {
        // Can read and write
        supports |= FormatSupport::kCopySrc | FormatSupport::kCopyDst;
        if (mtlInfo.fUsage & MTLTextureUsageShaderRead) {
            supports |= FormatSupport::kSample; // Assuming format supports filtering
            if (mtlInfo.fUsage & MTLTextureUsageShaderWrite) {
                supports |= FormatSupport::kStorage;
            }
        }
    }

    if (mtlInfo.fUsage & MTLTextureUsageRenderTarget) {
        supports |= FormatSupport::kRender;
        // FIXME MSAA feels awkward here... also do we validate that the sample count is supported?
        if (texInfo.sampleCount() > SampleCount::k1) {
            supports |= FormatSupport::kMSAA;
        }
    }

    return supports;
}

// bool MtlCaps::isSampleCountSupported(TextureFormat format, SampleCount requestedSampleCount) const {
//     const FormatInfo& formatInfo = this->getFormatInfo(TextureFormatToMTLPixelFormat(format));
//     if (!SkToBool(formatInfo.fFlags & FormatInfo::kRenderable_Flag)) {
//         return false;
//     }
//     if (SkToBool(formatInfo.fFlags & FormatInfo::kMSAA_Flag)) {
//         return SkToBool(fSupportedSampleCounts & requestedSampleCount);
//     } else {
//         // Only single sampling is supported for the format, so 1 sample should be generally
//         // available, too.
//         SkASSERT(SkToBool(fSupportedSampleCounts & SampleCount::k1));
//         return requestedSampleCount == SampleCount::k1;
//     }
// }

// TextureFormat MtlCaps::getDepthStencilFormat(SkEnumBitMask<DepthStencilFlags> mask) const {
//     if (mask == DepthStencilFlags::kDepth) {
//         // Graphite only needs 16-bits for depth values, so save some memory. If needed for
//         // workarounds, MTLPixelFormatDepth32Float is also available.
//         return TextureFormat::kD16;
//     } else if (mask == DepthStencilFlags::kStencil) {
//         return TextureFormat::kS8;
//     } else if (mask == DepthStencilFlags::kDepthStencil) {
// #if defined(SK_BUILD_FOR_MAC)
//         if (SkToBool(this->getFormatInfo(MTLPixelFormatDepth24Unorm_Stencil8).fFlags)) {
//             return TextureFormat::kD24_S8;
//         }
// #endif
//         return TextureFormat::kD32F_S8;
//     }
//     return TextureFormat::kUnsupported;
// }

TextureInfo MtlCaps::getDefaultTextureInfo(TextureFormat format,
                                           Mipmapped mipmapped,
                                           SampleCount sampleCount,
                                           SkEnumBitMask<FormatSupport> supports,
                                           Protected,
                                           Discardable discardable) {
    // FIXME check for support? or assume already done by Caps?
    // if (!this->isSampleCountSupported(desc.fFormat, desc.fSampleCount)) {
    //     return {};
    // }

    // Default to private in the event it's not discardable or memoryless is not available
    MTLStorageMode storageMode = MTLStorageModePrivate;

    MTLTextureUsage usage = MTLTextureUsageUnknown;
    if (supports & FormatSupport::kRender) {
        usage |= MTLTextureUsageRenderTarget;

        // Try to use memoryless if it's available (only on new Apple silicon)
        // FIXME assert anything here?
        if (discardable == Discardable::kYes && this->isApple()) {
            if (@available(macOS 11.0, iOS 10.0, tvOS 10.0, *)) {
                storageMode = MTLStorageModeMemoryless;
            }
        }

    } // FIXME MSAA is a weird support bit here too
    if (supports & FormatSupport::kSample) {
        usage |= MTLTextureUsageShaderRead;
    }
    if (supports & FormatSupport::kStorage) {
        usage |= MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
    }

    MtlTextureInfo info;
    info.fSampleCount = sampleCount;
    info.fMipmapped = mipmapped;
    info.fFormat = TextureFormatToMTLPixelFormat(format);
    info.fUsage = usage;
    info.fStorageMode = storageMode;
    info.fFramebufferOnly = false;

    return TextureInfos::MakeMetal(info);
}

namespace {

skgpu::UniqueKey::Domain get_domain() {
    static const skgpu::UniqueKey::Domain kMtlGraphicsPipelineDomain =
            skgpu::UniqueKey::GenerateDomain();

    return kMtlGraphicsPipelineDomain;
}

}

static constexpr int kMtlGraphicsPipelineKeyData32Count = 4;

UniqueKey MtlCaps::makeGraphicsPipelineKey(const GraphicsPipelineDesc& pipelineDesc,
                                           const RenderPassDesc& renderPassDesc) const {
    UniqueKey pipelineKey;
    {
        // 4 uint32_t's (render step id, paint id, renderpass desc, uint16 write swizzle key)
        UniqueKey::Builder builder(&pipelineKey, get_domain(),
                                   kMtlGraphicsPipelineKeyData32Count, "MtlGraphicsPipeline");
        // add GraphicsPipelineDesc key
        builder[0] = static_cast<uint32_t>(pipelineDesc.renderStepID());
        builder[1] = pipelineDesc.paintParamsID().asUInt();

        // add RenderPassDesc key
        builder[2] = this->getRenderPassDescKey(renderPassDesc);
        builder[3] = renderPassDesc.fWriteSwizzle.asKey();

        builder.finish();
    }

    return pipelineKey;
}

bool MtlCaps::extractGraphicsDescs(const UniqueKey& key,
                                   GraphicsPipelineDesc* pipelineDesc,
                                   RenderPassDesc* renderPassDesc,
                                   const RendererProvider* rendererProvider) const {
    struct UnpackedKeyData {
        // From the GraphicsPipelineDesc
        RenderStep::RenderStepID fRenderStepID = RenderStep::RenderStepID::kInvalid;
        UniquePaintParamsID fPaintParamsID = UniquePaintParamsID::Invalid();

        // From the RenderPassDesc
        TextureFormat fColorFormat = TextureFormat::kUnsupported;
        SampleCount fColorSampleCount = SampleCount::k1;

        TextureFormat fDSFormat = TextureFormat::kUnsupported;
        SampleCount fDSSampleCount = SampleCount::k1;

        Swizzle fWriteSwizzle;
    } keyData;

    SkASSERT(key.domain() == get_domain());
    SkASSERT(key.dataSize() == 4 * kMtlGraphicsPipelineKeyData32Count);

    const uint32_t* rawKeyData = key.data();

    SkASSERT(RenderStep::IsValidRenderStepID(rawKeyData[0]));
    keyData.fRenderStepID = static_cast<RenderStep::RenderStepID>(rawKeyData[0]);
    keyData.fPaintParamsID = rawKeyData[1] ? UniquePaintParamsID(rawKeyData[1])
                                           : UniquePaintParamsID::Invalid();

    keyData.fDSFormat = static_cast<TextureFormat>((rawKeyData[2] >> 8) & 0xFF);
    keyData.fDSSampleCount = static_cast<SampleCount>(rawKeyData[2] & 0xFF);

    keyData.fColorFormat = static_cast<TextureFormat>((rawKeyData[2] >> 24) & 0xFF);
    keyData.fColorSampleCount = static_cast<SampleCount>((rawKeyData[2] >> 16) & 0xFF);

    keyData.fWriteSwizzle = SwizzleCtorAccessor::Make(rawKeyData[3]);

    // Recreate the RenderPassDesc, picking arbitrary load/store ops. Since Metal doesn't need
    // to include resolve attachment details, assume that if color attachment's sample count is > 1
    // that there is a matching resolve attachment (no MSAA-render-to-single-sample support in MTL).
    SkASSERT(keyData.fColorSampleCount == keyData.fDSSampleCount ||
             keyData.fDSFormat == TextureFormat::kUnsupported);
    *renderPassDesc = {};
    renderPassDesc->fColorAttachment = {keyData.fColorFormat,
                                        LoadOp::kClear,
                                        StoreOp::kStore,
                                        keyData.fColorSampleCount};
    renderPassDesc->fDepthStencilAttachment = {keyData.fDSFormat,
                                               LoadOp::kClear,
                                               StoreOp::kDiscard,
                                               keyData.fDSSampleCount};
    if (keyData.fColorSampleCount > SampleCount::k1) {
        renderPassDesc->fColorResolveAttachment = {keyData.fColorFormat,
                                                   LoadOp::kClear,
                                                   StoreOp::kStore,
                                                   SampleCount::k1};
        renderPassDesc->fColorAttachment.fStoreOp = StoreOp::kDiscard;
    }

    renderPassDesc->fSampleCount = keyData.fColorSampleCount;
    renderPassDesc->fWriteSwizzle = keyData.fWriteSwizzle;
    renderPassDesc->fDstReadStrategy = this->getDstReadStrategy();

    // Recreate the GraphicsPipelineDesc
    const RenderStep* renderStep = rendererProvider->lookup(keyData.fRenderStepID);

    UniquePaintParamsID paintID = renderStep->performsShading() ? keyData.fPaintParamsID
                                                                : UniquePaintParamsID::Invalid();

    *pipelineDesc = GraphicsPipelineDesc(renderStep->renderStepID(), paintID);

    return true;
}

uint32_t MtlCaps::getRenderPassDescKey(const RenderPassDesc& renderPassDesc) const {
    static_assert(kTextureFormatCount <= 256);

    // Each attachment format + sample count fits in 16-bits. Load/store ops are ignored.
    auto attachmentKey = [](AttachmentDesc desc) {
        SkASSERT(desc.fFormat != TextureFormat::kUnsupported ||
                 desc.fSampleCount == SampleCount::k1);
        return (static_cast<uint32_t>(desc.fFormat) << 8) |
                static_cast<uint32_t>(desc.fSampleCount);
    };

    // The MtlRenderPassDescriptor requires no information about the resolve attachment
    return (attachmentKey(renderPassDesc.fColorAttachment) << 16) |
            attachmentKey(renderPassDesc.fDepthStencilAttachment);
}

UniqueKey MtlCaps::makeComputePipelineKey(const ComputePipelineDesc& pipelineDesc) const {
    UniqueKey pipelineKey;
    {
        static const skgpu::UniqueKey::Domain kComputePipelineDomain = UniqueKey::GenerateDomain();
        // The key is made up of a single uint32_t corresponding to the compute step ID.
        UniqueKey::Builder builder(&pipelineKey, kComputePipelineDomain, 1, "ComputePipeline");
        builder[0] = pipelineDesc.computeStep()->uniqueID();

        // TODO(b/240615224): The local work group size should factor into the key on platforms
        // that don't support specialization constants and require the workgroup/threadgroup size to
        // be specified in the shader text (D3D12, Vulkan 1.0, and OpenGL).

        builder.finish();
    }
    return pipelineKey;
}

void MtlCaps::buildKeyForTexture(SkISize dimensions,
                                 const TextureInfo& info,
                                 ResourceType type,
                                 GraphiteResourceKey* key) const {
    const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(info);

    SkASSERT(!dimensions.isEmpty());

    // A MTLPixelFormat is an NSUInteger type which is documented to be 32 bits in 32 bit
    // applications and 64 bits in 64 bit applications. So it should fit in an uint64_t, but adding
    // the assert heere to make sure.
    static_assert(sizeof(MTLPixelFormat) <= sizeof(uint64_t));
    SkASSERT(mtlInfo.fFormat != MTLPixelFormatInvalid);
    uint64_t formatKey = static_cast<uint64_t>(mtlInfo.fFormat);

    uint32_t samplesKey = SamplesToKey(info.sampleCount());
    // We don't have to key the number of mip levels because it is inherit in the combination of
    // isMipped and dimensions.
    bool isMipped = mtlInfo.fMipmapped == Mipmapped::kYes;
    Protected isProtected = info.isProtected();
    bool isFBOnly = mtlInfo.fFramebufferOnly;

    // Confirm all the below parts of the key can fit in a single uint32_t. The sum of the shift
    // amounts in the asserts must be less than or equal to 32.
    SkASSERT(samplesKey                         < (1u << kNumSampleKeyBits));
    SkASSERT(static_cast<uint32_t>(isMipped)    < (1u << 1));
    SkASSERT(static_cast<uint32_t>(isProtected) < (1u << 1));
    SkASSERT(mtlInfo.fUsage                     < (1u << 5));
    SkASSERT(mtlInfo.fStorageMode               < (1u << 2));
    SkASSERT(static_cast<uint32_t>(isFBOnly)    < (1u << 1));

    // We need two uint32_ts for dimensions, 2 for format, and 1 for the rest of the key;
    static int kNum32DataCnt = 2 + 2 + 1;

    GraphiteResourceKey::Builder builder(key, type, kNum32DataCnt);

    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = formatKey & 0xFFFFFFFF;
    builder[3] = (formatKey >> 32) & 0xFFFFFFFF;
    builder[4] = (samplesKey                                  << 0) |
                 (static_cast<uint32_t>(isMipped)             << kNumSampleKeyBits) |
                 (static_cast<uint32_t>(isProtected)          << 4) |
                 (static_cast<uint32_t>(mtlInfo.fUsage)       << 5) |
                 (static_cast<uint32_t>(mtlInfo.fStorageMode) << 10)|
                 (static_cast<uint32_t>(isFBOnly)             << 12);

}

} // namespace skgpu::graphite
