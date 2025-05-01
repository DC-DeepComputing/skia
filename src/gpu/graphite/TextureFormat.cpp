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

struct OnGPU {
    Swizzle fReadSwizzle; // All format<->colortype mappings should be sampleable via *some* swizzle
    std::optional<Swizzle> fWriteSwizzle; // but they might not be renderable

    static OnGPU NativeSupport()   { return {Swizzle::RGBA(), Swizzle::RGBA()}; }
    static OnGPU MapRToAlpha()     { return {Swizzle("000r"), Swizzle("a000")}; }
    static OnGPU MapRToGray()      { return {Swizzle("rrr1"), Swizzle::RGBA()}; }
    static OnGPU SampleOnly()      { return {Swizzle::RGBA(), std::nullopt}; }
    static OnGPU SampleOnly_RGB1() { return {Swizzle("rgb1"), std::nullopt}; }
};

using XferConversionFn = void (*)(int width, int height,
                                  const void* src, size_t srcRowBytes,
                                  void* dst, size_t dstRowBytes);

// For uploading, we want identity to actually do a SkRectMemcpy directly to the target buffer
// For uploading, we want things that make changes to be able to write directly to the GPU buffer
// For readback, we want identity to be able to return the original buffer
// but if readback has conversion, then we want it to be able to convert from the original buffer
// into a new allocation.
//
// But we also want to be able to compose readback (and maybe upload?) with additional conversion
// operations? Yes, ugh, but there is currently no restriciton on the dst image info from a
// readback, and there's no restriction on the src image info for a writePixels.
// -> for upload during creating a texture, we reject if they aren't compatible, but for writes
//    we don't discard the write.
// -> could we add logic that basically says the src color format isn't compatible so do
//    a texture upload and then draw the image?
// -> sure, but that is also extra overhead than just creating a temporary source that has the
//    conversions applied,but if we do that, we don't wnat to create a *second* temporary source
//    just because there was further incompatibilities.

// Could we do this by having additional raster pipeline ops that we define?
// Does it even make sense to have a SwapRB or RGBx that is shared between upload and readback
//   in this situation when we most likely have to apply the operations in oppposite order.

// Maybe the types that get registered in the TextureFormat table don't have to be exactly what
// gets returned to the user, but produces something that can be composed with whatever conversion
// steps are required by the callers context.

// CPU -> GPU write operations
// Identity: compressed or not, can do SkRectMemcpy from source data to the mapped GPU buffer
// Three-channel-only: not-compressed, must do per-pixel transfer from source data to the mapped
//    GPU buffer, only copying the 3 channels from the src but skipping full pixel
// Swap RB: either load_op, swap_rb, store_op with SkRasterPipeline (SkConvertPixels?) that writes
//    directly into the GPU mapped buffer,
//    OR do a per-pixel transfer doing manual swap based on bit depths (harder for 565) and avoid
//    processing with SkRasterPipeline
// Three-channel+swap RB: Do swap RB with skrp into a temporary buffer, and then do per-pixel
//    three channel drop into mappged GPU buffer.
//    OR do a per-pixel transfer that does a swap and drop together into mapped GPU buffer.
//
// Now if we also combine these with some src CT to TF compatible CT, how does that change things?
// Identity: SkConvertPixels src->TF ct into mapped GPU buffer, does not work for compressed data
//   but that's fine because uploading to an SkImage requires the CT be compatible, and there isn't
//   compressed SkSurfaces to expose a writePixels API.
// Three-channel-only: SkConvertPixels into a temporary buffer, per-pixel transfer from temp data
//   nto GPU mapped buffer.
//   OR SkConvertPixels 1-by-1 plus per-pixel alpha drop into GPU buffer
// Swap RB: insert swap_rb in front of the store op but otherwise integrate into SkRP for the
//   src -> TF CT conversion, SkConvertPixels into mapped GPU buffer.
//   OR do a SkConvertPixels into temp buffer and then per-pixel manual swap.
//   OR invoke SkRP per pixel (wh==1) before the per-pixel manual swap.
// Three-channel+swap RB: SkRP conversion from src->TF w/ inserted swap_rb instruction into a
//   temp buffer, and then do the per-pixel transfer from temp into GPU mapped buffer
//   OR invoke SkRP per pixle, then do manual swap per pixel and alpha drop per pixel directly
//   into GPU mapped buffer.

