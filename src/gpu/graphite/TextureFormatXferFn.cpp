/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/TextureFormatXferFn.h"

#include "include/core/SkColorType.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkFloatBits.h"
#include "src/core/SkHalf.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkVx.h"

#include <functional>

namespace skgpu::graphite {

namespace {

using TF = TextureFormat;

// This is intentionally not a class enum and is distinct from FormatXferOps for two reasons:
// 1. We can't use SkEnumBitmask inside template parameters for the row bit manipulation functions.
// 2. It's convenient to split the FormatXferOps into specific ops based on their conversion
//    direction that doesn't need to be exposed in the public API.
enum ExtendedFormatXferOp : uint8_t {
    kDropAlpha   = 0x1,  // FormatXferOp::kDropAlpha for CPU->GPU conversion
    kPadAlpha    = 0x2,  // FormatXferOp::kDropAlpha for GPU->CPU conversion
    kSwapRB      = 0x4,  // FormatXferOp::kSwapRB behaves the same for either conversion direction
    kForceOpaque = 0x8,  // RGBx handling to avoid raster pipeline is the same in both directions
    kIgnoreSrc   = 0x10, // The output value does not depend on the src, will be either 0 or 1.
};

using XferRowFn = std::function<void(const char* src, char* dst, int width)>;

template <typename PxVec, typename PixelFn /* [](PxVec) -> PxVec */>
XferRowFn create_xfer_row_fn(int n, int srcBpp, int dstBpp, PixelFn applyPixel) {
    // PxVec should be sufficient to hold N src and dst pixels, and should match at least one
    SkASSERT(sizeof(PxVec) >= (size_t)srcBpp*n && sizeof(PxVec) >= (size_t)dstBpp*n);
    SkASSERT(sizeof(PxVec) == (size_t)srcBpp*n || sizeof(PxVec) == (size_t)dstBpp*n);

    return [n, srcBpp, dstBpp, applyPixel](const char* src, char* dst, int width) {
        const int srcBppN = n * srcBpp;
        const int dstBppN = n * dstBpp;

        PxVec pixel{};
        while (width >= n) {
            memcpy(&pixel, src, srcBppN);
            pixel = applyPixel(pixel);
            memcpy(dst, &pixel, dstBppN);

            width -= n;
            src += srcBppN;
            dst += dstBppN;
        }

        if (width > 0) {
            // Process tail that is less than a full vector
            SkASSERT(width < n);
            memcpy(&pixel, src, width * srcBpp);
            pixel = applyPixel(pixel);
            memcpy(dst, &pixel, width * dstBpp);
        }
    };
}

template <typename PxVec, typename PixelFn /* [](PxVec) -> PxVec */>
XferRowFn create_ignore_src_row_fn(int n, PxVec srcValue, int dstBpp, PixelFn applyPixel) {
    // PxVec should be sufficient to hold N dst pixels
    SkASSERT(sizeof(PxVec) >= (size_t)dstBpp*n);

    return [n, srcValue, dstBpp, applyPixel](const char* /* src */, char* dst, int width) {
        const int dstBppN = n * dstBpp;

        PxVec pixel{};
        while (width >= n) {
            pixel = srcValue;
            pixel = applyPixel(pixel);
            memcpy(dst, &pixel, dstBppN);

            width -= n;
            dst += dstBppN;
        }

        if (width > 0) {
            // Process tail that is less than a full vector
            SkASSERT(width < n);
            pixel = srcValue;
            pixel = applyPixel(pixel);
            memcpy(dst, &pixel, width * dstBpp);
        }
    };
}

// N represents the number of pixels being processed; each pixel is packed into a single element of
// the Vec. To implement kSwapRB, this requires that the R and B channels have the same number of
// bits. It arbitrarily requires that R is originally in the lower-significance bits relative to B.
//
// This assumes that all packed formats have a unorm data type, so opaque alpha can be inferred
// from the number of bits for that field.
template <typename Px, int N, uint8_t Ops,
          Px RShift, Px BShift, Px RBBits, // kSwapRB parameters
          Px AShift, Px ABits>             // kForceOpaque parameters
skvx::Vec<N, Px> apply_ops_packed(skvx::Vec<N, Px> pixel) {
    static_assert(!(Ops & (kPadAlpha | kDropAlpha)), "Packed formats do not drop/pad alpha");
    static_assert(RShift < BShift, "Shifts are not in the correct significance order");
    static_assert(RShift + RBBits <= BShift, "Low swap channel overflows");
    static_assert(BShift + RBBits <= sizeof(Px)*8, "Hi swap channel overflows");
    static_assert(AShift + ABits <= sizeof(Px)*8, "Alpha channel overflows");

    if constexpr (Ops & kSwapRB) {
        static constexpr Px kChannelMask = (1 << RBBits) - 1;
        static constexpr Px kRMask = kChannelMask << RShift;
        static constexpr Px kBMask = kChannelMask << BShift;

        pixel = (pixel & ~(kRMask | kBMask)) |            // Preserve non-RB bits
                ((pixel & kRMask) << (BShift - RShift)) | // Move red (lo) to blue (hi) position
                ((pixel & kBMask) >> (BShift - RShift));  // Move blue (hi) to red (lo) position
    }

    if constexpr (Ops & kForceOpaque) {
        static constexpr Px kAMask = ((1 << ABits) - 1) << AShift;
        // The unsigned normalized value for 1.0 is all 1 bits set in the packed field so we can
        // just OR the mask into place.
        SkASSERT(ABits > 0);

        pixel = pixel | kAMask;
    }

    return pixel;
}

// NOTE: This takes no parameters for alpha because none of the formats that use this support
// kDrop/PadAlpha. Since the channels are packed into Px, both source and dst must have the same bpp
// `zeroValue` specifies the bit value for the entire pixel representing {0, 0, 0, 0}.
template <typename Px, Px RShift, Px BShift, Px RBBits, Px AShift, Px ABits>
XferRowFn xfer_rows_packed(uint8_t ops, Px zeroValue=0) {
    static constexpr int kBpp = sizeof(Px);
    static constexpr int N = 16 / sizeof(Px); // Fit to 128-bit/16-byte SIMD

    static_assert(N == 1 || N == 2 || N == 4 || N == 8);
    using PxVec = skvx::Vec<N, Px>;

#define RETURN_XFER_ROW_FN(ops) \
        case ops: \
            if constexpr ((ops) & kIgnoreSrc) { \
                return create_ignore_src_row_fn<PxVec>(N, PxVec(zeroValue), kBpp, \
                        apply_ops_packed<Px, N, ops & ~kIgnoreSrc, RShift, BShift, RBBits, AShift, ABits>); \
            } else { \
                return create_xfer_row_fn<PxVec>(N, kBpp, kBpp, \
                        apply_ops_packed<Px, N, ops, RShift, BShift, RBBits, AShift, ABits>); \
            }

    switch (ops) {
        // The expected combination of ExtendedFormatXferOps
        RETURN_XFER_ROW_FN(kForceOpaque)
        RETURN_XFER_ROW_FN(kForceOpaque | kSwapRB)
        RETURN_XFER_ROW_FN(kForceOpaque | kIgnoreSrc)
        RETURN_XFER_ROW_FN(kSwapRB)
        RETURN_XFER_ROW_FN(kIgnoreSrc)
        default:
            SK_ABORT("Unsupported ExtendedFormatXferOps combination: %u", ops);
    }
#undef RETURN_XFER_ROW_FN
}

// N represents the number of pixels being processed; C represents the number of channels within
// each pixel. Some Ops are only valid if C is 4.
template <typename Cx, int C, int N, uint8_t Ops, Cx OpaqueAlpha>
skvx::Vec<C*N, Cx> apply_ops_by_channel(skvx::Vec<C*N, Cx> pixel) {
    static_assert(sizeof(pixel) == 16);

    if constexpr (Ops & kPadAlpha && C == 4) {
        // If we are padding alpha, we are moving from a 3-channel source data (loaded in the first
        // N*3 values of pixel) to a 4-channel value. 3*N+1 holds undefined data after loading the
        // pixel, which we can set to the opaque alpha value. Then we shuffle the slots to spread
        // out each pixel's R,G, and B values and insert copies of the opaque alpha value.
        static constexpr int kA = 3*N;
        pixel[kA] = OpaqueAlpha;

        if constexpr (N == 4) {
            pixel = skvx::shuffle<0,1,2,kA, 3,4,5,kA, 6,7,8,kA, 9,10,11,kA>(pixel);
        } else if constexpr (N == 2) {
            pixel = skvx::shuffle<0,1,2,kA, 3,4,5,kA>(pixel);
        } else {
            // no shuffling needed for N=1, since pixel == shuffle<0,1,2,A>(pixel)
            static_assert(N == 1);
        }
    }

    // Ops that assume 4 channels can be applied between kPadAlpha and kDropAlpha w/o worrying about
    // how to handle the 3-channel formats.
    if constexpr (Ops & kSwapRB && C == 4) {
        // For all 3 and 4 channel formats, it is assumed that R and B are in channels 0 and 2.
        if constexpr (N == 4) {
            pixel = skvx::shuffle<2,1,0,3, 6,5,4,7, 10,9,8,11, 14,13,12,15>(pixel);
        } else if constexpr (N == 2) {
            pixel = skvx::shuffle<2,1,0,3, 6,5,4,7>(pixel);
        } else {
            static_assert(N == 1);
            pixel = skvx::shuffle<2,1,0,3>(pixel);
        }
    }

    if constexpr (Ops & kForceOpaque && C != 2) {
        // Between kPadAlpha and kDropAlpha, it is assumed that alpha is always channel 3. Unlike
        // packed formats, the bit representation for OpaqueAlpha can't be assumed.
        if constexpr (C == 1) {
            // Single-channel formats will assign the value to the whole vector
            pixel = OpaqueAlpha;
        } else {
            static_assert(C == 4);
            if constexpr (N == 4) {
                pixel[3] = pixel[7] = pixel[11] = pixel[15] = OpaqueAlpha;
            } else if constexpr (N == 2) {
                pixel[3] = pixel[7] = OpaqueAlpha;
            } else {
                static_assert(N == 1);
                pixel[3] = OpaqueAlpha;
            }
        }
    }

    if constexpr (Ops & kDropAlpha && C == 4) {
        // If we are dropping alpha, we need to shuffle the R,G, and B values of the 4-channel
        // source data into the first N*3 slots of the returned pixel. The remaining N slots will
        // be ignored by the final memcpy.
        if constexpr (N == 4) {
            pixel = skvx::shuffle<0,1,2, 4,5,6, 8,9,10, 12,13,14, 14,14,14,14>(pixel);
        } else if constexpr (N == 2) {
            pixel = skvx::shuffle<0,1,2, 4,5,6, 6,6>(pixel);
        } else {
            // no shuffling needed for N=1, since pixel == shuffle<0,1,2,_>(pixel)
            static_assert(N == 1);
        }
    }

    return pixel;
}

// NOTE: This takes no parameters for SwapRB or DropAlpha because for all formats that use this,
// SwapRB involves swapping channel 0 and channel 2, and dropping alpha removes channel 3. This
// can be parameterized via template parameters to be able to push into the skvx::shuffle calls
// if needed in the future.
template <typename Cx, int C, Cx OpaqueAlpha>
XferRowFn xfer_rows_by_channel(uint8_t ops, Cx zeroValue = 0) {
    static_assert(C == 1 || C == 2 || C == 3 || C == 4);
    static constexpr int CPow2 = SkNextPow2(C);
    static constexpr int N = 16 / (CPow2 * sizeof(Cx)); // Fit to 128-bit/16-byte SIMD

    static_assert(N == 1 || N == 2 || N == 4 || N == 8 || N == 16);
    using PxVec = skvx::Vec<N*CPow2, Cx>;

    int srcBpp = C * sizeof(Cx);
    int dstBpp = C * sizeof(Cx);
    if (ops & kDropAlpha) {
        // Going from 4-channel src to the 3-channel format
        SkASSERT(C == 3);
        srcBpp = CPow2 * sizeof(Cx);
    } else if (ops & kPadAlpha) {
        // Going from the 3-channel format to 4-channel dst
        SkASSERT(C == 3);
        dstBpp = CPow2 * sizeof(Cx);
    }

    if (ops & kIgnoreSrc) {
        srcBpp = 0;
    }

    // template <typename Cx, int C, int N, uint8_t Ops, Cx OpaqueAlpha>
#define RETURN_XFER_ROW_FN(ops) \
        case ops: \
            if constexpr ((ops) & kIgnoreSrc) { \
                return create_ignore_src_row_fn<PxVec>(N, PxVec(zeroValue), dstBpp, \
                        apply_ops_by_channel<Cx, CPow2, N, ops & ~kIgnoreSrc, OpaqueAlpha>); \
            } else { \
                return create_xfer_row_fn<PxVec>(N, srcBpp, dstBpp, \
                        apply_ops_by_channel<Cx, CPow2, N, ops, OpaqueAlpha>); \
            }

    switch (ops) {
        // The expected combination of ExtendedFormatXferOps
        RETURN_XFER_ROW_FN(kDropAlpha)
        RETURN_XFER_ROW_FN(kDropAlpha | kSwapRB)
        RETURN_XFER_ROW_FN(kDropAlpha | kIgnoreSrc)
        RETURN_XFER_ROW_FN(kPadAlpha)
        RETURN_XFER_ROW_FN(kPadAlpha | kSwapRB)
        RETURN_XFER_ROW_FN(kPadAlpha | kIgnoreSrc)
        RETURN_XFER_ROW_FN(kForceOpaque)
        RETURN_XFER_ROW_FN(kForceOpaque | kSwapRB)
        RETURN_XFER_ROW_FN(kForceOpaque | kIgnoreSrc)
        RETURN_XFER_ROW_FN(kSwapRB)
        RETURN_XFER_ROW_FN(kIgnoreSrc)
        default:
            SK_ABORT("Unsupported ExtendedFormatXferOps combination: %u", ops);
    }
#undef RETURN_XFERFN_CASE
}

XferRowFn get_xfer_row_fn(TextureFormat format, uint8_t ops) {
    static constexpr uint32_t kFloatBits1 = 0x3f800000; // SkFloat2Bits isn't constexpr
    SkASSERT(kFloatBits1 == SkFloat2Bits(1.f));
    SkASSERT(ops); // For now, assume we only call into this if we have work to do.

    static constexpr uint32_t kXRZero = 0x180; // 10 bits encoding 0.0 for the _XR formats

    switch (format) {
        // Packed formats operate on a primitive that holds the entire pixel value
        case TF::kB5_G6_R5:
        case TF::kR5_G6_B5:
            return xfer_rows_packed<uint16_t, /*RShift=*/0, /*BShift=*/11, /*RBBits=*/5, /*AShift=*/0, /*ABits=*/0>(ops);

        case TF::kABGR4:
        case TF::kARGB4:
            return xfer_rows_packed<uint16_t, /*RShift=*/4, /*BShift=*/12, /*RBBits=*/4, /*AShift=*/0, /*ABits=*/4>(ops);

        case TF::kRGB10_A2:
        case TF::kBGR10_A2:
        case TF::kBGR10_XR:
            return xfer_rows_packed<uint32_t, /*RShift=*/0, /*BShift=*/20, /*RBBits=*/10, /*AShift=*/30, /*ABits=*/2>(ops,
                        format == TF::kBGR10_XR ? (((kXRZero << 20) | (kXRZero << 10) | kXRZero)) : 0);

        // The remaining formats can be operated on with each channel as a primitive
                case TF::kR8:
        case TF::kA8:
            return xfer_rows_by_channel<uint8_t, /*C=*/1, /*OpaqueAlpha=*/0xFF>(ops);

        case TF::kR16:
            return xfer_rows_by_channel<uint16_t, /*C=*/1, /*OpaqueAlpha=*/0xFFFF>(ops);

        case TF::kR16F:
            return xfer_rows_by_channel<uint16_t, /*C=*/1, /*OpaqueAlpha=*/SK_Half1>(ops);

        case TF::kR32F:
            return xfer_rows_by_channel<uint32_t, /*C=*/1, /*OpaqueAlpha=*/kFloatBits1>(ops);

        case TF::kRG8:
            return xfer_rows_by_channel<uint8_t, /*C=*/2, /*OpaqueAlpha=*/0>(ops);

        case TF::kRG16:
        case TF::kRG16F:
            return xfer_rows_by_channel<uint16_t, /*C=*/2, /*OpaqueAlpha=*/0>(ops);

        case TF::kRG32F:
            return xfer_rows_by_channel<uint32_t, /*C=*/2, /*OpaqueAlpha=*/0>(ops);

        case TF::kRGB8_sRGB:
        case TF::kRGB8:
        case TF::kBGR8:
            return xfer_rows_by_channel<uint8_t, /*C=*/3, 0xFF>(ops);

        case TF::kRGB16:
            return xfer_rows_by_channel<uint16_t, /*C=*/3, 0xFFFF>(ops);

        case TF::kRGB16F:
            return xfer_rows_by_channel<uint16_t, /*C=*/3, SK_Half1>(ops);

        case TF::kRGB32F:
            return xfer_rows_by_channel<uint32_t, /*C=*/3, kFloatBits1>(ops);

        case TF::kRGBA8:
        case TF::kRGBA8_sRGB:
        case TF::kBGRA8:
        case TF::kBGRA8_sRGB:
            return xfer_rows_by_channel<uint8_t, /*C=*/4, 0xFF>(ops);

        case TF::kRGBA10x6:
            // Each channel is the 10 real bits, with the least significant 6 padding bits.
            return xfer_rows_by_channel<uint16_t, /*C=*/4, 0xFFC0>(ops);

        case TF::kBGRA10x6_XR:
            return xfer_rows_by_channel<uint16_t, /*C=*/4, 0xFFC0>(ops, kXRZero << 6);
            // FIXME how did we not trip up a kIgnoreSrc issue with 10x6_XR?
            //
            // It would require us to do what configuration? src = alpha-only color to
            // BGRA10x_6_XR (but that needs the alpha value), so we need to not consume the
            // value -> read swizzle is RGB1.
            //
            // But we probably aren't seeing that as an exclusive channel replacement since
            // we don't treat the RGB1 swizzle as producing rgbx.

        case TF::kRGBA16:
            return xfer_rows_by_channel<uint16_t, /*C=*/4, 0xFFFF>(ops);

        case TF::kRGBA16F:
            return xfer_rows_by_channel<uint16_t, /*C=*/4, SK_Half1>(ops);

        case TF::kRGBA32F:
            return xfer_rows_by_channel<uint32_t, /*C=*/4, kFloatBits1>(ops);

        default:
            // Remaining cases are compressed, multiplanar, or non-color so shouldn't be reached.
            // If the first assert trips, we missed a valid transfer format in the cases above.
            // If we hit the unreachable, we missed rejecting the transfer sooner.
            SkASSERT(TextureFormatColorTypeInfo(format).second & FormatXferOp::kDisabled);
            SkUNREACHABLE;
    }
}

bool bgra_to_rgba(SkColorType* ct) {
    switch (*ct) {
        case kBGRA_8888_SkColorType:    *ct = kRGBA_8888_SkColorType;    return true;
        case kBGRA_1010102_SkColorType: *ct = kRGBA_1010102_SkColorType; return true;
        case kBGR_101010x_SkColorType:  *ct = kRGB_101010x_SkColorType;  return true;
        // NOTE: For now there's no RGBA version of the _XR color types, so they can't be swapped
        case kBGRA_10101010_XR_SkColorType:
        case kBGR_101010x_XR_SkColorType: [[fallthrough]];
        default:
            // No colortype consolidation possible by switching to a kSwapRB op instead.
            return false;
    }
}

bool rgbx_to_rgba(SkColorType* ct) {
    switch (*ct) {
        case kRGB_888x_SkColorType: *ct = kRGBA_8888_SkColorType; return true;
        case kRGB_101010x_SkColorType: *ct = kRGBA_1010102_SkColorType; return true;
        case kRGB_F16F16F16x_SkColorType: *ct = kRGBA_F16_SkColorType; return true;
        case kBGR_101010x_SkColorType: *ct = kBGRA_1010102_SkColorType; return true;
        default:
            // No colortype consolidation possible by switching to a kForceOpaque op instead.
            return false;
    }
}

// An RPModifier for RPOps::Make() that handles calculating bt709 luminance. The API is that it
// must have an explicit bool operator, and an apply(SkRasterPipeline*) function.
struct BT709Luminance {
    bool fEnabled;

