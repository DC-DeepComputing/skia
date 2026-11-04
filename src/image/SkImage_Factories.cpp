/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkMath.h"
#include "src/image/SkImageGeneratorPriv.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <utility>

namespace SkImages {

sk_sp<SkImage> FromBitmap(SkRecorder* recorder, const SkBitmap& bitmap) {
    if (!recorder || !bitmap.pixelRef()) {
        return nullptr;
    }

    return SkImage_Base::MakeBitmapImage(recorder, bitmap);
}

sk_sp<SkImage> FromPixmap(SkRecorder* recorder,
                          const SkPixmap& pmap,
                          RasterReleaseProc proc,
                          ReleaseContext ctx) {
    if (!recorder) {
        return nullptr;
    }

    if (recorder->type() == SkRecorder::Type::kCPU) {
        if (!proc && !ctx) {
            return RasterFromPixmapCopy(pmap);
        }
        return RasterFromPixmap(pmap, proc, ctx);
    }

    size_t size;
    if (!SkImage_Base::ValidArgs(pmap.info(), pmap.rowBytes(), &size) || !pmap.addr()) {
        return nullptr;
    }

    sk_sp<SkData> data(SkData::MakeWithProc(pmap.addr(), size, proc, ctx));
    return FromData(recorder, pmap.info(), std::move(data), pmap.rowBytes());
}

sk_sp<SkImage> FromData(SkRecorder* recorder,
                        const SkImageInfo& info,
                        sk_sp<SkData> data,
                        size_t rowBytes) {
    if (!recorder) {
        return nullptr;
    }

    size_t size;
    if (!SkImage_Base::ValidArgs(info, rowBytes, &size) || !data || data->size() < size) {
        return nullptr;
    }

    SkBitmap bitmap;
    void* addr = data->writable_data();
    if (!bitmap.installPixels(info, addr, rowBytes, SkImage_Base::ReleaseData, data.release())) {
        return nullptr;
    }
    bitmap.setImmutable();

    return SkImage_Base::MakeBitmapImage(recorder, bitmap);
}

sk_sp<SkImage> FromData(SkRecorder* recorder,
                        sk_sp<const SkData> encoded,
                        std::optional<SkAlphaType> alphaType) {
    if (!recorder || !encoded || encoded->empty()) {
        return nullptr;
    }

    std::unique_ptr<SkImageGenerator> generator =
            SkImageGenerators::MakeFromEncoded(std::move(encoded), alphaType);
    if (!generator) {
        return nullptr;
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(generator->getInfo())) {
        return nullptr;
    }
    if (!generator->getPixels(bitmap.pixmap())) {
        return nullptr;
    }
    bitmap.setImmutable();

    return SkImage_Base::MakeBitmapImage(recorder, bitmap);
}

}  // namespace SkImages