//   If any of these paths do need to create a temporary buffer, it is entirely internal to the
//   caller because the data ends up in the dst GPU buffer that they are responsible for. And it
//   could mean that we flexibly have it be a row of data to convert instead of a whole image?

// GPU -> CPU read operations
// Identity: return the mapped GPU buffer to the client
// Three-channel-only: create temp buffer, expand alpha channel from GPU mapped buffer to temp buffer
//    return temp buffer to the client. Per-pixel copy operation
// Swap RB: create temp buffer, SkConvertPixels from GPU buffer to tmp, return temp
//    OR create temp, per-pixel manual swap, return temp
// Three-channel+swap RB: create tmp buffer, copy GPU data + alpha expansion into tmp buffer,
//      then create 2nd tmp buffer w/ SkConvertPixels then return 2nd temp
//    OR create tmp buffer, copy GPU data per pixel with alpha expansion and manual swap
//
// And if there are additional operations, then we also need a tmp buffer. swapRB and identity could
// chain into that, and the others could do a 2nd tmp buffer, or we do per-pixel rsater pipeline
// conversion?

// How much slower is it to run 1x1 or run widthx1 to minimize the amount of extra memory needed
// in a temporary buffer?

// At least for GPU -> CPU, we can basically say make a full dst buffer if !identity. Any extra
// intermediate copy could be limited to a scanline, or be managed internally (and then no different
// than the current three-channel + extra conversion steps case).

enum class XferOps : uint8_t {
    kIdentity  = 0b000, // No conversion needed
    kDropAlpha = 0b001, // Drop the alpha channel of the SkColorType to store just 3 channels
    kPadAlpha  = 0b010, // Padd each pixel's data with an opaque alpha value to restore 4 channels
    kSwapRB    = 0b100, // Swap red/blue channels within each pixel

    kDisabled  = ?? // or wrap in optional?
};
SK_MAKE_BITMASK_OPS(XferOps)


