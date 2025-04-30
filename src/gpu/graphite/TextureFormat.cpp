/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 #include "src/gpu/graphite/TextureFormat.h"

 #include "include/core/SkColor.h"
 #include "include/core/SkColorType.h"
 #include "include/core/SkYUVAInfo.h"
 #include "include/private/base/SkTArray.h"
 #include "src/core/SkRasterPipelineOpList.h"

namespace skgpu::graphite {

const char* TextureFormatName(TextureFormat format) {
    switch (format) {
        case TextureFormat::kUnsupported:    return "Unsupported";
        case TextureFormat::kR8:             return "R8";
        case TextureFormat::kR16:            return "R16";
        case TextureFormat::kR16F:           return "R16F";
        case TextureFormat::kR32F:           return "R32F";
        case TextureFormat::kA8:             return "A8";
        case TextureFormat::kRG8:            return "RG8";
        case TextureFormat::kRG16:           return "RG16";
        case TextureFormat::kRG16F:          return "RG16F";
        case TextureFormat::kRG32F:          return "RG32F";
        case TextureFormat::kRGB8:           return "RGB8";
        case TextureFormat::kBGR8:           return "BGR8";
        case TextureFormat::kB5_G6_R5:       return "B5_G6_R5";
        case TextureFormat::kR5_G6_B5:       return "R5_G6_B5";
        case TextureFormat::kRGB16:          return "RGB16";
        case TextureFormat::kRGB16F:         return "RGB16F";
        case TextureFormat::kRGB32F:         return "RGB32F";
        case TextureFormat::kRGB8_sRGB:      return "RGB8_sRGB";
        case TextureFormat::kBGR10_XR:       return "BGR10_XR";
        case TextureFormat::kRGBA8:          return "RGBA8";
        case TextureFormat::kRGBA16:         return "RBGA16";
        case TextureFormat::kRGBA16F:        return "RGBA16F";
        case TextureFormat::kRGBA32F:        return "RGBA32F";
        case TextureFormat::kRGB10_A2:       return "RGB10_A2";
        case TextureFormat::kRGBA8_sRGB:     return "RGBA8_sRGB";
        case TextureFormat::kBGRA8:          return "BGRA8";
        case TextureFormat::kBGR10_A2:       return "BGR10_A2";
        case TextureFormat::kBGRA8_sRGB:     return "BGRA8_sRGB";
        case TextureFormat::kABGR4:          return "ABGR4";
        case TextureFormat::kARGB4:          return "ARGB4";
        case TextureFormat::kBGRA10x6_XR:    return "BGRA10x6_XR";
        case TextureFormat::kRGB8_ETC2:      return "RGB8_ETC2";
        case TextureFormat::kRGB8_ETC2_sRGB: return "RGB8_ETC2_sRGB";
        case TextureFormat::kRGB8_BC1:       return "RGB8_BC1";
        case TextureFormat::kRGBA8_BC1:      return "RGBA8_BC1";
        case TextureFormat::kRGBA8_BC1_sRGB: return "RGBA8_BC1_sRGB";
        case TextureFormat::kYUV8_P2_420:    return "YUV8_P2_420";
        case TextureFormat::kYUV8_P3_420:    return "YUV8_P3_420";
        case TextureFormat::kYUV10x6_P2_420: return "YUV10x6_P2_420";
        case TextureFormat::kExternal:       return "External";
        case TextureFormat::kS8:             return "S8";
        case TextureFormat::kD16:            return "D16";
        case TextureFormat::kD32F:           return "D32F";
        case TextureFormat::kD24_S8:         return "D24_S8";
        case TextureFormat::kD32F_S8:        return "D32F_S8";
    }
    SkUNREACHABLE;
}

SkTextureCompressionType TextureFormatCompressionType(TextureFormat format) {
    switch (format) {
        case TextureFormat::kRGB8_ETC2:      [[fallthrough]];
        case TextureFormat::kRGB8_ETC2_sRGB: return SkTextureCompressionType::kETC2_RGB8_UNORM;
        case TextureFormat::kRGB8_BC1:       return SkTextureCompressionType::kBC1_RGB8_UNORM;
        case TextureFormat::kRGBA8_BC1:      [[fallthrough]];
        case TextureFormat::kRGBA8_BC1_sRGB: return SkTextureCompressionType::kBC1_RGBA8_UNORM;
        default:                             return SkTextureCompressionType::kNone;
    }
}

size_t TextureFormatBytesPerBlock(TextureFormat format) {
    switch (format) {
        case TextureFormat::kUnsupported: return 0;
        case TextureFormat::kR8:          return 1;
        case TextureFormat::kR16:         return 2;
        case TextureFormat::kR16F:        return 2;
        case TextureFormat::kR32F:        return 4;
        case TextureFormat::kA8:          return 1;
        case TextureFormat::kRG8:         return 2;
        case TextureFormat::kRG16:        return 4;
        case TextureFormat::kRG16F:       return 4;
        case TextureFormat::kRG32F:       return 8;
        case TextureFormat::kRGB8:        return 3;
        case TextureFormat::kBGR8:        return 3;
        case TextureFormat::kB5_G6_R5:    return 2;
        case TextureFormat::kR5_G6_B5:    return 2;
        case TextureFormat::kRGB16:       return 6;
        case TextureFormat::kRGB16F:      return 6;
        case TextureFormat::kRGB32F:      return 12;
        case TextureFormat::kRGB8_sRGB:   return 3;
        case TextureFormat::kBGR10_XR:    return 4;
        case TextureFormat::kRGBA8:       return 4;
        case TextureFormat::kRGBA16:      return 8;
        case TextureFormat::kRGBA16F:     return 8;
        case TextureFormat::kRGBA32F:     return 16;
        case TextureFormat::kRGB10_A2:    return 4;
        case TextureFormat::kRGBA8_sRGB:  return 4;
        case TextureFormat::kBGRA8:       return 4;
        case TextureFormat::kBGR10_A2:    return 4;
        case TextureFormat::kBGRA8_sRGB:  return 4;
        case TextureFormat::kABGR4:       return 2;
        case TextureFormat::kARGB4:       return 2;
        case TextureFormat::kBGRA10x6_XR: return 8;
        case TextureFormat::kS8:          return 1;
        case TextureFormat::kD16:         return 2;
        case TextureFormat::kD32F:        return 4;
        case TextureFormat::kD24_S8:      return 4;
        case TextureFormat::kD32F_S8:     return 8;
        // NOTE: For compressed formats, the block size refers to an actual compressed block of
        // multiple texels, whereas with other formats the block size represents a single pixel.
        case TextureFormat::kRGB8_ETC2:
        case TextureFormat::kRGB8_ETC2_sRGB:
        case TextureFormat::kRGB8_BC1:
        case TextureFormat::kRGBA8_BC1:
        case TextureFormat::kRGBA8_BC1_sRGB:
            return 8;
        // NOTE: We don't actually know the size of external formats, so this is an arbitrary value.
        // We will see external formats only in wrapped SkImages, so this won't impact Skia's
        // internal budgeting.
        case TextureFormat::kExternal:
            return 4;
        // TODO(b/401016699): We are just over estimating this value to be used in gpu size
        // calculations even though the actually size is probably less. We should instead treat
        // planar formats similar to compressed textures that go through their own special query for
        // calculating size.
        case TextureFormat::kYUV8_P2_420:
        case TextureFormat::kYUV8_P3_420:
            return 3;
        case TextureFormat::kYUV10x6_P2_420:
            return 6;
    }
    SkUNREACHABLE;
}

uint32_t TextureFormatChannelMask(TextureFormat format) {
    switch (format) {
        case TextureFormat::kA8:             return kAlpha_SkColorChannelFlag;

        case TextureFormat::kR8:             [[fallthrough]];
        case TextureFormat::kR16:
        case TextureFormat::kR16F:
        case TextureFormat::kR32F:           return kRed_SkColorChannelFlag;

        case TextureFormat::kRG8:            [[fallthrough]];
        case TextureFormat::kRG16:
        case TextureFormat::kRG16F:
        case TextureFormat::kRG32F:          return kRG_SkColorChannelFlags;

        case TextureFormat::kRGB8:           [[fallthrough]];
        case TextureFormat::kBGR8:
        case TextureFormat::kB5_G6_R5:
        case TextureFormat::kR5_G6_B5:
        case TextureFormat::kRGB16:
        case TextureFormat::kRGB16F:
        case TextureFormat::kRGB32F:
        case TextureFormat::kRGB8_sRGB:
        case TextureFormat::kBGR10_XR:
        case TextureFormat::kRGB8_ETC2:
        case TextureFormat::kRGB8_ETC2_sRGB:
        case TextureFormat::kRGB8_BC1:
        case TextureFormat::kYUV8_P2_420:
        case TextureFormat::kYUV8_P3_420:
        case TextureFormat::kYUV10x6_P2_420: return kRGB_SkColorChannelFlags;

        case TextureFormat::kRGBA8:          [[fallthrough]];
        case TextureFormat::kRGBA16:
        case TextureFormat::kRGBA16F:
        case TextureFormat::kRGBA32F:
        case TextureFormat::kRGB10_A2:
        case TextureFormat::kRGBA8_sRGB:
        case TextureFormat::kBGRA8:
        case TextureFormat::kBGR10_A2:
        case TextureFormat::kBGRA8_sRGB:
        case TextureFormat::kABGR4:
        case TextureFormat::kARGB4:
        case TextureFormat::kBGRA10x6_XR:
        case TextureFormat::kRGBA8_BC1:
        case TextureFormat::kRGBA8_BC1_sRGB:
        case TextureFormat::kExternal:       return kRGBA_SkColorChannelFlags;

        case TextureFormat::kS8:             [[fallthrough]];
        case TextureFormat::kD16:
        case TextureFormat::kD32F:
        case TextureFormat::kD24_S8:
        case TextureFormat::kD32F_S8:
        case TextureFormat::kUnsupported:    return 0;
    }
    SkUNREACHABLE;
}

bool TextureFormatIsDepthOrStencil(TextureFormat format) {
    switch (format) {
        case TextureFormat::kS8:      [[fallthrough]];
        case TextureFormat::kD16:
        case TextureFormat::kD32F:
        case TextureFormat::kD24_S8:
        case TextureFormat::kD32F_S8:
            return true;
        default:
            return false;
    }
}

bool TextureFormatHasDepth(TextureFormat format) {
    switch (format) {
        case TextureFormat::kD16:     [[fallthrough]];
        case TextureFormat::kD32F:
        case TextureFormat::kD24_S8:
        case TextureFormat::kD32F_S8:
            return true;
        default:
            return false;
    }
}

bool TextureFormatHasStencil(TextureFormat format) {
    switch (format) {
        case TextureFormat::kS8:      [[fallthrough]];
        case TextureFormat::kD24_S8:
        case TextureFormat::kD32F_S8:
            return true;
        default:
            return false;
    }
}

bool TextureFormatIsMultiplanar(TextureFormat format) {
    switch (format) {
        case TextureFormat::kYUV8_P2_420:    [[fallthrough]];
        case TextureFormat::kYUV8_P3_420:
        case TextureFormat::kYUV10x6_P2_420:
            return true;
        default:
            return false;
    }
}

// SkColorType Mappings
// ------------------------------------------------------------------------------------------------
namespace {

using RasterPipelineOps = skia_private::STArray<1, SkRasterPipelineOp>;
using MultiPlanarConfig = std::pair<SkYUVAInfo::PlaneConfig, SkYUVAInfo::Subsampling>;

// Configuration related to transferring data to and from the CPU.
// NOTE: For plain CPU data transformed by raster pipeline ops, there is automatic support for
// padding or discarding RP's alpha channel when the TextureFormat only has 3 channels.
struct LoadStoreInfo {
    // When loading data from the CPU into the texture format, it can come from a linear layout
    // of data encoded in the SkColorType, in which case a series of additional SkRasterPipelineOps
    // can adjust it to match the format's GPU encoding. Alternatively, the CPU data can exactly
    // match the format's multiplanar config and data type, or the compressed texture format.
    std::optional<std::variant<RasterPipelineOps,
                               MultiPlanarConfig,
                               SkTextureCompressionType>> fFromCPU;