    explicit operator bool() const { return fEnabled; }
    void apply(SkRasterPipeline* rp) const {
        if (fEnabled) {
            rp->append(SkRasterPipelineOp::bt709_luminance_or_luma_to_alpha);
        } // else no op needed
    }
};

bool account_for_luminance(SkColorType srcCT,
                           bool hasColorSpaceTransform,
                           SkColorType* dstCT) {
    if (*dstCT == kGray_8_SkColorType) {
        if (srcCT != kGray_8_SkColorType || hasColorSpaceTransform) {
            // Luminance must be calculated by raster pipeline, but we'll have to apply it before
            // any srcToDst swizzle, so pull it out to its own RPModifier. The op stores the value
            // in the alpha channel, so that needs to be dstCT that SkRasterPipeline sees.
            *dstCT = kAlpha_8_SkColorType;
            return true;
        } // else leave it as gray->gray
    } else if (srcCT == kGray_8_SkColorType && *dstCT == kR8_unorm_SkColorType && !hasColorSpaceTransform) {
        // The source gray value can just be copied to the red channel, so adjust this to be
        // gray->"gray" for a no-op copy
        *dstCT = kGray_8_SkColorType;
        return false;
    } // else trivially no extra luminance needs to be calculated, leave dstCT as-is
    return false;
}

uint32_t gray_adjusted_channels(SkColorType ct) {
    return ct == kGray_8_SkColorType ? kRGB_SkColorChannelFlags : SkColorTypeChannelFlags(ct);
}

uint32_t swizzle_adjusted_channels(uint32_t channels, Swizzle swizzle) {
    auto removeChannelIfConstant = [&](SkColorChannelFlag flag, int index) {
        if (swizzle[index] == '0' || swizzle[index] == '1') {
            channels &= ~flag;
        }
    };
    removeChannelIfConstant(kRed_SkColorChannelFlag, 0);
    removeChannelIfConstant(kGreen_SkColorChannelFlag, 1);
    removeChannelIfConstant(kBlue_SkColorChannelFlag, 2);
    removeChannelIfConstant(kAlpha_SkColorChannelFlag, 3);
    return channels;
}

template <bool TextureIsDst>
std::pair</*ops=*/uint8_t, /*computeLuminance=*/bool> optimize_transfer(
        SkColorType* cpuCT,
        SkColorType* texBaseCT,
        Swizzle* texReadSwizzle,
        SkColorSpaceXformSteps* csSteps,
        SkEnumBitMask<FormatXferOp> xferOps) {
    // aliases for color types that are based on the transfer direction
    SkColorType* srcCT = TextureIsDst ? cpuCT     : texBaseCT;
    SkColorType* dstCT = TextureIsDst ? texBaseCT : cpuCT;

    uint8_t finalOps = 0;

    // First, adjust texBaseCT to match colortype semantics that can be inferred from swizzle
    {
        SkColorType adjustedBase = *texBaseCT;
        SkDebugf("adjusting texture ct for %d %s\n", (int) adjustedBase, texReadSwizzle->asString().c_str());
        if (*texReadSwizzle == Swizzle("000r") || *texReadSwizzle == Swizzle("0001")) {
            // Red -> Alpha so shift the texture's "base" colortype to be the matching alpha type
            switch(*texBaseCT) {
                case kR8_unorm_SkColorType:  adjustedBase = kAlpha_8_SkColorType;   break;
                case kR16_unorm_SkColorType: adjustedBase = kA16_unorm_SkColorType; break;
                case kR16_float_SkColorType: adjustedBase = kA16_float_SkColorType; break;
                default: break; // Go through regular RP + swizzle flow
            }
        } else if ((*texReadSwizzle == Swizzle("rrra") || (*texReadSwizzle == Swizzle("rrr1"))) &&
                   adjustedBase == kR8_unorm_SkColorType) {
            // Red -> Gray so shift to kGray, which either ensures RP will generate the gray values
            // from a non-gray input, or will be detected as a no-op when transferring to/from
            // existing gray.
            adjustedBase = kGray_8_SkColorType;
        }
        if (adjustedBase != *texBaseCT) {
            SkDebugf("adjusted to %d\n", adjustedBase);
            *texBaseCT = adjustedBase;
            *texReadSwizzle = Swizzle::RGBA();
        }
    }

    {
        if (*srcCT == kRGBA_F16Norm_SkColorType) {
            *srcCT = kRGBA_F16_SkColorType;
        }
        if (*dstCT == kRGBA_F16Norm_SkColorType) {
            *dstCT = kRGBA_F16_SkColorType;
        }
    }

    // Second, if the channels are disjoint between the src and the dst, then the dst values can
    // ignore the source, at which point we reset everything else to the identity and flag what
    // value is required by the dst (1 if the dst is alpha, 0 if it's RGB/gray).
    {
        uint32_t cpuChannels = gray_adjusted_channels(*cpuCT);
        uint32_t gpuChannels = swizzle_adjusted_channels(gray_adjusted_channels(*texBaseCT), *texReadSwizzle);

        if (!(gpuChannels & cpuChannels)) {
            finalOps |= kIgnoreSrc;
            *texReadSwizzle = Swizzle::RGBA();
            *srcCT = *dstCT;
            *csSteps = SkColorSpaceXformSteps{};
            if (SkColorTypeIsAlphaOnly(*dstCT)) {
                SkDebugf("force-opaque ignore-src\n");
                finalOps |= kForceOpaque;
            }
            SkDebugf("adding ignore-src: %d\n", (int) *dstCT);
            return {finalOps, /*computeLuminance=*/false};
        }
    }

    // Second, handle masking any unknown alpha bits if the dst doesn't already mask them.
    {
        bool cpuMasksAlpha = rgbx_to_rgba(cpuCT);
        bool gpuMasksAlpha = rgbx_to_rgba(texBaseCT) || (*texReadSwizzle)[3] == '1';
        bool srcHasJunkAlpha = TextureIsDst ? cpuMasksAlpha : gpuMasksAlpha;
        bool dstStoresAlpha =
                (SkColorTypeChannelFlags(*dstCT) & kAlpha_SkColorChannelFlag) &&
                !(TextureIsDst ? gpuMasksAlpha : cpuMasksAlpha);

        if (srcHasJunkAlpha) {
            csSteps->fFlags.premul = false;
            csSteps->fFlags.unpremul = false;
        }

        if (srcHasJunkAlpha && dstStoresAlpha) {
            // Must override the junk alpha bits with opaque values during the transfer.
            finalOps |= kForceOpaque;
        }
        // Reset any opacity forcing in the swizzle since it's either in finalOps or can be ignored
        // FIXME is this better to push to a unified place for just removing constants from the swizzle?
        if ((*texReadSwizzle)[3] == '1') {
            *texReadSwizzle = Swizzle((*texReadSwizzle)[0], (*texReadSwizzle)[1], (*texReadSwizzle)[2], 'a');
        }
    }

    // Third, lift gray/luminance calculation out of colortype so that its placement in the
    // raster pipeline ops list can be controlled (vs. attached to a store).
    bool computeLuminance = account_for_luminance(*srcCT, SkToBool(*csSteps), dstCT);

    // Fourth, consolidate red/blue swaps present in colortype, swizzle, and xferOps into just ops
    {
        // Any swaps from the texture's base color type, swizzle, and xfer ops can always be
        // combined since those operations are grouped together, regardless of `TextureIsDst`.
        int numRBSwaps = 0;
        if ((*texReadSwizzle)[0] == 'b' && (*texReadSwizzle)[2] == 'r') {
            // Remove the swap in the swizzle (moving towards a no-op swizzle).
            *texReadSwizzle = Swizzle::Concat(*texReadSwizzle, Swizzle::BGRA());
            numRBSwaps++;
            SkDebugf("swap for swizzle\n");
        }
        if (bgra_to_rgba(texBaseCT)) {
            SkDebugf("swap for tex CT\n");
            numRBSwaps++;
        }
        if (xferOps & FormatXferOp::kSwapRB) {
            SkDebugf("swap for xfer op\n");
            numRBSwaps++;
        }

        // If there is not any RGB-dependent calculation between CPU and GPU data, we can also
        // consolidate the swap from cpu color type.
        // FIXME only do it if it matches texBaseCT?
        if (!computeLuminance && !SkToBool(*csSteps) && bgra_to_rgba(cpuCT)) {
            SkDebugf("swap for cpu type\n");
            numRBSwaps++;
        }

        // An even number of swaps is a no-op; an odd number of swaps is the same as one swap. The
        // swap can be skipped if the source data is known to have the same values in R and B, or
        // if R and B will be discarded.
        // FIXME account for colorspace/computeLuminance and directionality rules
        bool srcHasRBData = *srcCT != kGray_8_SkColorType && !SkColorTypeIsAlphaOnly(*srcCT);
        bool dstKeepsRBData = !SkColorTypeIsAlphaOnly(*dstCT) || computeLuminance;
        if ((numRBSwaps & 1) && srcHasRBData && dstKeepsRBData) {
            SkDebugf("adding final swap op for count = %d\n", numRBSwaps);
            finalOps |= kSwapRB;
        } else {
            SkDebugf("not adding swap for count = %d, has rb data %d, keeps rb data %d\n",
                    numRBSwaps, srcHasRBData, dstKeepsRBData);
        }
    }

    // Finally, apply the remaining FormatXferOps that aren't compatible with color types.
    if (xferOps & FormatXferOp::kDropAlpha) {
        if constexpr (TextureIsDst) {
            finalOps |= kDropAlpha; // CPU->GPU drops the alpha channel
        } else {
            finalOps |= kPadAlpha; // GPU->CPU pads the alpha channel back
        }
        // Remove kForceOpaque as it's redundant with kDropAlpha/kPadAlpha
        finalOps &= ~kForceOpaque;
    }
    // if (xferOps & FormatXferOp::kSwapRB) {
    //     finalOps |= kSwapRB;
    // }

    return {finalOps, computeLuminance};
}

} // anonymous namespace

std::optional<TextureFormatXferFn> TextureFormatXferFn::MakeCpuToGpu(
        SkColorType srcCT,
        const SkColorSpaceXformSteps& csSteps,
        TextureFormat dstFormat,
        Swizzle dstReadSwizzle) {
    auto [baseCT, xferOps] = TextureFormatColorTypeInfo(dstFormat);
    if (xferOps & FormatXferOp::kDisabled) {
        return std::nullopt;
    }

    SkColorSpaceXformSteps copied = csSteps;
    auto [postOps, computeLuminance] = optimize_transfer<true>(&srcCT, &baseCT, &dstReadSwizzle, &copied, xferOps);

    // The CPU -> GPU transform is:
    //  SkRP{load(srcCT) ->
    //       csSteps? ->
    //       luminance? -> NOTE: luminance must be computed *before* the texture's swizzle
    //       srcToDst(dstReadSwizzle^-1)? ->
    //       store(baseCT)}? ->
    //  postOps(baseCT->TF)?
    auto rp = RPOps::Make(srcCT, baseCT, &postOps, // rpModifiers =>
                          copied, BT709Luminance{computeLuminance}, dstReadSwizzle.invert());
    return TextureFormatXferFn(dstFormat, /*preOps=*/0, std::move(rp), postOps, computeLuminance);
}

std::optional<TextureFormatXferFn> TextureFormatXferFn::MakeGpuToCpu(
        TextureFormat srcFormat,
        Swizzle srcReadSwizzle,
        const SkColorSpaceXformSteps& csSteps,
        SkColorType dstCT) {
    auto [baseCT, xferOps] = TextureFormatColorTypeInfo(srcFormat);
    if (xferOps & FormatXferOp::kDisabled) {
        return std::nullopt;
    }


    SkColorSpaceXformSteps copied = csSteps;
    auto [preOps, computeLuminance] = optimize_transfer<false>(&dstCT, &baseCT, &srcReadSwizzle, &copied, xferOps);

    if (preOps & kIgnoreSrc) {
        // The source is the texture format, which is what defines how ops are implemented, so
        // pick a new format that matches the `dstCT`; actual GPU support doesn't matter, this will
        // just ensure the CPU data is initialized correctly.
        srcFormat = PreferredTextureFormats(dstCT)[0];
    }

    // The GPU -> CPU transform is:
    //  preOps(TF->baseCT)? ->
    //  SkRP{load(baseCT) ->
    //       srcToDst(srcReadSwizzle)? ->
    //       csSteps? ->
    //       luminance? ->
    //       store(dstCT)}?
    auto rp = RPOps::Make(baseCT, dstCT, &preOps, // rpModifiers =>
                          srcReadSwizzle, copied, BT709Luminance{computeLuminance});
    return TextureFormatXferFn(srcFormat, preOps, std::move(rp), /*postOps=*/0, computeLuminance);
}

std::optional<TextureFormatXferFn> TextureFormatXferFn::MakeIdentity(TextureFormat format) {
    auto [baseCT, xferOps] = TextureFormatColorTypeInfo(format);
    if (xferOps & FormatXferOp::kDisabled) {
        if (TextureFormatCompressionType(format) == SkTextureCompressionType::kNone) {
            return std::nullopt;
        } // else allow compressed formats through for identity conversion uploads
    }

    return TextureFormatXferFn(format, /*preOps=*/0, /*rp=*/nullptr, /*postOps=*/0, false);
}

template <typename... RPModifiers>
sk_sp<TextureFormatXferFn::RPOps> TextureFormatXferFn::RPOps::Make(
        SkColorType srcColorType,
        SkColorType dstColorType,
        uint8_t* xferOps,
        RPModifiers... rpModifiers) {
    const bool modifiersNeedRP = (SkToBool(rpModifiers) || ...);

    if (srcColorType == dstColorType && !modifiersNeedRP) {
        return nullptr; // Identity conversion
    }

    // Luminance has to be calculated before the texture's swizzle so it's pulled out manually
    // and put in the right place in `rpModifiers`, so we don't want to encounter them where
    // the appendStore() also computes luminance.
    SkASSERT(dstColorType != kGray_8_SkColorType);

    sk_sp<RPOps> ops{new RPOps(/*srcBpp=*/SkColorTypeBytesPerPixel(srcColorType),
                               /*dstBpp=*/SkColorTypeBytesPerPixel(dstColorType))};

    // NOTE: The src and dst memory contexts are not modified here, they just provide stable
    // pointers for the appended ops to reference, and will be patched during run().
    ops->fRP.appendLoad(srcColorType, &ops->fSrcCtx);

    auto apply = [target = ops.get()] <typename RPModifier> (RPModifier modifier,
                                                             uint8_t* xferOps) {
        const RPModifier* stableModifier = &modifier;
        if constexpr (std::is_same_v<RPModifier, Swizzle>) {
            // Swizzle does not require any persisted context, but we can also fold kSwapRB into
            // the swizzle. This is done here so that the kSwapRB op is grouped with the swizzle
            // modifier (given the order of modifiers passed into Make).
            if (*xferOps & kSwapRB) {
                modifier = Swizzle::Concat(modifier, Swizzle::BGRA());
                *xferOps &= ~kSwapRB;
            }
            if (*xferOps & kForceOpaque) {
                modifier = Swizzle::Concat(modifier, Swizzle::RGB1());
                *xferOps &= ~kForceOpaque;
            }
        } else if constexpr (!std::is_same_v<RPModifier, BT709Luminance>) {
            // We must create a copy of rpModifiers[i] in the arena, because its apply() function
            // may reference parts of itself as the context's passed to the appended ops.
            stableModifier = target->fArena.make<RPModifier>(modifier);
        } // else BT709Luminance does not require any persisted context

        stableModifier->apply(&target->fRP);
    };

    (apply(rpModifiers, xferOps), ...);

    ops->fRP.appendStore(dstColorType, &ops->fDstCtx);

    ops->fSrcCT = srcColorType;
    ops->fDstCT = dstColorType;
    return ops;
}

bool TextureFormatXferFn::RPOps::setStrides(size_t srcRowBytes,
                                            size_t dstRowBytes,
                                            uint8_t otherOps) {
    // SkRasterPipeline operates in pixel units for its strides, so we should only be relying on
    // RP's built-in row stride handling if the data is aligned to the pixel size.
    if (srcRowBytes % fSrcBpp == 0 && dstRowBytes % fDstBpp == 0 && otherOps == 0) {
        fSrcCtx.stride = SkTo<int>(srcRowBytes / fSrcBpp);
        fDstCtx.stride = SkTo<int>(dstRowBytes / fDstBpp);
        return true;
    } else {
        // Control loop must proceed row by row, so stride can be 0
        fSrcCtx.stride = 0;
        fDstCtx.stride = 0;
        return false;
    }
}

// TODO(michaelludwig): This is a WIP implementation, it is not focusing on performance yet.
void TextureFormatXferFn::run(int width, int height,
                              const void* src, size_t srcRowBytes,
                              void* dst, size_t dstRowBytes) const {
    SkASSERT(width >= 1 && height >= 1);

    int rpInvokeCount;
    SkAutoMalloc tempRowStorage; // empty if no FormatXferOps have to be applied

    if (fRP && fRP->setStrides(srcRowBytes, dstRowBytes, fPreOps | fPostOps)) {
        // Conversions occur entirely within SkRasterPipeline, so we can configure the
        // MemoryCtx's to process the whole 2D image.
        rpInvokeCount = 1;
    } else {
        // Conversions will have to occur row-by-row. The SkRP row function will patch the
        // memory contexts to each row's offset address so we can leave stride as 0.
        SkASSERT(!fRP || (fRP->fSrcCtx.stride == 0 && fRP->fDstCtx.stride == 0));
        rpInvokeCount = height;
        height = 1;
    }

    skia_private::STArray<2, XferRowFn> rowFns; // At most 2 actions per row
    if (fPreOps) {
        // `src` is definitively the texture
        if (fRP) {
            // We need a temporary buffer equal to srcBpp*width to hold the output of the preOps
            // that is used as the source of data for SkRasterPipeline (executed per row).
            tempRowStorage.reset(fRP->fSrcBpp * width);
        }
        rowFns.push_back(get_xfer_row_fn(fFormat, fPreOps));
    }

    if (fRP) {
        rowFns.push_back([&](const char* src, char* dst, int width) {
            // NOTE: When height != 1, this invocation actually processes the entire image.
            // Otherwise we assume src and dst have been offset by y so we update the MemoryCtx's
            // pixel addresses.
            fRP->fSrcCtx.pixels = const_cast<char*>(src); // This won't be written to
            fRP->fDstCtx.pixels = dst;
            fRP->fRP.run(0, 0, width, height);
        });
    }

    if (fPostOps) {
        // `dst` is definitively the texture
        if (fRP) {
            // We need a temporary buffer equal to dstBpp*width to hold the output of the
            // SkRasterPipeline conversion that is used as the input to postOps (executed per row).
            tempRowStorage.reset(fRP->fDstBpp * width);
        }
        rowFns.push_back(get_xfer_row_fn(fFormat, fPostOps));
    }

    if (rowFns.empty()) {
        // Identity conversion function still needs to move the data
        const int bpp = TextureFormatBytesPerBlock(fFormat);
        rowFns.push_back([bpp](const char* src, char* dst, int width) {
            memcpy(dst, src, bpp * width);
        });
    }

    for (int y = 0; y < rpInvokeCount; ++y) {
        // Always start by processing `src`
        const char* input = static_cast<const char*>(src) + y * srcRowBytes;
        for (int i = 0; i < rowFns.size(); ++i) {
            // And either output to the temporary row or the final `dst`
            char* target = i == rowFns.size() - 1 ? (static_cast<char*>(dst) + y * dstRowBytes)
                                                  : static_cast<char*>(tempRowStorage.get());
            rowFns[i](input, target, width);
            // If there's more than one rowFn, switch to using the temporary row as input
            input = target;
        }
    }
}

} // namespace skgpu::graphite

// TODO(b/390473370): TextureUploadWriter isn't very useful with TextureFormatXferFn's
// capabilities and the writer model doesn't work well with the more explicit placement of dst
// data that has to happen. UploadBufferManager should just provide a span and then we can call
// run() directly and delete TextureUploadWriter entirely.
#include "src/gpu/BufferWriter.h"

namespace skgpu {

void TextureUploadWriter::convert(size_t offset, int width, int height,
                                  const void* src, size_t srcRowBytes,
                                  const graphite::TextureFormatXferFn& dst, size_t dstRowBytes) {
    this->validate(offset + dstRowBytes * height);
    void* dstPtr = SkTAddOffset<void>(fPtr, offset);
    dst.run(width, height, src, srcRowBytes, dstPtr, dstRowBytes);
}

} // namespace skgpu
