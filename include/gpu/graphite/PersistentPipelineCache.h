/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PersistentPipelineCache_DEFINED
#define skgpu_graphite_PersistentPipelineCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class SkData;
class SkString;

namespace skgpu::graphite {

/**
 * Abstract class which can store Pipeline data in a cache that persists between sessions.
 */
class SK_API PersistentPipelineCache {
public:
    virtual ~PersistentPipelineCache() = default;

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
    PersistentPipelineCache() = default;
    PersistentPipelineCache(const PersistentPipelineCache&) = delete;
    PersistentPipelineCache& operator=(const PersistentPipelineCache&) = delete;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_PersistentPipelineCache_DEFINED
