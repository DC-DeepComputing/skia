/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkChecksum_DEFINED
#define SkChecksum_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkOpts_spi.h"
#include "include/private/base/SkTLogic.h"

#include <optional>
#include <string>
#include <string_view>

class SkChecksum {
public:
    SkChecksum() = default;
    // Make noncopyable
    SkChecksum(const SkChecksum&) = delete;
    SkChecksum& operator=(const SkChecksum&) = delete;

    /**
     * uint32_t -> uint32_t hash, useful for when you're about to trucate this hash but you
     * suspect its low bits aren't well mixed.
     *
     * This is the Murmur3 finalizer.
     */
    static uint32_t Mix(uint32_t hash) {
        hash ^= hash >> 16;
        hash *= 0x85ebca6b;
        hash ^= hash >> 13;
        hash *= 0xc2b2ae35;
        hash ^= hash >> 16;
        return hash;
    }

    /**
     * uint32_t -> uint32_t hash, useful for when you're about to trucate this hash but you
     * suspect its low bits aren't well mixed.
     *
     *  This version is 2-lines cheaper than Mix, but seems to be sufficient for the font cache.
     */
    static uint32_t CheapMix(uint32_t hash) {
        hash ^= hash >> 16;
        hash *= 0x85ebca6b;
        hash ^= hash >> 16;
        return hash;
    }
};

// SkGoodHash should usually be your first choice in hashing data.
// It should be both reasonably fast and high quality.
template <typename K>
struct SkGoodHash {
    uint32_t operator()(const K& k) const {
        if constexpr(sizeof(K) == 4) {
            return SkChecksum::Mix(*(const uint32_t*)&k);
        } else {
            return SkOpts::hash_fn(&k, sizeof(K), 0);
        }
    }
};

template <typename K>
struct SkGoodHash<SkSpan<K>> {
    uint32_t operator()(SkSpan<K> k) const {
        return SkOpts::hash_fn(k.data(), k.size_bytes(), 0);
    }
};

template <typename K>
struct SkGoodHash<std::optional<K>> {
    uint32_t operator()(const std::optional<K>& k) const {
        return k.has_value() ? SkGoodHash<K>()(*k) : 0;
    }
};

template <>
struct SkGoodHash<std::string_view> {
    uint32_t operator()(std::string_view k) const {
        return SkOpts::hash_fn(k.data(), k.size(), 0);
    }
};

template <>
struct SkGoodHash<SkString> {
    uint32_t operator()(const SkString& k) const {
        return SkOpts::hash_fn(k.c_str(), k.size(), 0);
    }
};

template <>
struct SkGoodHash<std::string> {
    uint32_t operator()(const std::string& k) const {
        return SkOpts::hash_fn(k.c_str(), k.size(), 0);
    }
};

#endif
