/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ImageCache.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixelRef.h"
#include "include/gpu/GpuTypes.h"
#include "src/gpu/ResourceKey.h"

namespace skgpu::graphite {

ImageCache::~ImageCache() {}

static void make_bitmap_key(skgpu::UniqueKey* key, const SkBitmap& bm, Mipmapped mipmapped) {
    SkASSERT(key);

    SkIPoint origin = bm.pixelRefOrigin();
    SkIRect subset = SkIRect::MakePtSize(origin, bm.dimensions());

    static const skgpu::UniqueKey::Domain kImageIDDomain = skgpu::UniqueKey::GenerateDomain();
    skgpu::UniqueKey::Builder builder(key, kImageIDDomain, 6, "Image");
    builder[0] = bm.pixelRef()->getGenerationID();
    builder[1] = subset.fLeft;
    builder[2] = subset.fTop;
    builder[3] = subset.fRight;
    builder[4] = subset.fBottom;
    builder[5] = SkToBool(mipmapped);
}

uint32_t ImageCache::UniqueKeyHash::operator()(const skgpu::UniqueKey& key) const {
    return key.hash();
}

sk_sp<SkImage> ImageCache::findOrCreateCachedImage(Recorder* recorder,
                                                   const SkBitmap& bitmap,
                                                   Mipmapped mipmapped) {
    skgpu::UniqueKey key;

    if (mipmapped == Mipmapped::kNo) {
        make_bitmap_key(&key, bitmap, Mipmapped::kYes);

        if (sk_sp<SkImage>* cached = fCache.find(key)) {
            return *cached;
        }
    }

    make_bitmap_key(&key, bitmap, mipmapped);

    if (sk_sp<SkImage>* cached = fCache.find(key)) {
        return *cached;
    }

    sk_sp<SkImage> image = SkImages::RasterFromBitmap(bitmap);
    image = image->makeTextureImage(recorder, { mipmapped });
    fCache.set(key, image);
    return image;
}

} // namespace skgpu::graphite
