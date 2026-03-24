/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/TextureFormatXferFn.h"

#include "include/core/SkColorType.h"
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkFloatBits.h"
#include "src/base/SkHalf.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkVx.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/gpu/graphite/Log.h"

#include <functional>

namespace skgpu::graphite {

namespace {

using TF = TextureFormat;

// This is intentionally not a class enum and is distinct from FormatXferOps for two reasons:
// 1. We can't use SkEnumBitmask inside template parameters for the row bit manipulation functions.
// 2. It's convenient to split the FormatXferOps into specific ops based on their conversion
//    direction that doesn't need to be exposed in the public API.
enum ExtendedFormatXferOp : uint8_t {
    kDropAlpha = 0x1,
    kPadAlpha  = 0x2,
};


using XferRowFn = std::function<void(const char* src, char* dst, int width)>;

template <typename PxVec>
using ApplyPixelFn = PxVec(*)(PxVec);

template <typename PxVec>
XferRowFn create_xfer_row_fn(int n, int srcBpp, int dstBpp, ApplyPixelFn<PxVec> fn) {
    // Px should be sufficient to hold N src and dst pixels, and should match at least one
    SkASSERT(sizeof(PxVec) >= (size_t)srcBpp*n && sizeof(PxVec) >= (size_t)dstBpp*n);
    SkASSERT(sizeof(PxVec) == (size_t)srcBpp*n || sizeof(PxVec) == (size_t)dstBpp*n);

    return [n, srcBpp, dstBpp, fn](const char* src, char* dst, int width) {
        const int srcBppN = n * srcBpp;
        const int dstBppN = n * dstBpp;

        PxVec pixel;
        while (width >= n) {
            memcpy(&pixel, src, srcBppN);
            pixel = fn(pixel);
            memcpy(dst, &pixel, dstBppN);

            width -= n;
            src += srcBppN;
            dst += dstBppN;
        }

        if (width > 0) {
            // Process tail that is less than a full vector
            SkASSERT(width < n);
            memcpy(&pixel, src, width * srcBpp);
            pixel = fn(pixel);
            memcpy(dst, &pixel, width * dstBpp);
        }
    };
}

// N represents the number of pixels being processed; the currently supported Ops are only valid for
// a channel count of 3 (upgraded to 4 effectively) or exactly 4.
template <typename Cx, int N, uint8_t Ops, Cx OpaqueAlpha>
skvx::Vec<4*N, Cx> apply_ops_by_channel(skvx::Vec<4*N, Cx> pixel) {
    static_assert(N == 1 || N == 2 || N == 4);
    if constexpr (Ops & kPadAlpha) {
        // If we are padding alpha, we are moving from a 3-channel source data (loaded in the first
        // N*3 values of pixel) to a 4-channel value. 3*N=1 holds undefined data after loading the
        // pixel, which we can set to the opaque alpha value. Then we shuffle the slots to spread
        // out each pixel's R,G, and B values and insert copies of the opaque alpha value.
        static constexpr int A = 3*N;
        pixel[A] = OpaqueAlpha;

        if constexpr (N == 4) {
            pixel = skvx::shuffle<0,1,2,A, 3,4,5,A, 6,7,8,A, 9,10,11,A>(pixel);
        } else if constexpr (N == 2) {
            pixel = skvx::shuffle<0,1,2,A, 3,4,5,A>(pixel);
        } // else no shuffling needed for N=1, since pixel == shuffle<0,1,2,A>(pixel)
    }

    // TODO(michaelludwig): Add other extend ops here

    if constexpr (Ops & kDropAlpha) {
        // If we are dropping alpha, we need to shuffle the R,G, and B values of the 4-channel
        // source data into the first N*3 slots of the returned pixel. The remaining N slots will
        // be ignored by the final memcpy.
        static constexpr int _ = 3*N;
        if constexpr (N == 4) {
            pixel = skvx::shuffle<0,1,2, 4,5,6, 8,9,10, 12,13,14, _,_,_,_>(pixel);
        } else if constexpr (N == 2) {
            pixel = skvx::shuffle<0,1,2, 4,5,6, _,_>(pixel);
        } // else no shuffling needed for N=1, since pixel == shuffle<0,1,2,_>(pixel)
    }

    return pixel;
}

// NOTE: This takes no parameters for SwapRB or DropAlpha because for all formats that use this,
// SwapRB involves swapping channel 0 and channel 2, and dropping alpha removes channel 3. This
// can be parameterized via template parameters to be able to push into the skvx::shuffle calls
// if needed in the future.
template <typename Cx, Cx OpaqueAlpha>
XferRowFn xfer_rows_by_channel(uint8_t ops) {
    static constexpr int C = 3;
    static constexpr int CPow2 = SkNextPow2(C);
    static constexpr int N = 16 / (CPow2 * sizeof(Cx)); // Fit to 128-bit/16-byte SIMD

    static_assert(N == 1 || N == 2 || N == 4);
    using PxVec = skvx::Vec<N*CPow2, Cx>;

    int srcBpp = C * sizeof(Cx);
    int dstBpp = C * sizeof(Cx);

    ApplyPixelFn<PxVec> applyPixels = nullptr;
    if (ops & kDropAlpha) {
        // Going from 4-channel src to the 3-channel format
        srcBpp = CPow2 * sizeof(Cx);
        applyPixels = apply_ops_by_channel<Cx, N, kDropAlpha, OpaqueAlpha>;
    } else if (ops & kPadAlpha) {
        // Going from the 3-channel format to 4-channel dst
        dstBpp = CPow2 * sizeof(Cx);
        applyPixels = apply_ops_by_channel<Cx, N, kPadAlpha, OpaqueAlpha>;
    } else {
        SKGPU_LOG_F("Identity transfer should have been caught earlier");
    }

    return create_xfer_row_fn(N, srcBpp, dstBpp, applyPixels);
}

XferRowFn get_xfer_row_fn(TextureFormat format, uint8_t ops) {
    static constexpr uint32_t kFloat1 = 0x3f800000; // SkFloat2Bits isn't constexpr
    SkASSERT(kFloat1 == SkFloat2Bits(1.f));
    SkASSERT(ops); // For now, assume we only call into this if we have work to do.

    switch (format) {
        case TF::kR8:
        case TF::kA8:
        case TF::kR16:
        case TF::kR16F:
        case TF::kRG8:
        case TF::kRG16:
        case TF::kRG16F:
        case TF::kRG32F:
            // 1 and 2 channel formats cannot be combined with colortypes in such a way to create
            // format conversions, so we should never reach here
            SKGPU_LOG_F("Unexpected ops (%u) requested for format %s",
                        ops, TextureFormatName(format));
            break;

        // Packed formats operate on a primitive that holds the entire pixel value
        case TF::kB5_G6_R5:
        case TF::kR5_G6_B5:
        case TF::kABGR4:
        case TF::kARGB4:
        case TF::kRGB10_A2:
        case TF::kBGR10_A2:
        case TF::kBGR10_XR:
            // TODO(michaelludwig): These formats could do r/b swaps and forcing-opaque, but
            // that isn't implemented yet.
            SKGPU_LOG_F("Unsupported texture format %s", TextureFormatName(format));
            break;

        // The remaining formats can be operated on with each channel is a primitive
        case TF::kRGB8_sRGB:
        case TF::kRGB8:
        case TF::kBGR8:
            return xfer_rows_by_channel<uint8_t, 0xFF>(ops);

        case TF::kRGB16:
            return xfer_rows_by_channel<uint16_t, 0xFFFF>(ops);

        case TF::kRGB16F:
            return xfer_rows_by_channel<uint16_t, SK_Half1>(ops);

        case TF::kRGB32F:
            return xfer_rows_by_channel<uint32_t, kFloat1>(ops);

        case TF::kRGBA8:
        case TF::kRGBA8_sRGB:
        case TF::kBGRA8:
        case TF::kBGRA8_sRGB:
        case TF::kRGBA16:
        case TF::kRGBA16F:
        case TF::kRGBA10x6:
        case TF::kBGRA10x6_XR:
        case TF::kRGBA32F:
            // TODO(michaelludwig): These formats could do r/b swaps and forcing-opaque, but
            // that isn't implemented yet.
            SKGPU_LOG_F("Unsupported texture format %s", TextureFormatName(format));
            break;

        default:
            // Remaining cases are compressed, multiplanar, or non-color so shouldn't be reached.
            // If the first assert trips, we missed a valid transfer format in the cases above.
            // If we hit the unreachable, we missed rejecting the transfer sooner.
            SkASSERT(TextureFormatColorTypeInfo(format).second & FormatXferOp::kDisabled);
            SkUNREACHABLE;
    }
}

} // anonymous namespace

std::optional<TextureFormatXferFn> TextureFormatXferFn::MakeCpuToGpu(SkColorType srcCT,
                                                                     TextureFormat dstFormat,
                                                                     Swizzle dstReadSwizzle) {
    auto [baseCT, xferOps] = TextureFormatColorTypeInfo(dstFormat);
    if (xferOps & FormatXferOp::kDisabled) {
        return std::nullopt;
    }

    uint8_t postOps = 0;
    Swizzle srcToDst = dstReadSwizzle.invert();
    if (xferOps & FormatXferOp::kSwapRB) {
        srcToDst = Swizzle::Concat(srcToDst, Swizzle::BGRA());
    }
    if (xferOps & FormatXferOp::kDropAlpha) {
        // On CPU->GPU conversion, FormatXferOp::kDropAlpha actually drops the alpha bits
        postOps |= kDropAlpha;
    }
    auto rp = RPOps::Make(srcCT, srcToDst, baseCT);
    return TextureFormatXferFn(dstFormat, /*preOps=*/0, std::move(rp), postOps);
}

std::optional<TextureFormatXferFn> TextureFormatXferFn::MakeGpuToCpu(TextureFormat srcFormat,
                                                                     Swizzle srcReadSwizzle,
                                                                     SkColorType dstCT) {
    auto [baseCT, xferOps] = TextureFormatColorTypeInfo(srcFormat);
    if (xferOps & FormatXferOp::kDisabled) {
        return std::nullopt;
    }

    uint8_t preOps = 0;
    Swizzle srcToDst = srcReadSwizzle;
    if (xferOps & FormatXferOp::kSwapRB) {
        srcToDst = Swizzle::Concat(srcToDst, Swizzle::BGRA());
    }
    if (xferOps & FormatXferOp::kDropAlpha) {
        // On GPU->CPU conversion, FormatXferOp::kDropAlpha must pad alpha bits back
        preOps |= kPadAlpha;
    }
    auto rp = RPOps::Make(baseCT, srcToDst, dstCT);
    return TextureFormatXferFn(srcFormat, preOps, std::move(rp), /*postOps=*/0);
}

std::unique_ptr<TextureFormatXferFn::RPOps> TextureFormatXferFn::RPOps::Make(
        SkColorType srcColorType,
        Swizzle srcToDst,
        SkColorType dstColorType) {
    if (srcColorType == dstColorType && srcToDst == Swizzle::RGBA()) {
        return nullptr; // Identity conversion
    }
    auto ops = std::make_unique<RPOps>();

    ops->fSrcBpp = SkColorTypeBytesPerPixel(srcColorType);
    ops->fDstBpp = SkColorTypeBytesPerPixel(dstColorType);

    // NOTE: The src and dst memory contexts are not modified here, they just provide stable
    // pointers for the appended ops to reference, and will be patched during run().
    ops->fRP.appendLoad(srcColorType, &ops->fSrcCtx);
    srcToDst.apply(&ops->fRP);
    ops->fRP.appendStore(dstColorType, &ops->fDstCtx);
    return ops;
}

// TODO(michaelludwig): This is a simple stub implementation (we aren't planning to use the
// SkConvertPixels code currently used by UploadTask and asyncReads). It's purpose is to validate
// that the generated test cases make sense before we introduce a new TextureFormatXferFn utility.
// As such, 3-channel formats are not supported.
void TextureFormatXferFn::run(int width, int height,
                              const void* src, size_t srcRowBytes,
                              void* dst, size_t dstRowBytes) const {
    SkASSERT(width >= 1 && height >= 1);

    int rpInvokeCount;
    SkAutoMalloc tempRowStorage; // empty if no FormatXferOps have to be applied

    const bool rpOnly = fRP &&                             // requires raster pipeline
                        !(fPreOps || fPostOps) &&          // and no custom conversion ops
                        srcRowBytes % fRP->fSrcBpp == 0 && // and srcRowBytes is pixel aligned
                        dstRowBytes % fRP->fDstBpp == 0;   // and dstRowBytes is pixel aligned

    if (rpOnly) {
        // Conversions occur entirely within SkRasterPipeline, so we can configure the
        // MemoryCtx's to process the whole 2D image. The "strides" specified in the SkRP contexts
        // are in pixel units so we have to convert (which is why we only use the bulk 2D
        // invocation when the rowbytes are multiples of the bytes-per-pixel).
        rpInvokeCount = 1;
        fRP->fSrcCtx.stride = SkTo<int>(srcRowBytes / fRP->fSrcBpp);
        fRP->fDstCtx.stride = SkTo<int>(dstRowBytes / fRP->fDstBpp);
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
