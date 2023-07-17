/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkRescaleAndReadPixelsUtils.h"

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/core/SkYUVAInfo.h"

#include <cmath>
#include <cstddef>
#include <memory>
#include <utility>

namespace SkRescaleAndReadPixelsUtils {

void SynchronousRescaleAndReadPixels(SkBitmap bmp,
                                     const SkImageInfo& resultInfo,
                                     const SkIRect& srcRect,
                                     SkImage::RescaleGamma rescaleGamma,
                                     SkImage::RescaleMode rescaleMode,
                                     SkImage::RGBAReadPixelsCallback callback,
                                     SkImage::ReadPixelsContext context) {
    int srcW = srcRect.width();
    int srcH = srcRect.height();

    float sx = (float)resultInfo.width() / srcW;
    float sy = (float)resultInfo.height() / srcH;
    // How many bilerp/bicubic steps to do in X and Y. + means upscaling, - means downscaling.
    int stepsX;
    int stepsY;
    if (rescaleMode != SkImage::RescaleMode::kNearest) {
        stepsX = static_cast<int>((sx > 1.f) ? std::ceil(std::log2f(sx))
                                             : std::floor(std::log2f(sx)));
        stepsY = static_cast<int>((sy > 1.f) ? std::ceil(std::log2f(sy))
                                             : std::floor(std::log2f(sy)));
    } else {
        stepsX = sx != 1.f;
        stepsY = sy != 1.f;
    }

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    if (stepsX < 0 || stepsY < 0) {
        // Don't trigger MIP generation. We don't currently have a way to trigger bicubic for
        // downscaling draws.

        // TODO: should we trigger cubic now that we can?
        if (rescaleMode != SkImage::RescaleMode::kNearest) {
            rescaleMode = SkImage::RescaleMode::kRepeatedLinear;
        }
    }

    auto rescaling_to_sampling = [](SkImage::RescaleMode rescaleMode) {
        SkSamplingOptions sampling;
        if (rescaleMode == SkImage::RescaleMode::kRepeatedLinear) {
            sampling = SkSamplingOptions(SkFilterMode::kLinear);
        } else if (rescaleMode == SkImage::RescaleMode::kRepeatedCubic) {
            sampling = SkSamplingOptions({1.0f/3, 1.0f/3});
        }
        return sampling;
    };
    SkSamplingOptions sampling = rescaling_to_sampling(rescaleMode);

    sk_sp<SkSurface> tempSurf;
    sk_sp<SkImage> srcImage;
    int srcX = srcRect.fLeft;
    int srcY = srcRect.fTop;
    SkCanvas::SrcRectConstraint constraint = SkCanvas::kStrict_SrcRectConstraint;
    // Assume we should ignore the rescale linear request if the surface has no color space since
    // it's unclear how we'd linearize from an unknown color space.
    if (rescaleGamma == SkSurface::RescaleGamma::kLinear && bmp.info().colorSpace() &&
        !bmp.info().colorSpace()->gammaIsLinear()) {
        auto cs = bmp.info().colorSpace()->makeLinearGamma();
        // Promote to F16 color type to preserve precision.
        auto ii = SkImageInfo::Make(srcW, srcH, kRGBA_F16_SkColorType, bmp.info().alphaType(),
                                    std::move(cs));
        auto linearSurf = SkSurfaces::Raster(ii);
        if (!linearSurf) {
            callback(context, nullptr);
            return;
        }
        linearSurf->getCanvas()->drawImage(bmp.asImage().get(), -srcX, -srcY, sampling, &paint);
        tempSurf = std::move(linearSurf);
        srcImage = tempSurf->makeImageSnapshot();
        srcX = 0;
        srcY = 0;
        constraint = SkCanvas::kFast_SrcRectConstraint;
    } else {
        // MakeFromBitmap would trigger a copy if bmp is mutable.
        srcImage = SkImages::RasterFromPixmap(bmp.pixmap(), nullptr, nullptr);
    }
    while (stepsX || stepsY) {
        int nextW = resultInfo.width();
        int nextH = resultInfo.height();
        if (stepsX < 0) {
            nextW = resultInfo.width() << (-stepsX - 1);
            stepsX++;
        } else if (stepsX != 0) {
            if (stepsX > 1) {
                nextW = srcW * 2;
            }
            --stepsX;
        }
        if (stepsY < 0) {
            nextH = resultInfo.height() << (-stepsY - 1);
            stepsY++;
        } else if (stepsY != 0) {
            if (stepsY > 1) {
                nextH = srcH * 2;
            }
            --stepsY;
        }
        auto ii = srcImage->imageInfo().makeWH(nextW, nextH);
        if (!stepsX && !stepsY) {
            // Might as well fold conversion to final info in the last step.
            ii = resultInfo;
        }
        auto next = SkSurfaces::Raster(ii);
        if (!next) {
            callback(context, nullptr);
            return;
        }
        next->getCanvas()->drawImageRect(
                srcImage.get(), SkRect::Make(SkIRect::MakeXYWH(srcX, srcY, srcW, srcH)),
                SkRect::MakeIWH(nextW, nextH), sampling, &paint, constraint);
        tempSurf = std::move(next);
        srcImage = tempSurf->makeImageSnapshot();
        srcX = srcY = 0;
        srcW = nextW;
        srcH = nextH;
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }

    SkBitmap readBmp;
    readBmp.allocPixels(resultInfo);
    if (srcImage->readPixels(nullptr, readBmp.pixmap(), srcX, srcY)) {
        class Result : public SkImage::RGBAAsyncReadPixelsResult {
        public:
            Result(SkBitmap bmp) : fBmp(bmp) {}

            const SkPixmap& pixmap() const override { return fBmp.pixmap(); }

        private:
            SkBitmap fBmp;
        };
        callback(context, std::make_unique<Result>(std::move(readBmp)));
    } else {
        callback(context, nullptr);
    }
}

bool IsYUVAInfoValid(const SkYUVAInfo& info, const SkYUVAPixmapInfo::DataType dataType) {
    if (info.planeConfig() != SkYUVAInfo::PlaneConfig::kY_U_V &&
        info.planeConfig() != SkYUVAInfo::PlaneConfig::kY_U_V_A) {
        return false;
    }

    if (info.subsampling() != SkYUVAInfo::Subsampling::k420) {
        return false;
    }

    if (info.origin() != kTopLeft_SkEncodedOrigin) {
        return false;
    }

    if (info.width() & 0b1 || info.height() & 0b1) {
        return false;
    }

    if (dataType != SkYUVAPixmapInfo::DataType::kUnorm8) {
        return false;
    }

    return true;
}

void LegacyRGBACallbackWrapper(
        SkImage::ReadPixelsContext rgbaContext,
        std::unique_ptr<const SkImage::RGBAAsyncReadPixelsResult> rgbaResult) {
    class LegacyResultWrapper : public SkImage::AsyncReadResult {
    public:
        LegacyResultWrapper(std::unique_ptr<const SkImage::RGBAAsyncReadPixelsResult> wrappedResult)
                : fWrappedResult(std::move(wrappedResult)) {}

        int count() const override { return 1; }

        const void* data(int i) const override {
            return (i == 0) ? fWrappedResult->pixmap().addr() : nullptr;
        }

        size_t rowBytes(int i) const override {
            return (i == 0) ? fWrappedResult->pixmap().rowBytes() : 0;
        }

    private:
        std::unique_ptr<const SkImage::RGBAAsyncReadPixelsResult> fWrappedResult;
    };

    std::unique_ptr<LegacyWrapperContext> wrapperContext(
            static_cast<LegacyWrapperContext*>(rgbaContext));

    std::unique_ptr<const SkImage::AsyncReadResult> clientResult;
    if (rgbaResult) {
        clientResult = std::make_unique<LegacyResultWrapper>(std::move(rgbaResult));
    }

    wrapperContext->fClientCallback(wrapperContext->fClientContext, std::move(clientResult));
}

void LegacyYUVACallbackWrapper(
        SkImage::ReadPixelsContext yuvaContext,
        std::unique_ptr<const SkImage::YUVAAsyncReadPixelsResult> yuvaResult) {
    class LegacyResultWrapper : public SkImage::AsyncReadResult {
    public:
        LegacyResultWrapper(std::unique_ptr<const SkImage::YUVAAsyncReadPixelsResult> wrappedResult)
                : fWrappedResult(std::move(wrappedResult)) {}

        int count() const override { return fWrappedResult->pixmaps().numPlanes(); }

        const void* data(int i) const override {
            if (i < 0 || i >= fWrappedResult->pixmaps().numPlanes()) {
                return nullptr;
            }
            return fWrappedResult->pixmaps().plane(i).addr();
        }

        size_t rowBytes(int i) const override {
            if (i < 0 || i >= fWrappedResult->pixmaps().numPlanes()) {
                return 0;
            }
            return fWrappedResult->pixmaps().plane(i).rowBytes();
        }

    private:
        std::unique_ptr<const SkImage::YUVAAsyncReadPixelsResult> fWrappedResult;
    };

    std::unique_ptr<LegacyWrapperContext> wrapperContext(
            static_cast<LegacyWrapperContext*>(yuvaContext));

    std::unique_ptr<const SkImage::AsyncReadResult> clientResult;
    if (yuvaResult) {
        clientResult = std::make_unique<LegacyResultWrapper>(std::move(yuvaResult));
    }

    wrapperContext->fClientCallback(wrapperContext->fClientContext, std::move(clientResult));
}

}  // namespace SkRescaleAndReadPixelsUtils
