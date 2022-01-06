/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/CodeDictionary.h"

#include "src/core/SkOpts.h"
#include "src/utils/SkShaderUtils.h"

UniqueCodeID UniqueCodeID::Next() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidUniqueID);

    return UniqueCodeID(id);
}

namespace skgpu {

CodeDictionary::CodeDictionary() {
    // The 0th index is reserved as invalid
    fEntryVector.push_back(nullptr);
}

size_t CodeDictionary::Hash::operator()(SkCode code) const {
    return SkOpts::hash_fn(code.data(), code.sizeInBytes(), 0);
}

const CodeDictionary::FullCodeEntry *CodeDictionary::findOrCreateFull(
        FullCodeEntry *newEntry) {
    auto iter = fFullCodeHash.find(newEntry->fCode);
    if (iter != fFullCodeHash.end()) {
        SkASSERT(fEntryVector[iter->second->fUniqueCodeID.asUInt()] == iter->second);
        return iter->second;
    }

    newEntry->setUniqueCodeID(fEntryVector.size());
    fFullCodeHash.insert(std::make_pair(newEntry->fCode, newEntry));
    fEntryVector.push_back(newEntry);

    return newEntry;
}

const CodeDictionary::FullCodeEntry* CodeDictionary::lookup(UniqueCodeID codeID) const {
    SkASSERT(codeID.isValid());
    SkASSERT(codeID.asUInt() < fEntryVector.size());

    return fEntryVector[codeID.asUInt()];
}

#if 0
const CodeDictionary::FullCodeEntry *
CodeDictionary::exhaustiveSearch(const SkSL::String &code) const {

    for (const auto &e: fFullCodeHash) {
        if (code == e.second->fSkSL) {
            return e.second;
        }
    }

    SkDebugf("------------------------------\n");
    SkDebugf("Didn't find this code:\n");
    SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(code));

    SkDebugf("------------------------------\n");
    SkDebugf("in this %d list:\n", (int) fFullCodeHash.size());

    int index = 0;
    for (const auto &e: fFullCodeHash) {
        SkDebugf("%d ------------------------------\n", index++);
        SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(e.second->fSkSL));
    }

    return nullptr;
}
#endif

} // namespace skgpu
