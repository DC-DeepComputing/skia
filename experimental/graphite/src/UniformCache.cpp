/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/UniformCache.h"

#include "src/core/SkOpts.h"
#include "src/core/SkUniformData.h"

namespace skgpu {

size_t UniformCache::Hash::operator()(SkUniformBlock* ub) const {
    if (!ub) {
        return 0;
    }

    return ub->hash();
//    return SkOpts::hash_fn(ub->data(), ub->dataSize(), 0);
}

bool UniformCache::Eq::operator()(SkUniformBlock* a, SkUniformBlock* b) const {
    if (!a || !b) {
        return !a && !b;
    }

    return *a == *b;
#if 0
    if (a->count() != b->count() ||
        a->uniforms() != b->uniforms() ||
        a->dataSize() != b->dataSize()) {
        return false;
    }

    return !memcmp(a->data(), b->data(), a->dataSize()) &&
           !memcmp(a->offsets(), b->offsets(), a->count()*sizeof(uint32_t));
#endif
};

UniformCache::UniformCache() {
    // kInvalidUniformID is reserved
    static_assert(kInvalidUniformID == 0);
    fUniformData.push_back(nullptr);
    fUniformDataIDs.insert({nullptr, 0});
}

#ifdef SK_DEBUG
void UniformCache::validate() const {
    for (size_t i = 0; i < fUniformData.size(); ++i) {
        auto kv = fUniformDataIDs.find(fUniformData[i].get());
        SkASSERT(kv != fUniformDataIDs.end());
        SkASSERT(kv->first == fUniformData[i].get());
        SkASSERT(SkTo<uint32_t>(i) == kv->second);
    }
}
#endif

uint32_t UniformCache::insert(std::unique_ptr<SkUniformBlock> block) {
    auto kv = fUniformDataIDs.find(block.get());
    if (kv != fUniformDataIDs.end()) {
        return kv->second;
    }

    uint32_t id = SkTo<uint32_t>(fUniformData.size());
    SkASSERT(block && id != kInvalidUniformID);

    fUniformDataIDs.insert({block.get(), id});
    fUniformData.push_back(std::move(block));
    this->validate();
    return id;
}

SkUniformBlock* UniformCache::lookup1(uint32_t uniqueID) {
    SkASSERT(uniqueID < fUniformData.size());
    return fUniformData[uniqueID].get();
}

} // namespace skgpu
