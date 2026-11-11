/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkMath.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Raster.h"

#include <cstddef>
#include <cstdint>
#include <utility>

class SkImageFilter;
struct SkIPoint;
struct SkIRect;
enum class SkTextureCompressionType;

namespace SkImages {
sk_sp<SkImage> RasterFromBitmap(const SkBitmap& bm) {
    if (!bm.pixelRef()) {
        return nullptr;
    }

    return SkImage_Raster::MakeFromBitmap(bm, SkCopyPixelsMode::kIfMutable);
}

sk_sp<SkImage> RasterFromPixmapCopy(const SkPixmap& pmap) {
    size_t size;
    if (!SkImage_Base::ValidArgs(pmap.info(), pmap.rowBytes(), &size) || !pmap.addr()) {
        return nullptr;
    }

    // Here we actually make a copy of the caller's pixel data
    sk_sp<SkData> data(SkData::MakeWithCopy(pmap.addr(), size));
    return sk_make_sp<SkImage_Raster>(
            pmap.info(), std::move(data), pmap.rowBytes(), /*mips=*/nullptr, kNeedNewImageUniqueID);
}

sk_sp<SkImage> RasterFromData(const SkImageInfo& info, sk_sp<SkData> data, size_t rowBytes) {
    size_t size;
    if (!SkImage_Base::ValidArgs(info, rowBytes, &size) || !data) {
        return nullptr;
    }

    // did they give us enough data?
    if (data->size() < size) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Raster>(
            info, std::move(data), rowBytes, /*mips=*/nullptr, kNeedNewImageUniqueID);
}

sk_sp<SkImage> MakeWithFilter(sk_sp<SkImage> src,
                              const SkImageFilter* filter,
                              const SkIRect& subset,
                              const SkIRect& clipBounds,
                              SkIRect* outSubset,
                              SkIPoint* offset) {
    if (!src || !filter) {
        return nullptr;
    }

    sk_sp<skif::Backend> backend = skif::MakeRasterBackend({}, src->colorType());
    return as_IFB(filter)->makeImageWithFilter(std::move(backend),
                                               std::move(src),
                                               subset,
                                               clipBounds,
                                               outSubset,
                                               offset);
}

// TODO: this could be improved to decode and make use of the mipmap
// levels potentially present in the compressed data. For now, any
// mipmap levels are discarded.
sk_sp<SkImage> RasterFromCompressedTextureData(sk_sp<SkData> data,
                                               int width,
                                               int height,
                                               SkTextureCompressionType type) {
    size_t expectedSize = SkCompressedFormatDataSize(type, {width, height}, false);
    if (!data || data->size() < expectedSize) {
        return nullptr;
    }

    SkAlphaType at =
            SkTextureCompressionTypeIsOpaque(type) ? kOpaque_SkAlphaType : kPremul_SkAlphaType;

    SkImageInfo ii = SkImageInfo::MakeN32(width, height, at);

    if (!SkImage_Base::ValidArgs(ii, ii.minRowBytes(), nullptr)) {
        return nullptr;
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(ii)) {
        return nullptr;
    }

    if (!SkDecompress(std::move(data), {width, height}, type, &bitmap)) {
        return nullptr;
    }

    bitmap.setImmutable();
    return RasterFromBitmap(bitmap);
}

sk_sp<SkImage> RasterFromPixmap(const SkPixmap& pmap, RasterReleaseProc proc, ReleaseContext ctx) {
    size_t size;
    if (!SkImage_Base::ValidArgs(pmap.info(), pmap.rowBytes(), &size) || !pmap.addr()) {
        return nullptr;
    }

    sk_sp<SkData> data(SkData::MakeWithProc(pmap.addr(), size, proc, ctx));
    return sk_make_sp<SkImage_Raster>(
            pmap.info(), std::move(data), pmap.rowBytes(), /*mips=*/nullptr, kNeedNewImageUniqueID);
}

sk_sp<SkImage> FromBitmap(SkRecorder* recorder, const SkBitmap& bitmap) {
    if (!recorder || !bitmap.pixelRef()) {
        return nullptr;
    }

    SkCopyPixelsMode cpm;
    if (recorder->type() == SkRecorder::Type::kCPU) {
        cpm = SkCopyPixelsMode::kIfMutable;
    } else {
        cpm = SkCopyPixelsMode::kNever;
    }

    return SkImage_Base::MakeRecorderBackedImage(recorder,
                                                 SkImage_Raster::MakeFromBitmap(bitmap, cpm));
}

sk_sp<SkImage> FromPixmap(SkRecorder* recorder,
                          const SkPixmap& pmap,
                          RasterReleaseProc proc,
                          ReleaseContext ctx) {
    if (!recorder) {
        return nullptr;
    }

    sk_sp<SkImage> image;
    if (recorder->type() == SkRecorder::Type::kCPU && !proc && !ctx) {
        image = RasterFromPixmapCopy(pmap);
    } else {
        image = RasterFromPixmap(pmap, proc, ctx);
    }

    return SkImage_Base::MakeRecorderBackedImage(recorder, std::move(image));
}

sk_sp<SkImage> FromData(SkRecorder* recorder,
                        const SkImageInfo& info,
                        sk_sp<SkData> data,
                        size_t rowBytes) {
    if (!recorder) {
        return nullptr;
    }

    return SkImage_Base::MakeRecorderBackedImage(recorder,
                                                 RasterFromData(info, std::move(data), rowBytes));
}

}  // namespace SkImages
