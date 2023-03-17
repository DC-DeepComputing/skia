/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/DitherUtils.h"

#ifndef SK_IGNORE_GPU_DITHER

#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"

namespace skgpu {

float DitherRangeForConfig(SkColorType dstColorType) {
    // We use 1 / (2^bitdepth-1) as the range since each channel can hold 2^bitdepth values
    switch (dstColorType) {
        // 4 bit
        case kARGB_4444_SkColorType: //GrColorType::kABGR_4444:
//            case GrColorType::kARGB_4444:
//            case GrColorType::kBGRA_4444:
            return 1 / 15.f;
            // 6 bit
        case kRGB_565_SkColorType: // GrColorType::kBGR_565:
            return 1 / 63.f;
        // 8 bit
        case kUnknown_SkColorType: // GrColorType::kUnknown:
        case kAlpha_8_SkColorType: // GrColorType::kAlpha_8:
//            case GrColorType::kAlpha_8xxx:
        case kGray_8_SkColorType: // GrColorType::kGray_8:
//            case GrColorType::kGrayAlpha_88:
//            case GrColorType::kGray_8xxx:
        case kR8_unorm_SkColorType: // GrColorType::kR_8:
//            case GrColorType::kR_8xxx:
        case kR8G8_unorm_SkColorType: // GrColorType::kRG_88:
//            case GrColorType::kRGB_888:
        case kRGB_888x_SkColorType: // GrColorType::kRGB_888x:
        case kRGBA_8888_SkColorType: // GrColorType::kRGBA_8888:
        case kSRGBA_8888_SkColorType: // GrColorType::kRGBA_8888_SRGB:
        case kBGRA_8888_SkColorType: // GrColorType::kBGRA_8888:
            return 1 / 255.f;
        // 10 bit
        case kRGBA_1010102_SkColorType: // GrColorType::kRGBA_1010102:
        case kBGRA_1010102_SkColorType: // GrColorType::kBGRA_1010102:
        case kRGB_101010x_SkColorType:
        case kBGR_101010x_SkColorType:
        case kBGR_101010x_XR_SkColorType:
            return 1 / 1023.f;
        // 16 bit
        case kA16_unorm_SkColorType: // GrColorType::kAlpha_16:
//            case GrColorType::kR_16:
        case kR16G16_unorm_SkColorType: // GrColorType::kRG_1616:
        case kR16G16B16A16_unorm_SkColorType: //GrColorType::kRGBA_16161616:
            return 1 / 32767.f;
        // Half
        case kA16_float_SkColorType: // GrColorType::kAlpha_F16:
//            case GrColorType::kGray_F16:
//            case GrColorType::kR_F16:
        case kR16G16_float_SkColorType: // GrColorType::kRG_F16:
        case kRGBA_F16_SkColorType: // GrColorType::kRGBA_F16:
        case kRGBA_F16Norm_SkColorType: // GrColorType::kRGBA_F16_Clamped:
        // Float
//            case GrColorType::kAlpha_F32xxx:
        case kRGBA_F32_SkColorType: // GrColorType::kRGBA_F32:
            return 0.f; // no dithering
    }
    SkUNREACHABLE;
}

 SkBitmap MakeDitherLUT() {
    static constexpr struct DitherTable {
        constexpr DitherTable() : data() {
            for (int x = 0; x < 8; ++x) {
                for (int y = 0; y < 8; ++y) {
                    // The computation of 'm' and 'value' is lifted from CPU backend.
                    unsigned int m = (y & 1) << 5 | (x & 1) << 4 |
                                     (y & 2) << 2 | (x & 2) << 1 |
                                     (y & 4) >> 1 | (x & 4) >> 2;
                    float value = float(m) * 1.0 / 64.0 - 63.0 / 128.0;
                    // Bias by 0.5 to be in 0..1, mul by 255 and round to nearest int to make byte.
                    data[y * 8 + x] = (uint8_t)((value + 0.5) * 255.f + 0.5f);
                }
            }
        }
        uint8_t data[64];
    } gTable;

    SkBitmap bmp;
    bmp.setInfo(SkImageInfo::MakeA8(8, 8));
    bmp.setPixels(const_cast<uint8_t*>(gTable.data));
    bmp.setImmutable();
    return bmp;
}

}  // namespace skgpu

#endif // SK_IGNORE_GPU_DITHER
