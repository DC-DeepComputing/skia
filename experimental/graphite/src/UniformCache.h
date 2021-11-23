/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UniformCache_DEFINED
#define skgpu_UniformCache_DEFINED

#include "include/core/SkRefCnt.h"

#include <unordered_map>
#include <vector>

namespace skgpu {

class UniformData;

class UniformCache {
public:
    static constexpr uint32_t kInvalidUniformID = 0;

    UniformCache();

    uint32_t insert(sk_sp<UniformData>);

    sk_sp<UniformData> lookup(uint32_t uniqueID);

    // The number of unique uniformdata objects in the cache
    size_t count() const {
        SkASSERT(fUniformData.size() == fUniformDataIDs.size() && fUniformData.size() > 0);
        return fUniformData.size() - 1;
    }

private:
    struct Hash {
        // This hash operator de-references and hashes the data contents
        size_t operator()(UniformData*) const;
    };
    struct Eq {
        // This equality operator de-references and compares the actual data contents
        bool operator()(UniformData*, UniformData*) const;
    };

    // The UniformData's unique ID is only unique w/in a Recorder _not_ globally
    std::unordered_map<UniformData*, uint32_t, Hash, Eq> fUniformDataIDs;
    std::vector<sk_sp<UniformData>> fUniformData;

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif
};

} // namespace skgpu

#endif // skgpu_UniformCache_DEFINED