std::pair<XferConversionFn, bool/*isIdentity*/> get_conversion_fn(
        SkEnumBitMask<XferOps> preSrcType,
        SkColorType srcType,
        SkColorType dstType,
        SkEnumBitMask<XferOps> postDstType) {
    SkASSERT(srcType != kUnknown_SkColorType && dstType != kUnknown_SkColorType);

    // For a CPU->GPU transfer, `preSrcType` ops should be the identity, since the source CPU data
    // should be exactly described by `srcType`. For this direction, `postDstType` adds extra ops
    // to finish converting from a format-compatible `dstType` to the actual TextureFormat.
    //
    // Conversely, for a GPU->CPU transfer, the `postDstType` ops should be the identity because the
    // destination CPU data should be exactly described by `dstType`. In this case, `preSrcType`
    // adds extra ops to first convert from the TextureFormat to the format-compatible `srcType`.
    SkASSERT(preSrcType == XferOps::kIdentity || postDstType == XferOps::kIdentity);
    SkEnumBitMask<XferOps> xferOps = preSrcType | postDstType;

    // A TextureFormat <-> SkColorType mapping shouldn't apply both of these for a single transfer.
    SkASSERT(!((xferOps & XferOps::kDropAlpha) && (xferOps & XferOps::kPadAlpha)));

    if (srcType != dstType) {
        // Some color type combinations are either no-ops when there is no intermediate color space
        // conversion, or are equivalent to an XferOp (SkRasterPipelineOp::swap_rb) that might
        // cancel each other out.

        // Now in this sort of logic, nothing is going to be "canonical" by padding or dropping alpha
        // And the swapRB could be inferred by looking at channel order of the color type, even if
        // the bit depths are all different.  In that world, if there was a channel swap and we
        // had a XferOp swap, we'd skip the xfer op since the load/store ops are still needed.
        //
        // But if there was channel swap but load/store ops were no-ops, we'd go the other way and
        // push it to a xferOp swap -> colortype identity and either xfer op identity or add swap
        // depending on whether or not there was a second swap in place.

        switch (srcType) {
            case kAlpha_8_SkColorType:
                // NOTE: not "identical" to gray or r8, because they set alpha = 1.0 and discard
                // their data.
            case kRGB_565_SkColorType:

            case kARGB_4444_SkColorType:

            case kRGBA_8888_SkColorType:
                // FIXME or is it better to make a canonical dst type + xtra xfer ops and then
                // cancel out the src/dst xfer ops?
                if (dstType == kBGRA_8888_SkColorType && (xferOps & XferOps::kSwapRB)) {
                    dstType = srcType;
                    xferOps &= ~XferOps::kSwapRB;
                }
                break;
            case kRGB_888x_SkColorType:
            case kBGRA_8888_SkColorType:
                // -> kRGBA_8888 + SwapRB
            case kRGBA_1010102_SkColorType:
                // -> could be a no-op if dst type is kRGB_101010x_SkColorType

            case kRGB_101010x_SkColorType:
            case kBGR_101010x_SkColorType:
                // kRGB_101010x + SwapRB

            case kBGR_101010x_XR_SkColorType:
            case kBGRA_10101010_XR_SkColorType:
            case kRGBA_10x6_SkColorType:
            case kGray_8_SkColorType:
                // -> kR8_unorm are identical in value

            case kRGBA_F16Norm_SkColorType:
                // -> kRGBA_F16_ (norm doesn't matter)
            case kRGBA_F16_SkColorType:

            case kRGBA_F32_SkColorType:

            case kR8G8_unorm_SkColorType:

            case kA16_float_SkColorType:

            case kR16G16_float_SkColorType:

            case kA16_unorm_SkColorType:

            case kR16G16_unorm_SkColorType:

            case kR16G16B16A16_unorm_SkColorType:

            case kSRGBA_8888_SkColorType:
                // kRGBA_8888_ since the actual bytes are sRGB encoded, but we will also want to
                // pair this with appropriate logic to detect when the color spaces are the same
                // or not, given expectation that the SkImage for kSRGB_CT has a linear CS.

            case kR8_unorm_SkColorType:
                // -> kGray8
        }
    }
}

struct Xfer {
    // native bothways
    // swap RB
    // upload-only
    // compressed upload only
    // wrapped only

    // + combo with these

    // three channel
    // opaque alpha type?

    std::optional<RasterPipelineOps> fUpload;
    std::optional<RasterPipelineOps> fReadback;
    // True if the alpha channel needs to be discarded on upload and padded back in on readback,
    // because SkRasterPipeline generally assumes 4 channels (565 being the exception).
    bool fThreeChannel = false;
    // True if the data is compressed, in which case dedicated codepaths have to be used to
    // upload the compressed data matching the texture format's encoding.
    bool fIsCompressed = false;

    static Xfer DirectCopy() { return {/*fUpload=*/{{}}, /*fReadback=*/{{}}}; }
    static Xfer ThreeChannel() { return {/*fUpload=*/{{}}, /*fReadback=*/{{}}, /*fThreeChannel=*/true}; }
    static Xfer SwapRB(bool threeChannel=false) { return {/*fUpload=*/{SkRasterPipelineOp::swap_rb},
                                                          /*fReadback=*/{SkRasterPipelineOp::swap_rb},
                                                          threeChannel}; }
    static Xfer CompressedUpload() { return {/*fUpload=*/{{}}, /*fReadback=*/std::nullopt, /*fTreeChannel=*/false, /*fIsCompressed=*/true}; }
    static Xfer Disabled() { return {/*fUpload=*/std::nullopt, /*fReadback=*/std::nullopt}; }
};

