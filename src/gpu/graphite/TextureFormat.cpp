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
 #include "src/core/SkImageInfoPriv.h"
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

bool TextureFormatAutoClamps(TextureFormat format) {
    // Floating point formats, extended range formats, and non-normalized integer formats do not
    // auto-clamp. Everything behaves like an unsigned normalized number.
    return !(TextureFormatIsFloatingPoint(format) ||
             format == TextureFormat::kBGR10_XR ||
             format == TextureFormat::kBGRA10x6_XR ||
             format == TextureFormat::kS8);
}

bool TextureFormatIsFloatingPoint(TextureFormat format) {
    switch (format) {
        // Floating point formats
        case TextureFormat::kR16F:           [[fallthrough]];
        case TextureFormat::kR32F:
        case TextureFormat::kRG16F:
        case TextureFormat::kRG32F:
        case TextureFormat::kRGB16F:
        case TextureFormat::kRGB32F:
        case TextureFormat::kRGBA16F:
        case TextureFormat::kRGBA32F:
        case TextureFormat::kD32F:
        case TextureFormat::kD32F_S8:        return true;

        // Everything else is unorm, unorm-srgb, fixed point, or integral
        case TextureFormat::kUnsupported:    [[fallthrough]];
        case TextureFormat::kR8:
        case TextureFormat::kR16:
        case TextureFormat::kA8:
        case TextureFormat::kRG8:
        case TextureFormat::kRG16:
        case TextureFormat::kRGB8:
        case TextureFormat::kBGR8:
        case TextureFormat::kB5_G6_R5:
        case TextureFormat::kR5_G6_B5:
        case TextureFormat::kRGB16:
        case TextureFormat::kRGB8_sRGB:
        case TextureFormat::kBGR10_XR:
        case TextureFormat::kRGBA8:
        case TextureFormat::kRGBA16:
        case TextureFormat::kRGB10_A2:
        case TextureFormat::kRGBA8_sRGB:
        case TextureFormat::kBGRA8:
        case TextureFormat::kBGR10_A2:
        case TextureFormat::kBGRA8_sRGB:
        case TextureFormat::kABGR4:
        case TextureFormat::kARGB4:
        case TextureFormat::kBGRA10x6_XR:
        case TextureFormat::kRGB8_ETC2:
        case TextureFormat::kRGB8_ETC2_sRGB:
        case TextureFormat::kRGB8_BC1:
        case TextureFormat::kRGBA8_BC1:
        case TextureFormat::kRGBA8_BC1_sRGB:
        case TextureFormat::kYUV8_P2_420:
        case TextureFormat::kYUV8_P3_420:
        case TextureFormat::kYUV10x6_P2_420:
        case TextureFormat::kExternal:
        case TextureFormat::kS8:
        case TextureFormat::kD16:
        case TextureFormat::kD24_S8:          return false;
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

// Supporting implementation details for TextureFormatInfo and SkColorType conversions
// ------------------------------------------------------------------------------------------------
namespace {

SkColorType restore_alpha(SkColorType ct) {
    switch (ct) {
        case kRGB_888x_SkColorType: return kRGBA_8888_SkColorType;
        case kRGB_101010x_SkColorType: return kRGBA_1010102_SkColorType;
        case kBGR_101010x_SkColorType: return kBGRA_1010102_SkColorType;
        case kRGB_F16F16F16x_SkColorType: return kRGBA_F16_SkColorType;

        default:
            SkASSERT(SkColorTypeChannelFlags(ct) != kRGB_SkColorChannelFlags);
            return ct;
    }
}

SkColorType swap_rb(SkColorType ct) {
    switch (ct) {
        case kRGBA_8888_SkColorType: return kBGRA_8888_SkColorType;
        case kRGBA_1010102_SkColorType: return kBGRA_8888_SkColorType;
        case kRGB_101010x_SkColorType: return kBGR_101010x_SkColorType;

        case kBGRA_8888_SkColorType: return kRGBA_8888_SkColorType;
        case kBGRA_1010102_SkColorType: return kRGBA_1010102_SkColorType;
        case kBGRA_10101010_XR_SkColorType: return kRGB_101010x_SkColorType;

        default: return kUnknown_SkColorType;
    }
}

SkColorType consolidate(SkColorType ct) {
    switch (ct) {
        case kRGBA_F16Norm_SkColorType: return kRGBA_F16_SkColorType;
        case kSRGBA_8888_SkColorType: return kRGBA_8888_SkColorType;
        case kGray_8_SkColorType: return kR8_unorm_SkColorType;
        default: return ct;
    }
}

enum XferOps : uint8_t {
    kIdentity = 0b000, // No conversion needed -> name it direct copy for legibility in tables?
    kDropAlpha  = 0b001, // Drop the alpha channel of the SkColorType to store just 3 channels
    kPadAlpha   = 0b010, // Padd each pixel's data with an opaque alpha value to restore 4 channels
    kSwapRB     = 0b100, // Swap red/blue channels within each pixel

    kCompressed = 0b1000, // Used to enforce that other aspects of the conversion fn require no other steps? e.g. fails we don't return identity
    // Or do we embody this with something that still collects the XferOps masks in a new struct that
    // can track disabled and compressed?
    // leaning towards just storing uplaod and readback enum bitmasks on ColorFormatInfo

    kDisabled  = 0b10000 // or wrap in optional?
};
SK_MAKE_BITMASK_OPS(XferOps)

using XferConversionFn = void (*)(int width, int height,
                                  const void* src, size_t srcRowBytes,
                                  void* dst, size_t dstRowBytes);

// FIXME: What about SkAlphaType conversions here? what were we doing before in graphite/ganesh?
std::pair<XferConversionFn, bool/*isIdentity*/> get_conversion_fn(
        SkEnumBitMask<XferOps> preSrcType,
        SkColorType srcType,
        SkColorType dstType,
        SkEnumBitMask<XferOps> postDstType) {
    // One of srcType or dstType represents a compatible colortype with a TextureFormat, so it
    // won't be kUnknown. The other represents the request (either src or dst) and should have
    // been validated to raise an error to the client.
    SkASSERT(srcType != kUnknown_SkColorType && dstType != kUnknown_SkColorType);

    // For a CPU->GPU transfer, `preSrcType` ops should be the identity, since the source CPU data
    // should be exactly described by `srcType`. For this direction, `postDstType` adds extra ops
    // to finish converting from a format-compatible `dstType` to the actual TextureFormat.
    //
    // Conversely, for a GPU->CPU transfer, the `postDstType` ops should be the identity because the
    // destination CPU data should be exactly described by `dstType`. In this case, `preSrcType`
    // adds extra ops to first convert from the TextureFormat to the format-compatible `srcType`.
    SkASSERT(preSrcType == XferOps::kIdentity || postDstType == kIdentity);
    SkEnumBitMask<XferOps> xferOps = preSrcType | postDstType;

    // If the transfer is with compressed data, there cannot be any other transfer ops to modify
    // the content. For the same reasons, color type conversion is not supported.
    SkASSERT(!(xferOps & XferOps::kCompressed) || xferOps == XferOps::kCompressed);
    if ((xferOps & XferOps::kDisabled) ||
        ((xferOps & XferOps::kCompressed) && srcType != dstType)) {
        // Cannot convert because it's disabled explicitly or cannot decompress / re-compress.
        return {nullptr, false};
    }

    // A TextureFormat <-> SkColorType mapping shouldn't apply both of these for a single transfer.
    SkASSERT(!((xferOps & XferOps::kDropAlpha) && (xferOps & XferOps::kPadAlpha)));

    // Strip out color type variations that don't impact bit equivalence for the transfer.
    // For the dst color type, this extends to mapping RGBx formats to RGBA since the data will
    // still be interpreted by the caller as the original RGBx so any alpha is still ignored.
    SkColorType effectiveSrcType = consolidate(srcType);
    SkColorType effectiveDstType = consolidate(dstType);
    if (effectiveDstType != effectiveSrcType) {
        // Can always further consolidate the dst to from RGBx to RGBA, since the caller will
        // continue to use RGBx and ignore the alpha.
        effectiveDstType = restore_alpha(effectiveDstType);
        // And if the dst alpha channel is being ignored anyways, then we can also consolidate the
        // src from RGBx to RGBA to increase chances of an identity color type conversion.
        if (!(SkColorTypeChannelFlags(dstType) & kAlpha_SkColorChannelFlag) ||
            (xferOps & XferOps::kDropAlpha)) {
            effectiveSrcType = restore_alpha(effectiveSrcType);
        }
    }

    SkColorType swappedDstType   = swap_rb(effectiveDstType); // may be kUnknown if no swap exists
    if (xferOps & XferOps::kSwapRB) {
        if (swappedDstType != kUnknown_SkColorType) {
            // Turn off the bespoke RB swap and adjust the effectiveDstType to be the swapped type.
            xferOps &= ~XferOps::kSwapRB;
            // Conversion will either now be a no-op or everything is optimally handled by SkRP.
            effectiveDstType = swappedDstType;
        } else {
            // Try swapping RB in the src color type instead (we don't do this by default because
            // w/o the `restore_alpha` consolidation, it's less likely to have a valid swapped CT).
            SkColorType swappedSrcType = swap_rb(effectiveSrcType);
            if (swappedSrcType != kUnknown_SkColorType) {
                xferOps &= ~XferOps::kSwapRB;
                effectiveSrcType = swappedSrcType;
            }
        }
    } else {
        if (effectiveSrcType == swappedDstType) {
            // Turn on bespoke RB swapping and set effective types to avoid SkRP entirely
            xferOps |= XferOps::kSwapRB;
            effectiveDstType = effectiveSrcType;
        } // else either already a no-op or everything is optimally handled by SkRasterPipeline
    }

    // FIXME if we create the pipeline here, do any stack allocations that it used remain valid?
    // or should we create the SkRasterPipeline within the returned function?

    return {[&](int width, int height,
               const void* src, size_t srcRowBytes,
               void* dst, size_t dstRowBytes) {
            const char* srcBytes = static_cast<const char*>(src);
            char* dstBytes = static_cast<char*>(dst);

            size_t trimRowBytes = width * bpp; // FIXME where do we get this from?

            if (xferOps == kDirectCopy && !pipelineFn && trimRowBytes == srcRowBytes && trimRowBytes == dstRowBytes) {
                // Full 2D identity copy to match SkRectMemcpy fast path
                memcpy(dstBytes, srcBytes, trimRowBytes * height);
            }

            SkRasterPipeline_<256> pipeline;
            pipeline.appendLoad(effectiveSrcType, srcBytes);
            pipeline.appendStore(effectiveDstType, dstBytes);
            auto pipelineFn = pipeline.compile();

            // shit, if the load and stores require the addresses, we have to rewrite the entire
            // pipeline if we introduce a temporary

            // Although actually it's passing in the MemoryCtx pointers, so if I instead just rewrite
            // the memory ctx addresses on each loop through, it should be picked up automatically?
            // Yes, that should be correct because I will always say x =0, y=0 and height = 1
            // we will never advance them.

            // NOTE: If xferOps is kIdentity and there's no pipeline conversion, this degrades
            // into the same behavior as SkRectMemcpy. If there is only a pipeline conversion, this
            // degrades into SkConvertPixels. If there are both kinds of conversions, this only
            // requires an extra allocation sized to a row of pixels, instead of making a full
            // intermediate copy.
            for (int y = 0; y < height; ++y) {
                if (xferOps != XferOps::kIdentity && pipelineFn) {
                    if (xferOps & XferOps::kPadAlpha) {
                        // Run xfer ops before RP to bring it back to expected 4 channels
                        applyXferOps(srcBytes, tmpBuffer);
                        applyRP(tmpBuffer, dstBytes);
                    } else {
                        // Run xfer ops after RP, since this can only be a postDstType op or just
                        // swapping RB, which can commute to before or after RP without impact.
                        applyRP(srcBytes, tmpBuffer);
                        applyXferOps(tmpBuffer, dstBytes);
                    }
                } else if (pipelineFn) {
                    applyRP(srcBytes, dstBytes);
                } else if (xferOps != XferOps::kIdentity) {
                    applyXferOps(srcBytes, dstBytes);
                } else {
                    // Identity copy the whole row and carry on
                    memcpy(dstBytes, srcBytes, trimRowBytes);
                }

                srcBytes += srcRowBytes;
                dstBytes += dstRowBytes;
            }

        },
        /*isIdentity=*/xferOps == XferOps::kIdentity && effectiveSrcType == effectiveDstType};
}

//     if (xferOps == XferOps::kIdentity && effectiveSrcType == effectiveDstType) {
//         // Identity entirely so can be returned as a function that calls SkRectMemcpy
//         // Or can it all just be one for loop that also skips compiling any pipeline?
//         // how much would an if(pipelineFn) branch be each row? pretty minimal I think
//     } else {
//         // Function that has to perform conversions.
//         // If we have both, then we need to do a temporary scanline and chain them into dst buffer
//         // If we have just xferOps then do custom scanline conversion into dst buffer
//         // If we have just colortype diffs, call SkConvertPixels?
//         //
//         // Or is it clearer to do one scanline like loop structure that maybe skips scanline
//         // allocation? Probably...
//     }
// }

using Upload = XferOps;
using Readback = XferOps;
using Sample = Swizzle;
using Render = Swizzle;

static constexpr std::optional<Swizzle> RenderDisabled = std::nullopt;
enum NativeSupport_Ctor { NativeSupport };

struct ColorFormatInfo {
    SkColorType fColorType;

    Swizzle fReadSwizzle;
    std::optional<Swizzle> fWriteSwizzle;
    SkEnumBitMask<XferOps> fUploadOps   = XferOps::kIdentity;
    SkEnumBitMask<XferOps> fReadbackOps = XferOps::kIdentity;

    // Some TextureFormats are opaque, but the SkColorType does not have an implicit SkAlphaType,
    // so this provides such an override.
    std::optional<SkAlphaType> fAlphaTypeOverride;

    ColorFormatInfo(SkColorType ct, NativeSupport_Ctor)
            : ColorFormatInfo(ct, Swizzle::RGBA()) {}

    ColorFormatInfo(SkColorType ct,
                    SkEnumBitMask<XferOps> uploadOps,
                    SkEnumBitMask<XferOps> readbackOps,
                    std::optional<SkAlphaType> alphaTypeOverride = std::nullopt)
            : ColorFormatInfo(ct, Swizzle::RGBA(), uploadOps, readbackOps, alphaTypeOverride) {}
    ColorFormatInfo(SkColorType ct,
                    std::optional<Swizzle> writeSwizzle,
                    SkEnumBitMask<XferOps> uploadOps,
                    SkEnumBitMask<XferOps> readbackOps,
                    std::optional<SkAlphaType> alphaTypeOverride = std::nullopt)
            : ColorFormatInfo(ct, Swizzle::RGBA(), writeSwizzle, uploadOps, readbackOps, alphaTypeOverride) {}

    ColorFormatInfo(SkColorType ct,
                    Swizzle readSwizzle,
                    std::optional<Swizzle> writeSwizzle = Swizzle::RGBA(),
                    SkEnumBitMask<XferOps> uploadOps = XferOps::kIdentity,
                    SkEnumBitMask<XferOps> readbackOps = XferOps::kIdentity,
                    std::optional<SkAlphaType> alphaTypeOverride = std::nullopt)
            : fColorType(ct)
            , fReadSwizzle(readSwizzle)
            , fWriteSwizzle(writeSwizzle)
            , fUploadOps(uploadOps)
            , fReadbackOps(readbackOps)
            , fAlphaTypeOverride(alphaTypeOverride) {}
};

struct ColorFormatInfos {
    ColorFormatInfos(std::initializer_list<ColorFormatInfo> formatInfos) {
        fColorTypes.reserve(formatInfos.size());
        fFormatInfos.reserve(formatInfos.size());

        for (const ColorFormatInfo& i : formatInfos) {
            fFormatInfos.push_back(i);
            fColorTypes.push_back(i.fColorType);
        }
    }

    // Parallel arrays, fFormats[i] == fFormatInfos[i].fColorType
    skia_private::STArray<2, SkColorType> fColorTypes;
    skia_private::STArray<2, ColorFormatInfo> fFormatInfos;
};

// TextureFormat <-> SkColorType mappings
///////////////////////////////////////////////////////////////////////////////////////////////////

const ColorFormatInfos& get_infos_for_format(TextureFormat format) {

#define CASE(F, ...) case F: { static const ColorFormatInfos kInfo{{__VA_ARGS__}}; return kInfo; }

    switch (format) {
        CASE(TextureFormat::kUnsupported,    /* no colortypes */)

        // Single channel
        CASE(TextureFormat::kR8,
            {kR8_unorm_SkColorType,           NativeSupport},
            {kAlpha_8_SkColorType,            Sample("000r"), Render("a000")},
            {kGray_8_SkColorType,             Sample("000r"), Render("a000")})

        CASE(TextureFormat::kR16,
            {kA16_unorm_SkColorType,          Sample("000r"), Render("a000")})

        CASE(TextureFormat::kR16F,
            {kA16_float_SkColorType,          Sample("000r"), Render("a000")})

        CASE(TextureFormat::kR32F,
            {kA16_float_SkColorType,          Sample("000r"),
                                              Render("a000"),
                                              Upload::kDisabled,
                                              Readback::kDisabled})

        CASE(TextureFormat::kA8,
            {kAlpha_8_SkColorType,            NativeSupport})

        // Two channel
        CASE(TextureFormat::kRG8,
            {kR8G8_unorm_SkColorType,         NativeSupport})

        CASE(TextureFormat::kRG16,
            {kR16G16_unorm_SkColorType,       NativeSupport})

        CASE(TextureFormat::kRG16F,
            {kR16G16_float_SkColorType,       NativeSupport})

        CASE(TextureFormat::kRG32F,
            {kR16G16_float_SkColorType,       Upload::kDisabled,
                                              Readback::kDisabled})

        // Three channel
        CASE(TextureFormat::kRGB8,
            {kRGB_888x_SkColorType,           Upload::kDropAlpha, Readback::kPadAlpha})

        CASE(TextureFormat::kBGR8,
            {kRGB_888x_SkColorType,           Upload::kDropAlpha  | kSwapRB,
                                              Readback::kPadAlpha | kSwapRB})

        // NOTE: kRGB_565_SkColorType is misnamed
        CASE(TextureFormat::kB5_G6_R5,
            {kRGB_565_SkColorType,            NativeSupport})

        CASE(TextureFormat::kR5_G6_B5,
            {kRGB_565_SkColorType,            Upload::kSwapRB, Readback::kSwapRB})

        CASE(TextureFormat::kRGB16,
            {kR16G16B16A16_unorm_SkColorType, Upload::kDropAlpha,
                                              Readback::kPadAlpha,
                                              kOpaque_SkAlphaType})

        CASE(TextureFormat::kRGB16F,
            {kRGB_F16F16F16x_SkColorType,     Upload::kDropAlpha, Readback::kPadAlpha})

        CASE(TextureFormat::kRGB32F,
            {kRGBA_F32_SkColorType,           Upload::kDropAlpha,
                                              Readback::kPadAlpha,
                                              kOpaque_SkAlphaType})

        CASE(TextureFormat::kRGB8_sRGB,
            {kSRGBA_8888_SkColorType,         Upload::kDropAlpha,
                                              Readback::kPadAlpha,
                                              kOpaque_SkAlphaType})

        CASE(TextureFormat::kBGR10_XR,
            {kBGR_101010x_XR_SkColorType,     NativeSupport})

        // Four channel
        CASE(TextureFormat::kRGBA8,
            {kRGBA_8888_SkColorType,          NativeSupport},
            {kRGB_888x_SkColorType,           Sample("rgb1"), RenderDisabled})

        CASE(TextureFormat::kRGBA16,
            {kR16G16B16A16_unorm_SkColorType, NativeSupport})

        CASE(TextureFormat::kRGBA16F,
            {kRGBA_F16_SkColorType,           NativeSupport},
            {kRGBA_F16Norm_SkColorType,       NativeSupport},
            {kRGB_F16F16F16x_SkColorType,     Sample("rgb1"), RenderDisabled})

        CASE(TextureFormat::kRGBA32F,
            {kRGBA_F32_SkColorType,           NativeSupport})

        CASE(TextureFormat::kRGB10_A2,
            {kRGBA_1010102_SkColorType,       NativeSupport},
            {kRGB_101010x_SkColorType,        Sample("rgb1"),  RenderDisabled},
            {kBGRA_1010102_SkColorType,       Upload::kSwapRB, Readback::kSwapRB})

        CASE(TextureFormat::kRGBA8_sRGB,
            {kSRGBA_8888_SkColorType,         NativeSupport})

        CASE(TextureFormat::kBGRA8,
            {kBGRA_8888_SkColorType,          NativeSupport},
            {kRGB_888x_SkColorType,           Sample("rgb1"),
                                              RenderDisabled,
                                              Upload::kSwapRB,
                                              Readback::kSwapRB})

        CASE(TextureFormat::kBGR10_A2,
            {kBGRA_1010102_SkColorType,       NativeSupport},
            {kBGR_101010x_SkColorType,        Sample("rgb1"),  RenderDisabled},
            {kRGBA_1010102_SkColorType,       Upload::kSwapRB, Readback::kSwapRB})

        CASE(TextureFormat::kBGRA8_sRGB,
            {kSRGBA_8888_SkColorType,         Upload::kSwapRB, Readback::kSwapRB})

        // NOTE: kARGB_4444_SkColorType is misnamed
        CASE(TextureFormat::kABGR4,
            {kARGB_4444_SkColorType,          NativeSupport})
        CASE(TextureFormat::kARGB4,
            {kARGB_4444_SkColorType,          Upload::kSwapRB, Readback::kSwapRB})

        CASE(TextureFormat::kBGRA10x6_XR,
            {kBGRA_10101010_XR_SkColorType,   NativeSupport})

        // Compressed
        CASE(TextureFormat::kRGB8_ETC2,
            {kRGB_888x_SkColorType,           RenderDisabled,
                                              Upload::kCompressed,
                                              Readback::kDisabled})

        CASE(TextureFormat::kRGB8_ETC2_sRGB,
            {kSRGBA_8888_SkColorType,         RenderDisabled,
                                              Upload::kCompressed,
                                              Readback::kDisabled,
                                              kOpaque_SkAlphaType})

        CASE(TextureFormat::kRGB8_BC1,
            {kRGB_888x_SkColorType,           RenderDisabled,
                                              Upload::kCompressed,
                                              Readback::kDisabled})

        CASE(TextureFormat::kRGBA8_BC1,
            {kRGBA_8888_SkColorType,          RenderDisabled,
                                              Upload::kCompressed,
                                              Readback::kDisabled})

        CASE(TextureFormat::kRGBA8_BC1_sRGB,
            {kSRGBA_8888_SkColorType,         RenderDisabled,
                                              Upload::kCompressed,
                                              Readback::kDisabled})

        // Multi-planar
        CASE(TextureFormat::kYUV8_P2_420,
            {kRGB_888x_SkColorType,           RenderDisabled,
                                              Upload::kDisabled,
                                              Readback::kDisabled})

        CASE(TextureFormat::kYUV8_P3_420,
            {kRGB_888x_SkColorType,           RenderDisabled,
                                              Upload::kDisabled,
                                              Readback::kDisabled})

        CASE(TextureFormat::kYUV10x6_P2_420,
            {kRGB_101010x_SkColorType,        RenderDisabled,
                                              Upload::kDisabled,
                                              Readback::kDisabled})

        CASE(TextureFormat::kExternal,
            {kRGBA_8888_SkColorType,          RenderDisabled,
                                              Upload::kDisabled,
                                              Readback::kDisabled})

        // Non-color
        CASE(TextureFormat::kS8,              /* no colortypes */)
        CASE(TextureFormat::kD16,             /* no colortypes */)
        CASE(TextureFormat::kD32F,            /* no colortypes */)
        CASE(TextureFormat::kD24_S8,          /* no colortypes */)
        CASE(TextureFormat::kD32F_S8,         /* no colortypes */)
    }

#undef CASE
}

const ColorFormatInfo* get_color_info(TextureFormat format, SkColorType ct) {
    for (const ColorFormatInfo& info : get_infos_for_format(format).fFormatInfos) {
        if (info.fColorType == ct) {
            return &info;
        }
    }
    return nullptr;
}

// Preferred TextureFormats (not derived from compatible SkColorTypes since many TextureFormats map
// to the same SkColorType but shouldn't be chosen by default).
SkSpan<const TextureFormat> get_preferred_formats(SkColorType ct) {

#define N(...) std::size({__VA_ARGS__})
#define CASE(C, ...) case C: { \
        static const std::array<TextureFormat, N(__VA_ARGS__)> kFormats{{__VA_ARGS__}}; \
        return SkSpan(kFormats); }

    switch (ct) {
        case kUnknown_SkColorType:  return {};
        // There is currently no TextureFormat that has a 10x6 bit representation without an
        // extended range. If we implemented an XferOp to move between 10-bit and 16-bit unorms,
        // then TextureFormat::kRGBA16 would be reasonable, but there's no reason clients shouldn't
        // just use kR16G16B16A16_unorm_SkColorType as the GPU's SkColorType.
        case kRGBA_10x6_SkColorType: return {};

        // In almost all cases, there is a single best format match that is supported by all
        // backends, or only supported by a single backend and there's no reasonable fallback. In
        // a few cases, a fallback TextureFormat is provided when the best match is not supported
        // on all backends.


        // FIXME: just let rgba8 and bgra8 be used for both color types...??
        // Would that be here as part of preferences? Or just in whether or not it's technically supported?

        CASE(kAlpha_8_SkColorType,            TextureFormat::kA8,       TextureFormat::kR8)
        CASE(kRGB_565_SkColorType,            TextureFormat::kB5_G6_R5)
        CASE(kARGB_4444_SkColorType,          TextureFormat::kABGR4)
        CASE(kRGBA_8888_SkColorType,          TextureFormat::kRGBA8)
        CASE(kRGB_888x_SkColorType,           TextureFormat::kRGB8,     TextureFormat::kRGBA8)
        CASE(kBGRA_8888_SkColorType,          TextureFormat::kBGRA8)
        CASE(kRGBA_1010102_SkColorType,       TextureFormat::kRGB10_A2)
        CASE(kBGRA_1010102_SkColorType,       TextureFormat::kBGR10_A2, TextureFormat::kRGB10_A2)
        CASE(kRGB_101010x_SkColorType,        TextureFormat::kRGB10_A2)
        CASE(kBGR_101010x_SkColorType,        TextureFormat::kBGR10_A2, TextureFormat::kRGB10_A2)
        CASE(kBGR_101010x_XR_SkColorType,     TextureFormat::kBGR10_XR)
        CASE(kBGRA_10101010_XR_SkColorType,   TextureFormat::kBGRA10x6_XR)
        CASE(kGray_8_SkColorType,             TextureFormat::kR8)
        CASE(kRGBA_F16Norm_SkColorType,       TextureFormat::kRGBA16F)
        CASE(kRGBA_F16_SkColorType,           TextureFormat::kRGBA16F)
        CASE(kRGB_F16F16F16x_SkColorType,     TextureFormat::kRGBA16F)
        CASE(kRGBA_F32_SkColorType,           TextureFormat::kRGBA32F)
        CASE(kR8G8_unorm_SkColorType,         TextureFormat::kRG8)
        CASE(kA16_float_SkColorType,          TextureFormat::kR16F)
        CASE(kR16G16_float_SkColorType,       TextureFormat::kRG16F)
        CASE(kA16_unorm_SkColorType,          TextureFormat::kR16)
        CASE(kR16G16_unorm_SkColorType,       TextureFormat::kRG16)
        CASE(kR16G16B16A16_unorm_SkColorType, TextureFormat::kRGBA16)
        CASE(kSRGBA_8888_SkColorType,         TextureFormat::kRGBA8_sRGB)
        CASE(kR8_unorm_SkColorType,           TextureFormat::kR8)
    }
}

} // anonymous namespace

// TextureFormatInfo
// ------------------------------------------------------------------------------------------------

SkSpan<SkColorType> TextureFormatInfo::compatibleColorTypes() const {
    return SkSpan<SkColorType>(get_infos_for_format(fFormat).fColorTypes);
}

SkEnumBitMask<TextureFormatInfo::Supports> TextureFormatInfo::supportedOperations(
        Tiling tiling, SkColorType ct) const {
    const ColorFormatInfo* ctInfo = get_color_info(fFormat, ct);
    if (!ctInfo) {
        // SkColorType is not compatible with the TextureFormat
        return Supports::kNone;
    }

    SkEnumBitMask<Supports> formatSupport = this->supportedOperations(tiling);
    SkEnumBitMask<Supports> colorSupport = Supports::kNone;
    // If there's a ColorFormatInfo, supporting kTexture/kStorage is implicit (relying on
    // `formatSupport` to disable storage support based on `tiling`)
    colorSupport = Supports::kTexture | Supports::kStorage;
    if (ctInfo->fWriteSwizzle.has_value()) {
        // By default we assume that if the format can be rendered to, it can also be resolved to,
        // but this can be disabled by `formatSupport`.
        colorSupport |= Supports::kRender | Supports::kResolve;
    }
    if (!(ctInfo->fUploadOps & XferOps::kDisabled)) {
        colorSupport |= Supports::kUpload;
    }
    if (!(ctInfo->fReadbackOps & XferOps::kDisabled)) {
        colorSupport |= Supports::kReadback;
    }
    return formatSupport & colorSupport;
}

Swizzle TextureFormatInfo::getReadSwizzle(SkColorType ct) const {
    // SkColorType compatibility is assumed
    SkASSERT(get_color_info(fFormat, ct));
    return get_color_info(fFormat, ct)->fReadSwizzle;
}
Swizzle TextureFormatInfo::getWriteSwizzle(SkColorType ct) const {
    // SkColorType compatibility is assumed
    SkASSERT(get_color_info(fFormat, ct) && get_color_info(fFormat, ct)->fWriteSwizzle.has_value());
    return *get_color_info(fFormat, ct)->fWriteSwizzle;
}

TextureFormatInfo::DataTransferFn TextureFormatInfo::getUploadTransferFn(
        SkColorType cpuSrcType, SkColorType gpuDstType) const {
    const ColorFormatInfo* info = get_color_info(fFormat, gpuDstType);
    if (!info) {
        // SkColorType is not compatible with the TextureFormat
        return nullptr;
    }

    // Logical conversion flow: src data -> identity conversion to cpuSrcType
    //                                   -> CPU conversion to gpuDstType
    //                                   -> upload XferOps to convert to TextureFormat layout
    //                                   -> write dst data to GPU
    return get_conversion_fn(XferOps::kIdentity, cpuSrcType, gpuDstType, info->fUploadOps);
}

TextureFormatInfo::DataTransferFn TextureFormatInfo::getReadbackTransferFn(
        SkColorType gpuSrcType, SkColorType cpuDstType) const {
    const ColorFormatInfo* info = get_color_info(fFormat, gpuSrcType);
    if (!info) {
        return nullptr;
    }

    // Logical conversion flow: src data -> mapped or copied to buffer from GPU
    //                                   -> readback XferOps to convert from format to gpuSrcType
    //                                   -> CPU conversion to cpuDstType into dst buffer
    return get_conversion_fn(info->fReadbackOps, gpuSrcType, cpuDstType, XferOps::kIdentity);
}

SkSpan<const TextureFormat> PreferredTextureFormats(SkColorType ct) {
    return get_preferred_formats(ct);
}

} // namespace skgpu::graphite
