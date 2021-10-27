/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UniformCache_DEFINED
#define skgpu_UniformCache_DEFINED

#include <unordered_set>
#include "include/core/SkRefCnt.h"

namespace skgpu {

class UniformData;

class UniformCache {
public:
    uint32_t findOrCreate(sk_sp<UniformData>);

    size_t count() const { return fUniforms.size(); }

private:
    struct Hash {
        size_t operator()(sk_sp<UniformData>) const;
    };
    struct Eq {
        // This equality operator doesn't compare the UniformData's ids
        bool operator()(sk_sp<UniformData>, sk_sp<UniformData>);
    };

    std::unordered_set<sk_sp<UniformData>, Hash, Eq> fUniforms;
};

} // namespace skgpu

#endif // skgpu_UniformCache_DEFINED
