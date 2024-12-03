/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/encode/SkPngEncoderBase.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSpan.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkMSAN.h"
#include "src/base/SkSafeMath.h"

namespace {

SkEncodedInfo makeInfo(const SkImageInfo& srcInfo,
                       SkEncodedInfo::Color color,
                       int bitsPerComponent) {
    SkEncodedInfo::Alpha alpha =
            color == SkEncodedInfo::kGray_Color || color == SkEncodedInfo::kRGB_Color
                    ? SkEncodedInfo::kOpaque_Alpha
                    : SkEncodedInfo::kUnpremul_Alpha;

    return SkEncodedInfo::Make(srcInfo.width(), srcInfo.height(), color, alpha, bitsPerComponent);
}

SkEncodedInfo makeGray8Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kGray_Color, 8);
}

SkEncodedInfo makeGrayAlpha8Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kGrayAlpha_Color, 8);
}

SkEncodedInfo makeRgb8Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kRGB_Color, 8);
}

SkEncodedInfo makeRgba8Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kRGBA_Color, 8);
}

SkEncodedInfo makeRgb16Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kRGB_Color, 16);
}

SkEncodedInfo makeRgba16Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kRGBA_Color, 16);
}

size_t computeDstRowSize(const SkEncodedInfo& dstInfo) {
    // `static_cast<size_t>`(dstInfo.bitsPerPixel())` uses trustworthy, bounded
    // data as input - no need to use `SkSafeMath` for this part.
    SkASSERT(dstInfo.bitsPerComponent() == 8 || dstInfo.bitsPerComponent() == 16);
    SkASSERT(dstInfo.bitsPerPixel() <= (16 * 4));
    size_t bitsPerPixel = static_cast<size_t>(dstInfo.bitsPerPixel());
    SkASSERT((bitsPerPixel % 8) == 0);
    size_t bytesPerPixel = bitsPerPixel / 8;

    SkSafeMath safe;
    size_t dstRowSize = safe.mul(safe.castTo<size_t>(dstInfo.width()), bytesPerPixel);
    return safe.ok() ? dstRowSize : 0;
}

}  // namespace

// static
std::optional<std::pair<SkEncodedInfo, transform_scanline_proc>> SkPngEncoderBase::getTargetInfo(
        const SkImageInfo& srcInfo) {
    switch (srcInfo.colorType()) {
        case kUnknown_SkColorType:
            return std::nullopt;

        // TODO: I don't think this can just use kRGBA's procs.
        // kPremul is especially tricky here, since it's presumably TF⁻¹(rgb * a),
        // so to get at unpremul rgb we'd need to undo the transfer function first.
        case kSRGBA_8888_SkColorType:
            return std::nullopt;

        case kRGBA_8888_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                    return std::make_pair(makeRgb8Info(srcInfo), transform_scanline_RGBX);
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba8Info(srcInfo), transform_scanline_memcpy);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba8Info(srcInfo), transform_scanline_rgbA);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kBGRA_8888_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                    return std::make_pair(makeRgb8Info(srcInfo), transform_scanline_BGRX);
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba8Info(srcInfo), transform_scanline_BGRA);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba8Info(srcInfo), transform_scanline_bgrA);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kRGB_565_SkColorType:
            SkASSERT(srcInfo.isOpaque());
            return std::make_pair(makeRgb8Info(srcInfo), transform_scanline_565);
        case kRGB_888x_SkColorType:
            SkASSERT(srcInfo.isOpaque());
            return std::make_pair(makeRgb8Info(srcInfo), transform_scanline_RGBX);
        case kARGB_4444_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                    return std::make_pair(makeRgb8Info(srcInfo), transform_scanline_444);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba8Info(srcInfo), transform_scanline_4444);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kGray_8_SkColorType:
            SkASSERT(srcInfo.isOpaque());
            return std::make_pair(makeGray8Info(srcInfo), transform_scanline_memcpy);

        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo), transform_scanline_F16);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo), transform_scanline_F16_premul);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kRGB_F16F16F16x_SkColorType:
            SkASSERT(srcInfo.isOpaque());
            return std::make_pair(makeRgb16Info(srcInfo), transform_scanline_F16F16F16x);
        case kRGBA_F32_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo), transform_scanline_F32);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo), transform_scanline_F32_premul);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kRGBA_1010102_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo), transform_scanline_1010102);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo),
                                          transform_scanline_1010102_premul);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kBGRA_1010102_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo), transform_scanline_bgra_1010102);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo),
                                          transform_scanline_bgra_1010102_premul);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kRGB_101010x_SkColorType:
            return std::make_pair(makeRgb16Info(srcInfo), transform_scanline_101010x);
        case kBGR_101010x_SkColorType:
            return std::make_pair(makeRgb16Info(srcInfo), transform_scanline_bgr_101010x);
        case kBGR_101010x_XR_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                    return std::make_pair(makeRgb16Info(srcInfo),
                                          transform_scanline_bgr_101010x_xr);
                default:
                    SkDEBUGFAIL("unsupported color type");
                    return std::nullopt;
            }
        case kBGRA_10101010_XR_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo),
                                          transform_scanline_bgra_10101010_xr);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo),
                                          transform_scanline_bgra_10101010_xr_premul);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kAlpha_8_SkColorType:
            return std::make_pair(makeGrayAlpha8Info(srcInfo), transform_scanline_A8_to_GrayAlpha);
        case kR8G8_unorm_SkColorType:
        case kR16G16_unorm_SkColorType:
        case kR16G16_float_SkColorType:
        case kA16_unorm_SkColorType:
        case kA16_float_SkColorType:
        case kR16G16B16A16_unorm_SkColorType:
        case kR8_unorm_SkColorType:
        case kRGBA_10x6_SkColorType:
            return std::nullopt;
    }
    SkDEBUGFAIL("unsupported color type");
    return std::nullopt;
}

SkPngEncoderBase::SkPngEncoderBase(const SkEncodedInfo& dstInfo,
                                   const SkPixmap& src,
                                   transform_scanline_proc transformProc)
        : SkEncoder(src, computeDstRowSize(dstInfo))
        , fTransformProc(transformProc)
        , fDstRowSize(computeDstRowSize(dstInfo)) {
    SkASSERT(fTransformProc);
}

bool SkPngEncoderBase::onEncodeRows(int numRows) {
    // Postponed check from the constructor, in case `SkSafeMath` has failed.
    if (fDstRowSize == 0) {
        return false;
    }

    // https://www.w3.org/TR/png-3/#11IHDR says that "zero is an invalid value"
    // for width and height.
    if (fSrc.width() == 0 || fSrc.height() == 0) {
        return false;
    }

    if (numRows < 0) {
        return false;
    }

    while (numRows > 0) {
        if (fCurrRow == fSrc.height()) {
            return false;
        }

        const void* srcRow = fSrc.addr(0, fCurrRow);
        sk_msan_assert_initialized(srcRow,
                                   (const uint8_t*)srcRow + (fSrc.width() << fSrc.shiftPerPixel()));

        fTransformProc((char*)fStorage.get(),
                       (const char*)srcRow,
                       fSrc.width(),
                       SkColorTypeBytesPerPixel(fSrc.colorType()));

        SkSpan<uint8_t> rowToEncode(fStorage.get(), fDstRowSize);
        if (!this->onEncodeRow(rowToEncode)) {
            return false;
        }

        fCurrRow++;
        numRows--;
    }

    if (fCurrRow == fSrc.height() && !fFinishedEncoding) {
        fFinishedEncoding = true;
        return this->onFinishEncoding();
    }

    return true;
}
