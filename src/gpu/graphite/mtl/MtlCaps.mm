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

    this->initFormatTable(device);

    // Metal-specific MtlCaps

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
    fResourceBindingReqs.fRenderStepBufferBinding =
            MtlGraphicsPipeline::kRenderStepUniformBufferIndex;
    fResourceBindingReqs.fPaintParamsBufferBinding = MtlGraphicsPipeline::kPaintUniformBufferIndex;
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
}

void MtlCaps::initShaderCaps() {
    SkSL::ShaderCaps* shaderCaps = fShaderCaps.get();

    // Dual source blending requires Metal 1.2.
    if (@available(macOS 10.12, iOS 10.0, tvOS 10.0, *)) {
        shaderCaps->fDualSourceBlendingSupport = true;
    }

    // Setting this true with the assumption that this cap will eventually mean we support varying
    // precisions and not just via modifiers.
    shaderCaps->fUsesPrecisionModifiers = true;
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

// Define this so we can use it to initialize arrays and work around
// the fact that these pixel formats are not always available.
#define kMTLPixelFormatB5G6R5Unorm MTLPixelFormat(40)
#define kMTLPixelFormatABGR4Unorm MTLPixelFormat(42)
#define kMTLPixelFormatETC2_RGB8 MTLPixelFormat(180)

// These are all the valid MTLPixelFormats that we currently support in Skia.  They are roughly
// ordered from most frequently used to least to improve look up times in arrays.
static constexpr MTLPixelFormat kMtlFormats[] = {
    MTLPixelFormatRGBA8Unorm,
    MTLPixelFormatR8Unorm,
    MTLPixelFormatA8Unorm,
    MTLPixelFormatBGRA8Unorm,
    kMTLPixelFormatB5G6R5Unorm,
    MTLPixelFormatRGBA16Float,
    MTLPixelFormatR16Float,
    MTLPixelFormatRG8Unorm,
    MTLPixelFormatRGB10A2Unorm,
    // MTLPixelFormatBGR10A2Unorm
    kMTLPixelFormatABGR4Unorm,
    MTLPixelFormatRGBA8Unorm_sRGB,
    MTLPixelFormatR16Unorm,
    MTLPixelFormatRG16Unorm,
    kMTLPixelFormatETC2_RGB8,
#ifdef SK_BUILD_FOR_MAC
    MTLPixelFormatBC1_RGBA,
#endif
    MTLPixelFormatRGBA16Unorm,
    MTLPixelFormatRG16Float,

    MTLPixelFormatStencil8,
    MTLPixelFormatDepth16Unorm,
    MTLPixelFormatDepth32Float,
#ifdef SK_BUILD_FOR_MAC
    MTLPixelFormatDepth24Unorm_Stencil8,
#endif
    MTLPixelFormatDepth32Float_Stencil8,

    MTLPixelFormatInvalid,
};

void MtlCaps::initFormatTable(const id<MTLDevice> device) {
    // Init sample counts. All devices support being single-sampled.
    static constexpr SkEnumBitMask<TextureFormatInfo::SampleCount> kSingleSampled
    SkEnumBitMask<TextureFormatInfo::SampleCount> colorSampleCounts = TextureFormatInfo::SampleCount::k1;
    if (!this->isIntel()) {
        for (auto sampleCnt : {2, 4, 8}) {
            if ([device supportsTextureSampleCount:sampleCnt]) {
                colorSampleCounts |= (TextureFormatInfo::SampleCount) sampleCnt;
            }
        }
    }

    auto initFormats = [this](SkEnumBitMask<TextureFormatInfo::Supports> supports,
                              SkEnumBitMask<TextureFormatInfo::SampleCount> sampleCounts,
                              SkSpan<MTLPixelFormat> formats) {
        // Currently we aren't using any linear textures in the Metal backend
        static constexpr std::pair<SkEnumBitMask<TextureFormatInfo::Supports>,
                                   SkEnumBitMask<TextureFormatInfo::SampleCount>> kDisabled =
                {TextureFormatInfo::Supports::kNone, TextureFormatInfo::SampleCount::kNone};
        for (MTLPixelFormat format : formats) {
            TextureFormat tf = MTLPixelFormatToTextureFormat(format);
            this->initFormat(tf, /*optimal=*/{supports, sampleCounts}, /*linear=*/kDisabled);
        }
    };

    // The Metal feature set tables specify capabilities with the following options:
    //  - Atomic: The GPU can use atomic operations on textures with the pixel format.
    //  - All: The GPU has the following texture capabilities for the pixel format:
    //      - Filter: The GPU can filter a texture with the pixel format during sampling.
    //      - Write: The GPU can write to a texture on a per-pixel basis with the pixel format.
    //      - Color: The GPU can use a texture with the pixel format as a color render target.
    //      - Blend: The GPU can blend a texture with the pixel format.
    //      - MSAA: The GPU can use a texture with the pixel format as a destination for multisample
    //        antialias (MSAA) data.
    //      - Sparse: The GPU supports sparse-texture allocations for textures with the pixel
    //        format.
    //      - Resolve: The GPU can use a texture with the pixel format as a source for multisample
    //        antialias (MSAA) resolve operations
    //
    // These map on to the Supports enum as follows:
    //  - kRender requires MSAA and Resolve for sample count > 1, and requires Color + Blend if the
    //    format is a color texture format.
    //  - kSample requires Filter.
    //  - kStorage requires Write.
    //  - All available texture formats support kCopyDst, kCopySrc.
    //
    // Currently there is no Sparse or Atomic support in Graphite, so those features aren't checked.
    // Graphite does not need to support any device in the Apple2 family, as this corresponds to the
    // A8 GPU on the iPhone 6. This device is limited to iOS 12, so does not meet the iOS 13 minimum
    // that Graphite requires. Apple3 hardware is supported through iOS 15, so we do check those.
    static constexpr auto kSupportsAll = TextureFormatInfo::Supports::kRender |
                                         TextureFormatInfo::Supports::kSample |
                                         TextureFormatInfo::Supports::kCopyDst |
                                         TextureFormatInfo::Supports::kCopySrc |
                                         TextureFormatInfo::Supports::kStorage;

    static constexpr auto kTextureOnly = TextureFormatInfo::Supports::kSample |
                                         TextureFormatInfo::Supports::kCopyDst |
                                         TextureFormatInfo::Supports::kCopySrc;

    static constexpr auto kRenderOnly  = TextureFormatInfo::Supports::kRender |
                                         TextureFormatInfo::Supports::kCopyDst |
                                         TextureFormatInfo::Supports::kCopySrc;

    initFormats(kSupportsAll, colorSampleCounts, {MTLPixelFormatA8Unorm, // NOTE: limited on Apple2
                                                  MTLPixelFormatR8Unorm,
                                                  MTLPixelFormatR16Float,
                                                  MTLPixelFormatRG8Unorm,
                                                  MTLPixelFormatRG16Float,
                                                  MTLPixelFormatRGBA8Unorm,
                                                  MTLPixelFormatRGB10A2Unorm,
                                                  MTLPixelFormatBGRA8Unorm,
                                                  MTLPixelFormatBGR10A2Unorm,
                                                  MTLPixelFormatRGBA16Float});

    // 16-bit unorm formats do not have the Resolve feature on apple3 hardware
    auto unorm16SampleCounts = (this->isMac() || fFamilyGroup > 3)
            ? colorSampleCounts  : TextureFormatInfo::SampleCount::k1;
    initFormats(kSupportsAll, unorm16SampleCounts, {MTLPixelFormatR16Unorm,
                                                    MTLPixelFormatRG16Unorm,
                                                    MTLPixelFormatRGBA16Unorm});

    // sRGB formats can't be written to on Mac
    auto srgbSupport = kSupportsAll & ~(this->isMac() ? TextureFormatInfo::Supports::kStorage
                                                      : TextureFormatInfo::Supports::kNone);
    initFormats(srgbSupport, colorSampleCounts, {MTLPixelFormatRGBA8Unorm_sRGB,
                                                 MTLPixelFormatBGRA8Unorm_sRGB});

    // 32-bit float formats can't be sampled or MSAA until Apple9 or Mac
    auto float32Support = (this->isMac() || fFamilyGroup >= 9) ? kSupportsAll : kRenderOnly;
    auto float32Samples = (this->isMac() || fFamilyGroup >= 9) ? colorSampleCounts : TextureFormatInfo::SampleCount::k1;
    initFormats(float32Support, float32Samples, {MTLPixelFormatR32Float,
                                                 MTLPixelFormatRG32Float,
                                                 MTLPixelFormatRGBA32Float});

    if (this->isApple()) {
        // Apply-only formats
        if (@available(macOS 11.0, iOS 8.0, tvOS 9.0, *)) {
            initFormats(kSupportsAll, colorSampleCounts, {MTLPixelFormatB5G6R5Unorm,
                                                          MTLPixelFormatABGR4Unorm,
                                                          MTLPixelFormatBGR10_XR,
                                                          MTLPixelFormatBGRA10_XR});
            initFormats(kTextureOnly, TextureFormatInfo::SampleCount::k1, {
                                                          MTLPixelFormatETC2_RGB8,
                                                          MTLPixelFormatETC2_RGB8_sRGB});
        }
    } else {
        // NOTE: Some Apple7,8 devices and all Apple9 devices support BC1 as well, but for now we
        // treat BC1 as Mac-only.
        initFormats(kTextureOnly, TextureFormatInfo::SampleCount::k1, {MTLPixelFormatBC1_RGBA,
                                                                       MTLPixelFormatBC1_RGBA_sRGB});

    }

    // Depth-stencil formats (some levels of hardware support sampling depth formats, but since
    // Graphite currently doesn't utilize that functionality, we group them all as render-only).
    initFormats(kRenderOnly, colorSampleCounts, {MTLPixelFormatDepth16Unorm, // NOTE: limited on Apple2
                                                 MTLPixelFormatDepth32Float,
                                                 MTLPixelFormatDepth32Float_Stencil8});
    if (this->isApple()) {
        initFormats(kRenderOnly, colorSampleCounts, {MTLPixelFormatStencil8});
    }
#ifdef SK_BUILD_FOR_MAC
        if (this->isMac() && [device isDepth24Stencil8PixelFormatSupported]) {
            initFormats(kRenderOnly, colorSampleCounts, {MTLPixelFormatDepth24Unorm_Stencil8});
        }
#endif
}

TextureInfo MtlCaps::getDefaultAttachmentTextureInfo(AttachmentDesc desc,
                                                     Protected,
                                                     Discardable discardable) const {
    if (!this->isSampleCountSupported(desc.fFormat, desc.fSampleCount)) {
        return {};
    }

    // Default to private in the event it's not discardable or memoryless is not available
    MTLStorageMode storageMode = MTLStorageModePrivate;

    // Try to use memoryless if it's available (only on new Apple silicon)
    if (discardable == Discardable::kYes && this->isApple()) {
        if (@available(macOS 11.0, iOS 10.0, tvOS 10.0, *)) {
            storageMode = MTLStorageModeMemoryless;
        }
    }

    MtlTextureInfo info;
    info.fSampleCount = desc.fSampleCount;
    info.fMipmapped = Mipmapped::kNo;
    info.fFormat = TextureFormatToMTLPixelFormat(desc.fFormat);
    info.fUsage = MTLTextureUsageRenderTarget;
    info.fStorageMode = storageMode;
    info.fFramebufferOnly = false;

    return TextureInfos::MakeMetal(info);
}

TextureInfo MtlCaps::getDefaultSampledTextureInfo(SkColorType colorType,
                                                  Mipmapped mipmapped,
                                                  Protected,
                                                  Renderable renderable) const {
    MTLTextureUsage usage = MTLTextureUsageShaderRead;
    if (renderable == Renderable::kYes) {
        usage |= MTLTextureUsageRenderTarget;
    }

    MTLPixelFormat format = this->getFormatFromColorType(colorType);
    if (format == MTLPixelFormatInvalid) {
        return {};
    }

    MtlTextureInfo info;
    info.fSampleCount = 1;
    info.fMipmapped = mipmapped;
    info.fFormat = format;
    info.fUsage = usage;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return TextureInfos::MakeMetal(info);
}

TextureInfo MtlCaps::getTextureInfoForSampledCopy(const TextureInfo& textureInfo,
                                                  Mipmapped mipmapped) const {
    MtlTextureInfo info;
    info.fSampleCount = 1;
    info.fMipmapped = mipmapped;
    info.fFormat = TextureInfoPriv::Get<MtlTextureInfo>(textureInfo).fFormat;
    info.fUsage = MTLTextureUsageShaderRead;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return TextureInfos::MakeMetal(info);
}

namespace {

skgpu::UniqueKey::Domain get_domain() {
    static const skgpu::UniqueKey::Domain kMtlGraphicsPipelineDomain =
            skgpu::UniqueKey::GenerateDomain();

    return kMtlGraphicsPipelineDomain;
}

MTLPixelFormat format_from_compression(SkTextureCompressionType compression) {
    switch (compression) {
        case SkTextureCompressionType::kETC2_RGB8_UNORM:
            return kMTLPixelFormatETC2_RGB8;
        case SkTextureCompressionType::kBC1_RGBA8_UNORM:
#ifdef SK_BUILD_FOR_MAC
            return MTLPixelFormatBC1_RGBA;
#endif
        default:
            return MTLPixelFormatInvalid;
    }
}
}

TextureInfo MtlCaps::getDefaultCompressedTextureInfo(SkTextureCompressionType compression,
                                                     Mipmapped mipmapped,
                                                     Protected) const {
    MTLTextureUsage usage = MTLTextureUsageShaderRead;

    MTLPixelFormat format = format_from_compression(compression);
    if (format == MTLPixelFormatInvalid) {
        return {};
    }

    MtlTextureInfo info;
    info.fSampleCount = 1;
    info.fMipmapped = mipmapped;
    info.fFormat = format;
    info.fUsage = usage;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return TextureInfos::MakeMetal(info);
}

TextureInfo MtlCaps::getDefaultStorageTextureInfo(SkColorType colorType) const {
    // Storage textures are currently always sampleable from a shader.
    MTLPixelFormat format = static_cast<MTLPixelFormat>(this->getFormatFromColorType(colorType));
    if (format == MTLPixelFormatInvalid) {
        return {};
    }

    const FormatInfo& formatInfo = this->getFormatInfo(format);
    if (!SkToBool(FormatInfo::kStorage_Flag & formatInfo.fFlags)) {
        return {};
    }

    MtlTextureInfo info;
    info.fSampleCount = 1;
    info.fMipmapped = Mipmapped::kNo;
    info.fFormat = format;
    info.fUsage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return TextureInfos::MakeMetal(info);
}

const Caps::ColorTypeInfo* MtlCaps::getColorTypeInfo(
        SkColorType ct, const TextureInfo& textureInfo) const {
    MTLPixelFormat mtlFormat = TextureInfoPriv::Get<MtlTextureInfo>(textureInfo).fFormat;
    if (mtlFormat == MTLPixelFormatInvalid) {
        return nullptr;
    }

    const FormatInfo& info = this->getFormatInfo(mtlFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const ColorTypeInfo& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == ct) {
            return &ctInfo;
        }
    }

    return nullptr;
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
        uint8_t fColorSampleCount = 1;

        TextureFormat fDSFormat = TextureFormat::kUnsupported;
        uint8_t fDSSampleCount = 1;

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
    keyData.fDSSampleCount = static_cast<uint8_t>(rawKeyData[2] & 0xFF);

    keyData.fColorFormat = static_cast<TextureFormat>((rawKeyData[2] >> 24) & 0xFF);
    keyData.fColorSampleCount = static_cast<uint8_t>((rawKeyData[2] >> 16) & 0xFF);

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
    if (keyData.fColorSampleCount > 1) {
        renderPassDesc->fColorResolveAttachment = {keyData.fColorFormat,
                                                   LoadOp::kClear,
                                                   StoreOp::kStore,
                                                   /*fSampleCount=*/1};
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
        SkASSERT(desc.fFormat != TextureFormat::kUnsupported || desc.fSampleCount == 1);
        return (static_cast<uint32_t>(desc.fFormat) << 8) | desc.fSampleCount;
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

bool MtlCaps::onIsTexturable(const TextureInfo& info) const {
    if (!info.isValid()) {
        return false;
    }
    const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(info);
    if (!(mtlInfo.fUsage & MTLTextureUsageShaderRead)) {
        return false;
    }
    if (mtlInfo.fFramebufferOnly) {
        return false;
    }
    return this->isTexturable(mtlInfo.fFormat);
}

bool MtlCaps::isTexturable(MTLPixelFormat format) const {
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    return SkToBool(FormatInfo::kTexturable_Flag & formatInfo.fFlags);
}

bool MtlCaps::isRenderable(const TextureInfo& info) const {
    if (!info.isValid()) {
        return false;
    }
    TextureFormat format = TextureInfoPriv::ViewFormat(info);
    const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(info);
    return (mtlInfo.fUsage & MTLTextureUsageRenderTarget) &&
           this->isSampleCountSupported(format, info.numSamples());
}

bool MtlCaps::isStorage(const TextureInfo& info) const {
    if (!info.isValid()) {
        return false;
    }
    const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(info);
    if (!(mtlInfo.fUsage & MTLTextureUsageShaderWrite)) {
        return false;
    }
    if (mtlInfo.fFramebufferOnly) {
        return false;
    }
    const FormatInfo& formatInfo = this->getFormatInfo(mtlInfo.fFormat);
    return mtlInfo.fSampleCount == 1 && SkToBool(FormatInfo::kStorage_Flag & formatInfo.fFlags);
}

bool MtlCaps::supportsWritePixels(const TextureInfo& texInfo) const {
    if (!texInfo.isValid()) {
        return false;
    }
    const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(texInfo);
    if (mtlInfo.fFramebufferOnly) {
        return false;
    }

    if (mtlInfo.fSampleCount > 1) {
        return false;
    }

    return true;
}

bool MtlCaps::supportsReadPixels(const TextureInfo& texInfo) const {
    if (!texInfo.isValid()) {
        return false;
    }
    const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(texInfo);
    if (mtlInfo.fFramebufferOnly) {
        return false;
    }

    // We disallow reading back directly from compressed textures.
    if (MtlFormatIsCompressed(mtlInfo.fFormat)) {
        return false;
    }

    if (mtlInfo.fSampleCount > 1) {
        return false;
    }

    return true;
}

std::pair<SkColorType, bool /*isRGBFormat*/> MtlCaps::supportedWritePixelsColorType(
        SkColorType dstColorType,
        const TextureInfo& dstTextureInfo,
        SkColorType srcColorType) const {
    if (!dstTextureInfo.isValid()) {
        return {kUnknown_SkColorType, false};
    }
    const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(dstTextureInfo);

    const FormatInfo& info = this->getFormatInfo(mtlInfo.fFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == dstColorType) {
            return {ctInfo.fTransferColorType, false};
        }
    }
    return {kUnknown_SkColorType, false};
}

std::pair<SkColorType, bool /*isRGBFormat*/> MtlCaps::supportedReadPixelsColorType(
        SkColorType srcColorType,
        const TextureInfo& srcTextureInfo,
        SkColorType dstColorType) const {
    if (!srcTextureInfo.isValid()) {
        return {kUnknown_SkColorType, false};
    }
    const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(srcTextureInfo);

    // TODO: handle compressed formats
    if (MtlFormatIsCompressed(mtlInfo.fFormat)) {
        SkASSERT(this->isTexturable(mtlInfo.fFormat));
        return {kUnknown_SkColorType, false};
    }

    const FormatInfo& info = this->getFormatInfo(mtlInfo.fFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == srcColorType) {
            return {ctInfo.fTransferColorType, false};
        }
    }
    return {kUnknown_SkColorType, false};
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

    uint32_t samplesKey = SamplesToKey(mtlInfo.fSampleCount);
    // We don't have to key the number of mip levels because it is inherit in the combination of
    // isMipped and dimensions.
    bool isMipped = mtlInfo.fMipmapped == Mipmapped::kYes;
    Protected isProtected = info.isProtected();
    bool isFBOnly = mtlInfo.fFramebufferOnly;

    // Confirm all the below parts of the key can fit in a single uint32_t. The sum of the shift
    // amounts in the asserts must be less than or equal to 32.
    SkASSERT(samplesKey                         < (1u << 3));
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
                 (static_cast<uint32_t>(isMipped)             << 3) |
                 (static_cast<uint32_t>(isProtected)          << 4) |
                 (static_cast<uint32_t>(mtlInfo.fUsage)       << 5) |
                 (static_cast<uint32_t>(mtlInfo.fStorageMode) << 10)|
                 (static_cast<uint32_t>(isFBOnly)             << 12);

}

} // namespace skgpu::graphite
