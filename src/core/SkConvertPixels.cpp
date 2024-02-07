/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkConvertPixels.h"

#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSize.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkHalf.h"
#include "src/base/SkRectMemcpy.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkSwizzlePriv.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>

#include <arm_neon.h>

static const int px = 179;
static const int py = 226;

static bool rect_memcpy(const SkImageInfo& dstInfo,       void* dstPixels, size_t dstRB,
                        const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB,
                        const SkColorSpaceXformSteps& steps) {
    // We can copy the pixels when no color type, alpha type, or color space changes.
    if (dstInfo.colorType() != srcInfo.colorType()) {
        return false;
    }
    if (dstInfo.colorType() != kAlpha_8_SkColorType
            && steps.flags.mask() != 0b00000) {
        return false;
    }

    SkRectMemcpy(dstPixels, dstRB,
                 srcPixels, srcRB, dstInfo.minRowBytes(), dstInfo.height());
    return true;
}

#define SK_USE_FAST_UNPREMUL_324099025

static bool swizzle_or_premul(const SkImageInfo& dstInfo,       void* dstPixels, size_t dstRB,
                              const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB,
                              const SkColorSpaceXformSteps& steps) {
    auto is_8888 = [](SkColorType ct) {
        return ct == kRGBA_8888_SkColorType || ct == kBGRA_8888_SkColorType;
    };
    if (!is_8888(dstInfo.colorType()) ||
        !is_8888(srcInfo.colorType()) ||
        steps.flags.linearize         ||
        steps.flags.gamut_transform   ||
        #if !defined(SK_USE_FAST_UNPREMUL_324099025)
        steps.flags.unpremul  ||
        #endif
        steps.flags.encode) {
        return false;
    }

    const bool swapRB = dstInfo.colorType() != srcInfo.colorType();

    void (*fn)(uint32_t*, const uint32_t*, int) = nullptr;

    if (steps.flags.premul) {
        fn = swapRB ? SkOpts::RGBA_to_bgrA
                    : SkOpts::RGBA_to_rgbA;
    } else if (steps.flags.unpremul) {
        fn = swapRB ? SkOpts::rgbA_to_BGRA
                    : SkOpts::rgbA_to_RGBA;
    } else {
        // If we're not swizzling, we ought to have used rect_memcpy().
        SkASSERT(swapRB);
        fn = SkOpts::RGBA_to_BGRA;
    }


    const uint32_t* srcPixelPtr = SkTAddOffset<const uint32_t>(srcPixels, srcRB * py);
    uint32_t srcPixel = srcPixelPtr[px];
    uint32_t* dstPixelPtr = SkTAddOffset<uint32_t>(dstPixels, dstRB * py);
    for (int y = 0; y < dstInfo.height(); y++) {
        fn((uint32_t*)dstPixels, (const uint32_t*)srcPixels, dstInfo.width());
        dstPixels = SkTAddOffset<void>(dstPixels, dstRB);
        srcPixels = SkTAddOffset<const void>(srcPixels, srcRB);
    }
    uint32_t dstPixel = dstPixelPtr[px];
    SkDebugf("Bulk - src: %x, dst: %x\n", srcPixel, dstPixel);
    return true;
}