    // When storing data to the CPU from the texture format, additional SkRasterPipelineOps may
    // be applied to make it compatible with the SkColorType. Currently there is no way to store
    // multiplanar data or compressed data (although specific planes can be stored by using texture
    // views per plane).
    std::optional<RasterPipelineOps> fToCPU;
};

LoadStoreInfo SimpleTransfer(std::initializer_list<SkRasterPipelineOp> loadOps,
                             std::initializer_list<SkRasterPipelineOp> storeOps) {
    return {RasterPipelineOps(loadOps), RasterPipelineOps(storeOps)};
}
LoadStoreInfo SwapRBTransfer() {
    return SimpleTransfer({SkRasterPipelineOp::swap_rb}, {SkRasterPipelineOp::swap_rb});
}
LoadStoreInfo DirectTransfer() {
    return SimpleTransfer({}, {});
}
LoadStoreInfo DirectUpload() {
    return {RasterPipelineOps(), std::nullopt};
}

LoadStoreInfo ThreeChannel() { return DirectTransfer(); } // FIXME explicitly store something here?

LoadStoreInfo MultiplanarUpload(SkYUVAInfo::PlaneConfig planeConfig,
                                SkYUVAInfo::Subsampling subSampling) {
    return {MultiPlanarConfig{planeConfig, subSampling}, std::nullopt};
}
LoadStoreInfo CompressedUpload(SkTextureCompressionType type) {
    return {type, std::nullopt};
}
LoadStoreInfo GPUOnly() { return {std::nullopt, std::nullopt}; }

// struct ReadWriteInfo {
//     Swizzle fReadSwizzle; // All format<->colortype mappings should be sampleable as an SkImage
//     std::optional<Swizzle> fWriteSwizzle; // but they might not be renderable
// };
// ReadWriteInfo RToAlpha() { return {Swizzle("000r"), Swizzle("a000")}; }
// ReadWriteInfo RToGray() { return {Swizzle("rrr1"), Swizzle::RGBA()}; }
// ReadWriteInfo SampleRGBx() { return {Swizzle::RGB1(), std::nullopt}; }
// ReadWriteInfo SampleOnly() { return {Swizzle::RGBA(), std::nullopt}; }

// std::pair<ReadWriteInfo, LoadStoreInfo> Exact() {
//     return {ReadWriteInfo{Swizzle::RGBA(), Swizzle::RGBA()}, DirectTransfer()};
// }

struct OnGPU {
    Swizzle fReadSwizzle; // All format<->colortype mappings should be sampleable via *some* swizzle
    std::optional<Swizzle> fWriteSwizzle; // but they might not be renderable

