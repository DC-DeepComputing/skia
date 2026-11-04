/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkImage_Base.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/private/SkDebug.h"
#include "include/private/SkMath.h"
#include "src/core/SkBitmapCache.h"
#include "src/image/SkRescaleAndReadPixels.h"

#include <atomic>
#include <utility>

SkImage_Base::SkImage_Base(const SkImageInfo& info, uint32_t uniqueID,
                           sk_sp<SkPixelStorage> storage)
        : SkImage(info, uniqueID)
        , fAddedToRasterCache(false) {
    if (storage) {
        fPixelStorages.push_back(std::move(storage));
    }
}

SkImage_Base::~SkImage_Base() {
    if (fAddedToRasterCache.load()) {
        SkNotifyBitmapGenIDIsStale(this->uniqueID());
    }
}

void SkImage_Base::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                               SkIRect origSrcRect,
                                               RescaleGamma rescaleGamma,
                                               RescaleMode rescaleMode,
                                               ReadPixelsCallback callback,
                                               ReadPixelsContext context) const {
    SkBitmap src;
    SkPixmap peek;
    SkIRect srcRect;
    if (this->peekPixels(&peek)) {
        src.installPixels(peek);
        srcRect = origSrcRect;
    } else {
        // Context TODO: Elevate GrDirectContext requirement to public API.
        auto dContext = as_IB(this)->directContext();
        src.setInfo(this->imageInfo().makeDimensions(origSrcRect.size()));
        src.allocPixels();
        if (!this->readPixels(dContext, src.pixmap(), origSrcRect.x(), origSrcRect.y())) {
            callback(context, nullptr);
            return;
        }
        srcRect = SkIRect::MakeSize(src.dimensions());
    }
    return SkRescaleAndReadPixels(src, info, srcRect, rescaleGamma, rescaleMode, callback, context);
}

bool SkImage_Base::onAsLegacyBitmap(GrDirectContext* dContext, SkBitmap* bitmap) const {
    // As the base-class, all we can do is make a copy (regardless of mode).
    // Subclasses that want to be more optimal should override.
    SkImageInfo info = fInfo.makeColorType(kN32_SkColorType).makeColorSpace(nullptr);
    if (!bitmap->tryAllocPixels(info)) {
        return false;
    }

    if (!this->readPixels(
                dContext, bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(), 0, 0)) {
        bitmap->reset();
        return false;
    }

    bitmap->setImmutable();
    return true;
}

sk_sp<SkImage> SkImage_Base::makeSubset(SkRecorder* recorder,
                                        const SkIRect& subset,
                                        RequiredProperties requiredProps) const {
    if (subset.isEmpty()) {
        return nullptr;
    }

    const SkIRect bounds = SkIRect::MakeWH(this->width(), this->height());
    if (!bounds.contains(subset)) {
        return nullptr;
    }

    return this->onMakeSubset(recorder, subset, requiredProps);
}

void SkImage_Base::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                                     bool readAlpha,
                                                     sk_sp<SkColorSpace> dstColorSpace,
                                                     SkIRect srcRect,
                                                     SkISize dstSize,
                                                     RescaleGamma,
                                                     RescaleMode,
                                                     ReadPixelsCallback callback,
                                                     ReadPixelsContext context) const {
    // TODO: Call non-YUV asyncRescaleAndReadPixels and then make our callback convert to YUV and
    // call client's callback.
    callback(context, nullptr);
}

sk_sp<SkImage> SkImage_Base::makeColorSpace(SkRecorder* recorder,
                                            sk_sp<SkColorSpace> target,
                                            RequiredProperties props) const {
    return this->makeColorTypeAndColorSpace(recorder, this->colorType(), std::move(target), props);
}

sk_sp<SkImage> SkImage_Base::MakeBitmapImage(SkRecorder* recorder, const SkBitmap& bitmap) {
    if (!recorder) {
        return nullptr;
    }
    return recorder->makeBitmapImage(bitmap);
}

bool SkImage_Base::ValidArgs(const SkImageInfo& info, size_t rowBytes, size_t* minSize) {
    const int maxDimension = SK_MaxS32 >> 2;

    // TODO(mtklein): eliminate anything here that setInfo() has already checked.
    SkBitmap b;
    if (!b.setInfo(info, rowBytes)) {
        return false;
    }

    if (info.width() <= 0 || info.height() <= 0) {
        return false;
    }
    if (info.width() > maxDimension || info.height() > maxDimension) {
        return false;
    }
    if ((unsigned)info.colorType() > (unsigned)kLastEnum_SkColorType) {
        return false;
    }
    if ((unsigned)info.alphaType() > (unsigned)kLastEnum_SkAlphaType) {
        return false;
    }

    if (kUnknown_SkColorType == info.colorType()) {
        return false;
    }
    if (!info.validRowBytes(rowBytes)) {
        return false;
    }

    size_t size = info.computeByteSize(rowBytes);
    if (SkImageInfo::ByteSizeOverflowed(size)) {
        return false;
    }

    if (minSize) {
        *minSize = size;
    }
    return true;
}

void SkImage_Base::ReleaseData(void* addr, void* context) {
    SkData* data = static_cast<SkData*>(context);
    data->unref();
}
