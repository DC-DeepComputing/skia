/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ProgramCache.h"

namespace skgpu {

////////////////////////////////////////////////////////////////////////////////////////////////////
ProgramCache::ProgramInfo::ProgramInfo(uint32_t uniqueID, Combination c)
    : fID(uniqueID)
    , fCombination(c) {
}

ProgramCache::ProgramInfo::~ProgramInfo() {}

////////////////////////////////////////////////////////////////////////////////////////////////////
ProgramCache::ProgramCache() {
    // kInvalidProgramID (aka 0) is reserved
    fProgramVector.push_back(nullptr);
}

size_t ProgramCache::Hash::operator()(Combination c) const {
    return static_cast<int>(c.fShaderType) +
           static_cast<int>(c.fTileMode) +
           static_cast<int>(c.fBlendMode);
}

sk_sp<ProgramCache::ProgramInfo> ProgramCache::findOrCreateProgram(Combination c) {
    auto iter = fProgramHash.find(c);
    if (iter != fProgramHash.end()) {
        SkASSERT(iter->second->id() != kInvalidProgramID);
        return iter->second;
    }

    sk_sp<ProgramInfo> pi(new ProgramInfo(fNextUniqueID++, c));
    fProgramHash.insert(std::make_pair(c, pi));
    fProgramVector.push_back(pi);
    SkASSERT(fProgramVector[pi->id()] == pi);
    return pi;
}

sk_sp<ProgramCache::ProgramInfo> ProgramCache::lookup(uint32_t uniqueID) {
    SkASSERT(uniqueID < fProgramVector.size());
    return fProgramVector[uniqueID];
}

} // namespace skgpu