    static OnGPU NativeSupport()          { return {Swizzle::RGBA(), Swizzle::RGBA()}; }
    static OnGPU MapRToAlpha()            { return {Swizzle("000r"), Swizzle("a000")}; }
    static OnGPU MapRToGray()             { return {Swizzle("rrr1"), Swizzle::RGBA()}; }
    static OnGPU SampleOnly()             { return {Swizzle::RGBA(), std::nullopt}; }
    static OnGPU SampleOnly_ForceOpaque() { return {Swizzle("rgb1"), std::nullopt}; }
};

struct OnCPU {
    // native bothways
    // swap RB
    // upload-only
    // compressed upload only
    // wrapped only

    // + combo with these

    // three channel
    // opaque alpha type?

}

struct ColorFormatInfo {
    SkColorType fColorType;

    OnGPU fReadWriteInfo;
    LoadStoreInfo fLoadStoreInfo;

    // Some TextureFormats are opaque, but the SkColorType does not have an implicit SkAlphaType,
    // so this provides such an override.
    std::optional<SkAlphaType> fAlphaTypeOverride = {};

    ColorFormatInfo(SkColorType colorType)
            : ColorFormatInfo(colorType, OnGPU::NativeSupport()) {}
    ColorFormatInfo(SkColorType colorType, OnGPU onGPU)
            : ColorFormatInfo(colorType, onGPU, DirectTransfer()) {}
    ColorFormatInfo(SkColorType colorType, LoadStoreInfo lsInfo)
            : ColorFormatInfo(colorType, ReadWriteInfo{Swizzle::RGBA(), Swizzle::RGBA()}, lsInfo) {}
    ColorFormatInfo(SkColorType colorType, std::pair<ReadWriteInfo, LoadStoreInfo> infos)
            : ColorFormatInfo(colorType, infos.first, infos.second) {}
    ColorFormatInfo(SkColorType colorType,
                    ReadWriteInfo rwInfo,
                    LoadStoreInfo lsInfo,
                    std::optional<SkAlphaType> alphaOverride = std::nullopt)
            : fColorType(colorType)
            , fReadWriteInfo(rwInfo)
            , fLoadStoreInfo(lsInfo)
            , fAlphaTypeOverride(alphaOverride) {}
    // FIXME should we have a fallback SkColorType or renderable SkColorType that is provided when
    // requesting renderable features?
};

struct ColorFormatInfos {
    ColorFormatInfos(std::initializer_list<ColorFormatInfo> formatInfos) {
        fFormats.reserve(formatInfos.size());
        fFormatInfos.reserve(formatInfos.size());

        for (const ColorFormatInfo& i : formatInfos) {
            fFormatInfos.push_back(i);
            fFormats.push_back(i.fColorType);
        }
    }

