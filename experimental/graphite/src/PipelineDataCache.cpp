/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/PipelineDataCache.h"

#include "src/core/SkOpts.h"
#include "src/core/SkPipelineData.h"

namespace skgpu {

size_t UniformDataCache::Hash::operator()(SkPipelineData::UniformDataBlock* udb) const {
    if (!udb) {
        return 0;
    }

    return udb->hash1();
}

bool UniformDataCache::Eq::operator()(SkPipelineData::UniformDataBlock* a,
                                      SkPipelineData::UniformDataBlock* b) const {
    if (!a || !b) {
        return !a && !b;
    }

    return *a == *b;
};

UniformDataCache::UniformDataCache() {
    // kInvalidUniformDataID is reserved
    static_assert(kInvalidUniformDataID == 0);
    fUniformDataBlocks.push_back(nullptr);
    fUniformDataBlockIDs.insert({nullptr, UniformDataIndex()});
}

#ifdef SK_DEBUG
void UniformDataCache::validate() const {
    for (size_t i = 0; i < fUniformDataBlocks.size(); ++i) {
        auto kv = fUniformDataBlockIDs.find(fUniformDataBlocks[i].get());
        SkASSERT(kv != fUniformDataBlockIDs.end());
        SkASSERT(kv->first == fUniformDataBlocks[i].get());
        SkASSERT(SkTo<uint32_t>(i) == kv->second.asUInt());
    }
}
#endif

UniformDataCache::UniformDataIndex UniformDataCache::insert1(const SkPipelineData::UniformDataBlock& udb) {
    auto kv = fUniformDataBlockIDs.find(const_cast<SkPipelineData::UniformDataBlock*>(&udb));
    if (kv != fUniformDataBlockIDs.end()) {
        return kv->second;
    }

    UniformDataIndex id(SkTo<uint32_t>(fUniformDataBlocks.size()));
    SkASSERT(id.isValid());

    std::unique_ptr<SkPipelineData::UniformDataBlock> tmp(new SkPipelineData::UniformDataBlock(udb));
    fUniformDataBlockIDs.insert({tmp.get(), id});
    fUniformDataBlocks.push_back(std::move(tmp));
    this->validate();
    return id;
}

SkPipelineData::UniformDataBlock* UniformDataCache::lookup1(UniformDataIndex uniqueID) {
    SkASSERT(uniqueID.asUInt() < fUniformDataBlocks.size());
    return fUniformDataBlocks[uniqueID.asUInt()].get();
}

} // namespace skgpu