struct ColorFormatInfo {
    SkColorType fColorType;

    OnGPU fOnGPU;
    Xfer fXfer;

    // Some TextureFormats are opaque, but the SkColorType does not have an implicit SkAlphaType,
    // so this provides such an override.
    std::optional<SkAlphaType> fAlphaTypeOverride = std::nullopt;
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

        CASE(TextureFormat::kR8,
            {kR8_unorm_SkColorType, OnGPU::NativeSupport(), Xfer::DirectCopy()},
            {kAlpha_8_SkColorType,  OnGPU::MapRToAlpha(),   Xfer::DirectCopy()},
            {kGray_8_SkColorType,   OnGPU::MapRToGray(),    Xfer::DirectCopy()})

        CASE(TextureFormat::kR16,
            {kA16_unorm_SkColorType, OnGPU::MapRToAlpha(), Xfer::DirectCopy()})

        CASE(TextureFormat::kR16F,
            {kA16_float_SkColorType, OnGPU::MapRToAlpha(), Xfer::DirectCopy()})

        CASE(TextureFormat::kR32F, {}) // alpha 16f?

        CASE(TextureFormat::kA8,
            {kAlpha_8_SkColorType, OnGPU::NativeSupport(), Xfer::NativeSupport()})

        CASE(TextureFormat::kRG8,
            {kR8G8_unorm_SkColorType, OnGPU::NativeSupport(), Xfer::NativeSupport()})

        CASE(TextureFormat::kRG16,
            {kR16G16_unorm_SkColorType, OnGPU::NativeSupport(), Xfer::NativeSupport()})

        CASE(TextureFormat::kRG16F,
            {kR16G16_float_SkColorType, OnGPU::NativeSupport(), Xfer::NativeSupport()})

        CASE(TextureFormat::kRG32F, {}) // kR16G16_float?

        CASE(TextureFormat::kRGB8, {kRGB_888x_SkColorType, Xfer::ThreeChannel()})

        CASE(TextureFormat::kBGR8, {kRGB_888x_SkColorType, Xfer::SwapRB().threeChannel()})

        CASE(TextureFormat::kB5_G6_R5, kRGB_565_SkColorType) // NOTE: kRGB_565_SkColorType is misnamed

        CASE(TextureFormat::kR5_G6_B5, {kRGB_565_SkColorType, Xfer::SwapRB()})

        CASE(TextureFormat::kRGB16, {kR16G16B16A16_unorm_SkColorType, Xfer::ThreeChannel(), kOpaque_SkAlphaType})

        CASE(TextureFormat::kRGB16F, {kRGB_F16F16F16x_SkColorType, Xfer::ThreeChannel()})

        CASE(TextureFormat::kRGB32F, {kRGBA_F32_SkColorType, Xfer::ThreeChannel(), kOpaque_SkAlphaType})

        CASE(TextureFormat::kRGB8_sRGB, {kSRGBA_8888_SkColorType, Xfer::ThreeChannel(), kOpaque_SkAlphaType})

        CASE(TextureFormat::kBGR10_XR, kBGR_101010x_XR_SkColorType)

        CASE(TextureFormat::kRGBA8, kRGBA_8888_SkColorType,
                                   {kRGB_888x_SkColorType, OnGPU::SampleOnly_ForceOpaque()})

        CASE(TextureFormat::kRGBA16, kR16G16B16A16_unorm_SkColorType)

        CASE(TextureFormat::kRGBA16F, kRGBA_F16_SkColorType,
                                      kRGBA_F16Norm_SkColorType, // FIXME not exact match, needs shader clamping right, yes but doesn't appear to make it into any transfer handling
                                     {kRGB_F16F16F16x_SkColorType, OnGPU::SampleOnly_ForceOpaque()})