static bool convert_to_alpha8(const SkImageInfo& dstInfo,       void* vdst, size_t dstRB,
                              const SkImageInfo& srcInfo, const void*  src, size_t srcRB,
                              const SkColorSpaceXformSteps&) {
    if (dstInfo.colorType() != kAlpha_8_SkColorType) {
        return false;
    }
    auto dst = (uint8_t*)vdst;

    switch (srcInfo.colorType()) {
        case kUnknown_SkColorType:
        case kAlpha_8_SkColorType: {
            // Unknown should never happen.
            // Alpha8 should have been handled by rect_memcpy().
            SkASSERT(false);
            return false;
        }

        case kA16_unorm_SkColorType: {
            auto src16 = (const uint16_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = src16[x] >> 8;
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                src16 = SkTAddOffset<const uint16_t>(src16, srcRB);
            }
            return true;
        }

        case kGray_8_SkColorType:
        case kRGB_565_SkColorType:
        case kR8G8_unorm_SkColorType:
        case kR16G16_unorm_SkColorType:
        case kR16G16_float_SkColorType:
        case kRGB_888x_SkColorType:
        case kRGB_101010x_SkColorType:
        case kBGR_101010x_SkColorType:
        case kBGR_101010x_XR_SkColorType:
        case kR8_unorm_SkColorType: {
            for (int y = 0; y < srcInfo.height(); ++y) {
               memset(dst, 0xFF, srcInfo.width());
               dst = SkTAddOffset<uint8_t>(dst, dstRB);
            }
            return true;
        }

        case kARGB_4444_SkColorType: {
            auto src16 = (const uint16_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = SkPacked4444ToA32(src16[x]);
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                src16 = SkTAddOffset<const uint16_t>(src16, srcRB);
            }
            return true;
        }

        case kBGRA_8888_SkColorType:
        case kRGBA_8888_SkColorType:
        case kSRGBA_8888_SkColorType: {
            auto src32 = (const uint32_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = src32[x] >> 24;
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                src32 = SkTAddOffset<const uint32_t>(src32, srcRB);
            }
            return true;
        }

        case kRGBA_1010102_SkColorType:
        case kBGRA_1010102_SkColorType: {
            auto src32 = (const uint32_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = (src32[x] >> 30) * 0x55;
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                src32 = SkTAddOffset<const uint32_t>(src32, srcRB);
            }
            return true;
        }

        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType: {
            auto src64 = (const uint64_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = (uint8_t) (255.0f * SkHalfToFloat(src64[x] >> 48));
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                src64 = SkTAddOffset<const uint64_t>(src64, srcRB);
            }
            return true;
        }

        case kRGBA_F32_SkColorType: {
            auto rgba = (const float*)src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = (uint8_t)(255.0f * rgba[4*x+3]);
                }
                dst  = SkTAddOffset<uint8_t>(dst, dstRB);
                rgba = SkTAddOffset<const float>(rgba, srcRB);
            }
            return true;
        }

        case kA16_float_SkColorType: {
            auto srcF16 = (const uint16_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = (uint8_t) (255.0f * SkHalfToFloat(srcF16[x]));
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                srcF16 = SkTAddOffset<const uint16_t>(srcF16, srcRB);
            }
            return true;
        }

        case kRGBA_10x6_SkColorType:
        case kR16G16B16A16_unorm_SkColorType: {
            auto src64 = (const uint64_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = (src64[x] >> 48) >> 8;
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                src64 = SkTAddOffset<const uint64_t>(src64, srcRB);
            }
            return true;
        }
    }
    return false;
}

// Default: Use the pipeline.
static void convert_with_pipeline(const SkImageInfo& dstInfo, void* dstRow, int dstStride,
                                  const SkImageInfo& srcInfo, const void* srcRow, int srcStride,
                                  const SkColorSpaceXformSteps& steps) {
    SkRasterPipeline_MemoryCtx src = { const_cast<void*>(srcRow), srcStride },
                               dst = {                   dstRow,  dstStride };

    SkRasterPipeline_<256> pipeline;
    pipeline.appendLoad(srcInfo.colorType(), &src);
    steps.apply(&pipeline);
    pipeline.appendStore(dstInfo.colorType(), &dst);
    pipeline.run(0,0, srcInfo.width(), srcInfo.height());
}

