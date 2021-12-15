/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/MagicCodeDictionary.h"

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

MagicCodeDictionary::MagicCodeDictionary() {
    // The 0th index is reserved as invalid
    fVector.push_back(nullptr);
}

size_t MagicCodeDictionary::Hash::operator()(SkCode code) const {
    return SkOpts::hash_fn(code.data(), code.sizeInBytes(), 0);
}

const MagicCodeDictionary::FullCodeEntry *MagicCodeDictionary::findOrCreateFull(
        FullCodeEntry *newEntry) {
    auto iter = fFullCodeHash.find(newEntry->fCode);
    if (iter != fFullCodeHash.end()) {
        return iter->second;
    }

    newEntry->setUniqueCodeID(UniqueCodeID::Next());
    fFullCodeHash.insert(std::make_pair(newEntry->fCode, newEntry));

    return newEntry;
}

const MagicCodeDictionary::FullCodeEntry *
MagicCodeDictionary::exhaustiveSearch(const SkSL::String &code) const {

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

} // namespace skgpu
