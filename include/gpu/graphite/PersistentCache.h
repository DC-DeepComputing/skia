/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PersistentCache_DEFINED
#define skgpu_graphite_PersistentCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class SkData;
class SkString;

namespace skgpu::graphite {

/**
 * Abstract class which stores Skia data in a cache that persists between sessions.
 */
class SK_API PersistentCache {
public:
    virtual ~PersistentCache() = default;

    /**
     * Returns the data for the key if it exists in the cache, otherwise returns null.
     */
    virtual sk_sp<SkData> load(const SkData& key) = 0;

    /**
     * Stores data in the cache, indexed by key. description provides a human-readable
     * version of the key.
     */
    virtual void store(const SkData& key, const SkData& data, const SkString& description) = 0;

protected:
    PersistentCache() = default;
    PersistentCache(const PersistentCache&) = delete;
    PersistentCache& operator=(const PersistentCache&) = delete;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_PersistentCache_DEFINED
