/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ImageCache_DEFINED
#define skgpu_graphite_ImageCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkTHash.h"

class SkBitmap;
class SkImage;

namespace skgpu {
    enum class Mipmapped : bool;
    class UniqueKey;
}

namespace skgpu::graphite {

class Recorder;

class ImageCache {
public:
    ~ImageCache();

    sk_sp<SkImage> findOrCreateCachedImage(Recorder*, const SkBitmap&, Mipmapped);

private:

    struct UniqueKeyHash {
        uint32_t operator()(const skgpu::UniqueKey& key) const;
    };
    typedef SkTHashMap<skgpu::UniqueKey, sk_sp<SkImage>, UniqueKeyHash> UniqueKeyImageHash;

    UniqueKeyImageHash fCache;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ImageCache_DEFINED