        CASE(TextureFormat::kRGBA32F, kRGBA_F32_SkColorType)

        CASE(TextureFormat::kRGB10_A2, kRGBA_1010102_SkColorType,
                                      {kRGB_101010x_SkColorType, OnGPU::SampleOnly_ForceOpaque()},
                                      {kBGRA_1010102_SkColorType, Xfer::SwapRB()})

        CASE(TextureFormat::kRGBA8_sRGB, kSRGBA_8888_SkColorType)

        CASE(TextureFormat::kBGRA8, kBGRA_8888_SkColorType,
                                   {kRGB_888x_SkColorType, SampleRGBx(), DirectUpload()}) // FIXME swap RB too

        CASE(TextureFormat::kBGR10_A2, kBGRA_1010102_SkColorType,
                                      {kBGR_101010x_SkColorType, OnGPU::SampleOnly_ForceOpaque()},
                                      {kRGBA_1010102_SkColorType, Xfer::SwapRB()})

        CASE(TextureFormat::kBGRA8_sRGB, {kSRGBA_8888_SkColorType, Xfer::SwapRB()})

        CASE(TextureFormat::kABGR4, kARGB_4444_SkColorType) // NOTE: kARGB_4444_SkColorType is misnamed

        CASE(TextureFormat::kARGB4, {kARGB_4444_SkColorType, Xfer::SwapRB()}) // FIXME do we need to do shader-swizzle instead for some historical reason?


        CASE(TextureFormat::kBGRA10x6_XR, kBGRA_10101010_XR_SkColorType)

// FIXME if we just used bitflags, we could combine ThreeChannel | SwapRB | UploadOnly
// and could add a Compressed flag, and not bother with specifying the compression type
        CASE(TextureFormat::kRGB8_ETC2, {kRGB_888x_SkColorType, OnGPU::SampleOnly(), Xfer::CompressedUpload(SkTextureCompressionType::kETC2_RGB8_UNORM)})

        CASE(TextureFormat::kRGB8_ETC2_sRGB,
             {kSRGBA_8888_SkColorType, OnGPU::SampleOnly(), Xfer::CompressedUpload(SkTextureCompressionType::kETC2_RGB8_UNORM), kOpaque_SkAlphaType})

        CASE(TextureFormat::kRGB8_BC1,
             {kRGB_888x_SkColorType, OnGPU::SampleOnly(), Xfer::CompressedUpload(SkTextureCompressionType::kBC1_RGB8_UNORM)})

        CASE(TextureFormat::kRGBA8_BC1,
             {kRGBA_8888_SkColorType, OnGPU::SampleOnly(), Xfer::CompressedUpload(SkTextureCompressionType::kBC1_RGBA8_UNORM)})

        CASE(TextureFormat::kRGBA8_BC1_sRGB,
             {kSRGBA_8888_SkColorType, OnGPU::SampleOnly(), Xfer::CompressedUpload(SkTextureCompressionType::kBC1_RGBA8_UNORM)})

          // YUV uses kRGBx
        CASE(TextureFormat::kYUV8_P2_420,
             {kRGB_888x_SkColorType, OnGPU::SampleOnly(), Xfer::Disabled()})
        CASE(TextureFormat::kYUV8_P3_420,
             {kRGB_888x_SkColorType, OnGPU::SampleOnly(), Xfer::Disabled()})

        CASE(TextureFormat::kYUV10x6_P2_420,
             {kRGB_101010x_SkColorType, OnGPU::SampleOnly(), Xfer::Disabled()},
             {kRGBA_1010102_SkColorType, OnGPU::SampleOnly(), Xfer::Disabled()})

        CASE(TextureFormat::kExternal,
            {kRGBA_8888_SkColorType, OnGPU::SampleOnly(), Xfer::Disabled()})

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
