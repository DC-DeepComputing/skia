/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/CodeDictionary.h"

namespace skgpu {

CodeDictionary::CodeDictionary() {
    // The 0th index is reserved as invalid
    fEntryVector.push_back(nullptr);
}

size_t CodeDictionary::Hash::operator()(Combination combo) const {
    return combo.key();
}

const CodeDictionary::Entry* CodeDictionary::findOrCreate(Combination combo) {
    auto iter = fHash.find(combo);
    if (iter != fHash.end()) {
        SkASSERT(fEntryVector[iter->second->uniqueID().asUInt()] == iter->second);
        return iter->second;
    }

    Entry* newEntry = new Entry(combo);
    newEntry->setUniqueID(fEntryVector.size());
    fHash.insert(std::make_pair(newEntry->combo(), newEntry));
    fEntryVector.push_back(newEntry);

    return newEntry;
}

const CodeDictionary::Entry* CodeDictionary::lookup(UniquePaintParamsID codeID) const {
    SkASSERT(codeID.isValid());
    SkASSERT(codeID.asUInt() < fEntryVector.size());

    return fEntryVector[codeID.asUInt()];
}

} // namespace skgpu