bool SkConvertPixels(const SkImageInfo& dstInfo,       void* dstPixels, size_t dstRB,
                     const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB) {
    SkASSERT(dstInfo.dimensions() == srcInfo.dimensions());
    SkASSERT(SkImageInfoValidConversion(dstInfo, srcInfo));

    int srcStride = (int)(srcRB / srcInfo.bytesPerPixel());
    int dstStride = (int)(dstRB / dstInfo.bytesPerPixel());
    if ((size_t)srcStride * srcInfo.bytesPerPixel() != srcRB ||
        (size_t)dstStride * dstInfo.bytesPerPixel() != dstRB) {
        return false;
    }

    SkColorSpaceXformSteps steps{srcInfo.colorSpace(), srcInfo.alphaType(),
                                 dstInfo.colorSpace(), dstInfo.alphaType()};



    std::unique_ptr<uint32_t[]> dstBulk{new uint32_t[dstInfo.width() * dstInfo.height()]};
    std::unique_ptr<uint32_t[]> dstRP{new uint32_t[dstInfo.width() * dstInfo.height()]};

    auto srcPixel = [srcPixels, srcRB](int x, int y) {
        const uint32_t* srcRow = SkTAddOffset<const uint32_t>(srcPixels, srcRB * y);
        return srcRow[x];
    };

    auto dstPixel = [dstPixels, dstRB](int x, int y) {
        const uint32_t* dstRow = SkTAddOffset<uint32_t>(dstPixels, dstRB * y);
        return dstRow[x];
    };

    for (auto fn : {rect_memcpy, swizzle_or_premul, convert_to_alpha8}) {
        if (fn(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB, steps)) {
            for (int y = 0; y < dstInfo.height(); ++y) {
                for (int x = 0; x < dstInfo.width(); ++x) {
                    dstBulk[y * dstInfo.width() + x] = dstPixel(x, y);
                }
            }
            // return true;
        }
    }

    convert_with_pipeline(dstInfo, dstPixels, dstStride, srcInfo, srcPixels, srcStride, steps);
    for (int y = 0; y < dstInfo.height(); ++y) {
        for (int x = 0; x < dstInfo.width(); ++x) {
            dstRP[y * dstInfo.width() + x] = dstPixel(x, y);
        }
    }

    auto roundToEven = [](float n) {
        return vcvtns_u32_f32(n);
    };

    auto expectDenormalized = [](float a, float s) {
        float reciprocalA = 255.0f / a;
        return std::floor(s * reciprocalA + 0.5);
    };

    auto expectNormalized = [roundToEven](float a, float s) {
        float aN = a * (1.0f / 255.0f);
        float sN = s * (1.0f / 255.0f);
        float reciprocalA = 1.0f / aN;
        return roundToEven((sN * reciprocalA) * 255.0f);
    };

    auto analyzeComponent = [expectDenormalized](float a, float s, float bulk, float rp) {
        float expected = expectDenormalized(a, s);
        if (expected != bulk) {
            SkDebugf("bad bulk - expected: %g actual: %g\n", expected, bulk);
        }

        if (expected != rp) {
            SkDebugf("bad rp - expected: %g actual: %g\n", expected, rp);
        }
    };

    auto analyzeDifference = [analyzeComponent](uint32_t src, uint32_t bulk, uint32_t rp) {
        uint32_t a = SkGetPackedA32(src);
        analyzeComponent(a, SkGetPackedR32(src), SkGetPackedR32(bulk), SkGetPackedR32(rp));
        analyzeComponent(a, SkGetPackedG32(src), SkGetPackedG32(bulk), SkGetPackedG32(rp));
        analyzeComponent(a, SkGetPackedB32(src), SkGetPackedB32(bulk), SkGetPackedB32(rp));


    };

    for (int y = 0; y < dstInfo.height(); ++y) {
        for (int x = 0; x < dstInfo.width(); ++x) {
            if (dstRP[y * dstInfo.width() + x] != dstBulk[y * dstInfo.width() + x]) {
                uint32_t srcP = srcPixel(x, y);
                uint32_t dstBulkP = dstBulk[y * dstInfo.width() + x];
                uint32_t dstRPP = dstRP[y * dstInfo.width() + x];
                analyzeDifference(srcP, dstBulkP, dstRPP);
            }
        }
    }

    return true;
}
