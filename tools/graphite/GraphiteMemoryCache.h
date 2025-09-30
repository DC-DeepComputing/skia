/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GraphiteMemoryCache_DEFINED
#define GraphiteMemoryCache_DEFINED

#include "include/core/SkData.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "src/core/SkChecksum.h"

#include <unordered_map>

namespace sk_gpu_test {

/**
 * This class can be used to maintain an in memory record of all programs cached by GrContext.
 * It can be shared by multiple GrContexts so long as those GrContexts are created with the same
 * options and will have the same GrCaps (e.g. same backend, same GL context creation parameters,
 * ...).
 */
class GraphiteMemoryCache : public skgpu::graphite::ContextOptions::PersistentCache {
public:
    GraphiteMemoryCache() = default;
    GraphiteMemoryCache(const GraphiteMemoryCache&) = delete;
    GraphiteMemoryCache& operator=(const GraphiteMemoryCache&) = delete;
    void reset() {
        this->resetCacheStats();
        fMap.clear();
    }

    sk_sp<SkData> load(const SkData& key) override;
    void store(const SkData& key, const SkData& data, const SkString& description) override;
    int numCacheMisses() const { return fCacheMissCnt; }
    int numCacheStores() const { return fCacheStoreCnt; }
    void resetCacheStats() {
        fCacheMissCnt = 0;
        fCacheStoreCnt = 0;
    }

private:
    struct Key {
        Key() = default;
        Key(const SkData& key) : fKey(SkData::MakeWithCopy(key.data(), key.size())) {}
        Key(const Key& that) = default;
        Key& operator=(const Key&) = default;
        bool operator==(const Key& that) const {
            return that.fKey->size() == fKey->size() &&
                   !memcmp(fKey->data(), that.fKey->data(), that.fKey->size());
        }
        sk_sp<const SkData> fKey;
    };

    struct Value {
        Value() = default;
        Value(const SkData& data, const SkString& description)
            : fData(SkData::MakeWithCopy(data.data(), data.size()))
            , fDescription(description)
            , fHitCount(1) {}
        Value(const Value& that) = default;
        Value& operator=(const Value&) = default;

        sk_sp<SkData> fData;
        SkString      fDescription;
        int           fHitCount;
    };

    struct Hash {
        using argument_type = Key;
        using result_type = uint32_t;
        uint32_t operator()(const Key& key) const {
            return key.fKey ? SkChecksum::Hash32(key.fKey->data(), key.fKey->size()) : 0;
        }
    };

    int fCacheMissCnt = 0;
    int fCacheStoreCnt = 0;
    std::unordered_map<Key, Value, Hash> fMap;
};

}  // namespace sk_gpu_test

#endif // GraphiteMemoryCache_DEFINED