    // Parallel arrays, fFormats[i] == fFormatInfos[i].fFormat
    skia_private::STArray<2, SkColorType> fFormats;
    skia_private::STArray<2, ColorFormatInfo> fFormatInfos;
};

const ColorFormatInfos& get_infos_for_format(TextureFormat format) {

#define CASE(F, ...) \
    case F: { \
        static const ColorFormatInfos kInfo{{__VA_ARGS__}}; \
        return kInfo; }

    switch (format) {
        CASE(TextureFormat::kUnsupported, /* no colortypes */)

        CASE(TextureFormat::kR8, kR8_unorm_SkColorType,
                                {kAlpha_8_SkColorType, OnGPU::MapRToAlpha())},
                                {kGray_8_SkColorType, OnGPU(RToGray)})

        CASE(TextureFormat::kR16, {kA16_unorm_SkColorType, RToAlpha()})

        CASE(TextureFormat::kR16F, {kA16_float_SkColorType, RToAlpha()})

        CASE(TextureFormat::kR32F, {}) // alpha 16f?

        CASE(TextureFormat::kA8, kAlpha_8_SkColorType)

        CASE(TextureFormat::kRG8, kR8G8_unorm_SkColorType)

        CASE(TextureFormat::kRG16, kR16G16_unorm_SkColorType)

        CASE(TextureFormat::kRG16F, kR16G16_float_SkColorType)

        CASE(TextureFormat::kRG32F, {}) // kR16G16_float?

        CASE(TextureFormat::kRGB8, {kRGB_888x_SkColorType, ThreeChannel()})

        CASE(TextureFormat::kBGR8, {kRGB_888x_SkColorType, SwapRBTransfer()}) // FIXME 3 channel

        CASE(TextureFormat::kB5_G6_R5, kRGB_565_SkColorType) // NOTE: kRGB_565_SkColorType is misnamed

        CASE(TextureFormat::kR5_G6_B5, {kRGB_565_SkColorType, SwapRBTransfer()})

        CASE(TextureFormat::kRGB16, {kR16G16B16A16_unorm_SkColorType, ThreeChannel(), kOpaque_SkAlphaType})

        CASE(TextureFormat::kRGB16F, {kRGB_F16F16F16x_SkColorType, ThreeChannel()})

        CASE(TextureFormat::kRGB32F, {kRGBA_F32_SkColorType, ThreeChannel(), kOpaque_SkAlphaType})

        CASE(TextureFormat::kRGB8_sRGB, {kSRGBA_8888_SkColorType, ThreeChannel(), kOpaque_SkAlphaType})

        CASE(TextureFormat::kBGR10_XR, kBGR_101010x_XR_SkColorType)

        CASE(TextureFormat::kRGBA8, kRGBA_8888_SkColorType,
                                   {kRGB_888x_SkColorType, SampleRGBx(), DirectUpload()})

        CASE(TextureFormat::kRGBA16, kR16G16B16A16_unorm_SkColorType)

        CASE(TextureFormat::kRGBA16F, kRGBA_F16_SkColorType,
                                      kRGBA_F16Norm_SkColorType, // FIXME not exact match, needs shader clamping right, yes but doesn't appear to make it into any transfer handling
                                     {kRGB_F16F16F16x_SkColorType, SampleRGBx(), DirectUpload()})

        CASE(TextureFormat::kRGBA32F, kRGBA_F32_SkColorType)

        CASE(TextureFormat::kRGB10_A2, kRGBA_1010102_SkColorType,
                                      {kRGB_101010x_SkColorType, SampleRGBx(), DirectUpload()},
                                      {kBGRA_1010102_SkColorType, SwapRBTransfer()})

        CASE(TextureFormat::kRGBA8_sRGB, kSRGBA_8888_SkColorType)

        CASE(TextureFormat::kBGRA8, kBGRA_8888_SkColorType,
                                   {kRGB_888x_SkColorType, SampleRGBx(), DirectUpload()}) // FIXME swap RB too

        CASE(TextureFormat::kBGR10_A2, kBGRA_1010102_SkColorType,
                                      {kBGR_101010x_SkColorType, SampleRGBx(), DirectUpload()},
                                      {kRGBA_1010102_SkColorType, SwapRBTransfer()})

        CASE(TextureFormat::kBGRA8_sRGB, {kSRGBA_8888_SkColorType, SwapRBTransfer()})

        CASE(TextureFormat::kABGR4, kARGB_4444_SkColorType) // NOTE: kARGB_4444_SkColorType is misnamed

        CASE(TextureFormat::kARGB4, {kARGB_4444_SkColorType, SwapRBTransfer()}) // FIXME do we need to do shader-swizzle instead for some historical reason?


        CASE(TextureFormat::kBGRA10x6_XR, kBGRA_10101010_XR_SkColorType)

// FIXME if we just used bitflags, we could combine ThreeChannel | SwapRB | UploadOnly
// and could add a Compressed flag, and not bother with specifying the compression type
        CASE(TextureFormat::kRGB8_ETC2, {kRGB_888x_SkColorType, CompressedUpload(SkTextureCompressionType::kETC2_RGB8_UNORM)})

        CASE(TextureFormat::kRGB8_ETC2_sRGB,
             {kSRGBA_8888_SkColorType, CompressedUpload(SkTextureCompressionType::kETC2_RGB8_UNORM), kOpaque_SkAlphaType})

        CASE(TextureFormat::kRGB8_BC1,
             {kRGB_888x_SkColorType, CompressedUpload(SkTextureCompressionType::kBC1_RGB8_UNORM)})

        CASE(TextureFormat::kRGBA8_BC1,
             {kRGBA_8888_SkColorType, CompressedUpload(SkTextureCompressionType::kBC1_RGBA8_UNORM)})

        CASE(TextureFormat::kRGBA8_BC1_sRGB,
             {kSRGBA_8888_SkColorType, CompressedUpload(SkTextureCompressionType::kBC1_RGBA8_UNORM)})

          // YUV uses kRGBx
        CASE(TextureFormat::kYUV8_P2_420,
        // FIXME could this be kY_VU too? is there any difference?
        // FIXME I think these are GPU only too, the YUVAPixmaps creates multiple single channel plane textures
        // not a multiplanar texture that can be uploaded into
             {kRGB_888x_SkColorType, SampleOnly(), MultiplanarUpload(SkYUVAInfo::PlaneConfig::kY_UV, SkYUVAInfo::Subsampling::k420)})
        CASE(TextureFormat::kYUV8_P3_420,
             {kRGB_888x_SkColorType, SampleOnly(), MultiplanarUpload(SkYUVAInfo::PlaneConfig::kY_U_V, SkYUVAInfo::Subsampling::k420)})

        CASE(TextureFormat::kYUV10x6_P2_420,
             {kRGB_101010x_SkColorType, SampleOnly(), MultiplanarUpload(SkYUVAInfo::PlaneConfig::kY_UV, SkYUVAInfo::Subsampling::k420)},
             {kRGBA_1010102_SkColorType, SampleOnly(), MultiplanarUpload(SkYUVAInfo::PlaneConfig::kY_UV, SkYUVAInfo::Subsampling::k420), kOpaque_SkAlphaType})

        CASE(TextureFormat::kExternal,
            {kRGBA_8888_SkColorType, SampleOnly(), GPUOnly()})

        CASE(TextureFormat::kS8,      /* no colortypes */)
        CASE(TextureFormat::kD16,     /* no colortypes */)
        CASE(TextureFormat::kD32F,    /* no colortypes */)
        CASE(TextureFormat::kD24_S8,  /* no colortypes */)
        CASE(TextureFormat::kD32F_S8, /* no colortypes */)
    }
}

const ColorFormatInfos2& get_infos_for_format(TextureFormat format) {
#define CASE(F, ...) \
    case F: { \
        static const ColorFormatInfos2 kInfo{__VA_ARGS__}; \
        return kInfo; }

    switch (format) {
        CASE(TextureFormat::kUnsupported, /* no colortypes */)

        CASE(TextureFormat::kR8,
             ExactMatch(kR8_unorm_SkColorType),
             ShaderSwizzle(kAlpha_8_SkColorType, /*readSwizzle=*/"000r", /*writeSwizzle=*/"a000"),
             ShaderSwizzle(kGray_8_SkColorType, /*readSwizzle=*/"rrr1", /*writeSwizzle=*/"rgba"))

        CASE(TextureFormat::kR16,
             ShaderSwizzle(kA16_unorm_SkColorType, "000r", "a000"))

        CASE(TextureFormat::kR16F,
             ShaderSwizzle(kA16_float_SkColorType, "000r", "a000"))

        CASE(TextureFormat::kR32F, {}) // alpha 16f?

        CASE(TextureFormat::kA8,
             ExactMatch(kAlpha_8_SkColorType))

        CASE(TextureFormat::kRG8,
             ExactMatch(kR8G8_unorm_SkColorType))

        CASE(TextureFormat::kRG16,
             ExactMatch(kR16G16_unorm_SkColorType))

        CASE(TextureFormat::kRG16F,
             ExactMatch(kR16G16_float_SkColorType))

        CASE(TextureFormat::kRG32F, {}) // kR16G16_float?

        CASE(TextureFormat::kRGB8,
             {kRGBx_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}) // FIXME 3-channel, rgbx?

        CASE(TextureFormat::kBGR8,
             {kRGBx_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}) // FIXME 3-channel, swizzle

        CASE(TextureFormat::kB5_G6_R5,
             ExactMatch(kRGB_565_SkColorType)) // NOTE: kRGB_565_SkColorType is misnamed

        CASE(TextureFormat::kR5_G6_B5,
             AdjustCPU(kRGB_565_SkColorType, SkRasterPipelineOp::swap_rb))

        CASE(TextureFormat::kRGB16,
             {kR16G16B16A16_unorm_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}) // FIXME 3 channel

        CASE(TextureFormat::kRGB16F,
             {kRGBx_F16_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}) // FIXME 3 channel

        CASE(TextureFormat::kRGB32F,
             {kRGBx_F32_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}) // FIXME 3 channel

        CASE(TextureFormat::kRGB8_sRGB,
             {kSRGBx_8888_SkColorType, Swizzle::RGBA(), Swizzle::RGBA()}) // FIXME 3 channel

        CASE(TextureFormat::kBGR10_XR,
             ExactMatch(kBGR_101010x_XR_SkColorType)) // FIXME: need a=1 read swizzle?

        CASE(TextureFormat::kRGBA8,
             ExactMatch(kRGBA_8888_SkColorType),
             ShaderSwizzle(kRGB_888x_SkColorType, "rgb1", "rgba")) // FIXME store 1 too?, not renderable?

        CASE(TextureFormat::kRGBA16,
             ExactMatch(kR16G16B16A16_unorm_SkColorType))

        CASE(TextureFormat::kRGBA16F,
             ExactMatch(kRGBA_F16_SkColorType),
             ExactMatch(kRGBA_F16Norm_SkColorType), // FIXME not exact match, needs shader clamping right?
             ShaderSwizzle(kRGB_F16F16F16x_SkColorType, "rgb1", "rgba")) // FIXME store 1? not renderable?

        CASE(TextureFormat::kRGBA32F,
             ExactMatch(kRGBA_F32_SkColorType))

        CASE(TextureFormat::kRGB10_A2,
             ExactMatch(kRGBA_1010102_SkColorType),
             ShaderSwizzle(kRGB_101010x_SkColorType, "rgb1", "rgba") // FIXME store 1? not renderable?
             // FIXME support kBGRA_1010102 with a swap_rb?
             )
        CASE(TextureFormat::kRGBA8_sRGB,
             ExactMatch(kSRGBA_8888_SkColorType))

        CASE(TextureFormat::kBGRA8,
             ExactMatch(kBGRA_8888_SkColorType),
             {kRGB_888x_SkColorType, "rgb1", "rgba", SkRasterPipelineOp::swap_rb, SkRasterPipelineOp::swap_rb}, // renderable? store 1?

             // FIXME handle RGBA_8888 with swap_rb?
            )

        CASE(TextureFormat::kBGR10_A2,
             ExactMatch(kBGRA_1010102_SkColorType),
             AdjustCPU(kBGR_101010x_SkColorType, SkRasterPipelineOp::swap_rb)
             // FIXME RGBA_1010102 with swap_rb? Chrome seems to want this for the color type for some reason...
             )
        CASE(TextureFormat::kBGRA8_sRGB,
             AdjustCPU(kSRGBA_8888_SkColorType, SkRasterPipelineOp::swap_rb))

        CASE(TextureFormat::kABGR4,
             ExactMatch(kARGB_4444_SkColorType)) // NOTE: kARGB_4444_SkColorType is misnamed

        CASE(TextureFormat::kARGB4,
             // FIXME do we need to do shader-swizzle instead for some historical reason?
             AdjustCPU(kARGB_4444_SkColorType, SkRasterPipelineOp::swap_rb))

        CASE(TextureFormat::kBGRA10x6_XR,
             ExactMatch(kBGRA_10101010_XR_SkColorType))

        CASE(TextureFormat::kRGB8_ETC2,
             GPUOnly(kRGB_888x_SkColorType)) // FIXME: GPUOnly is slight misnomer, data can be stored compressed on the CPU, but it'll be not an SkColorType
        CASE(TextureFormat::kRGB8_ETC2_sRGB,
             GPUOnly(kSRGBA_8888_SkColorType)) // FIXME: Should we also allow for a kAlphaType override?

        CASE(TextureFormat::kRGB8_BC1,
             GPUOnly(kRGB_888x_SkColorType))

        CASE(TextureFormat::kRGBA8_BC1,
             GPUOnly(kRGBA_8888_SkColorType))

        CASE(TextureFormat::kRGBA8_BC1_sRGB,
             GPUOnly(kSRGBA_8888_SkColorType))

          // YUV uses kRGBx
        CASE(TextureFormat::kYUV8_P2_420,
             GPUOnly(kRGB_888x_SkColorType))
        CASE(TextureFormat::kYUV8_P3_420,
             GPUOnly(kRGB_888x_SkColorType))

        CASE(TextureFormat::kYUV10x6_P2_420,
             GPUOnly(kRGBA_1010102_SkColorType), // opaque override?, present for SkYUVPixelFormat
             GPUOnly(kRGB_101010x_SkColorType))

        CASE(TextureFormat::kExternal,
             GPUOnly(kRGBA_8888_SkColorType))

        CASE(TextureFormat::kS8,      /* no colortypes */)
        CASE(TextureFormat::kD16,     /* no colortypes */)
        CASE(TextureFormat::kD32F,    /* no colortypes */)
        CASE(TextureFormat::kD24_S8,  /* no colortypes */)
        CASE(TextureFormat::kD32F_S8, /* no colortypes */)
    }
}

const ColorFormatInfos& get_infos_for_colortype(SkColorType ct) {

}

} // anonymous namespace

} // namespace skgpu::graphite
