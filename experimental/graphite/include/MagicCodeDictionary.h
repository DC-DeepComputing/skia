/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MagicCodeDictionary_DEFINED
#define skgpu_MagicCodeDictionary_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkCode.h"

class UniqueCodeID {
public:
    static UniqueCodeID InvalidID() { return UniqueCodeID(); }

    static UniqueCodeID Next();

    UniqueCodeID() : fID(SK_InvalidUniqueID) {}

    uint32_t asUInt() const { return fID; }

    bool operator==(const UniqueCodeID &that) const { return fID == that.fID; }

    bool operator!=(const UniqueCodeID &that) const { return !(*this == that); }

    bool isValid() const { return fID != SK_InvalidUniqueID; }

private:
    explicit UniqueCodeID(uint32_t id) : fID(id) {}

    uint32_t fID;
};

namespace skgpu {

class MagicCodeDictionary {
public:
    MagicCodeDictionary();

    struct FullCodeEntry {
        void setUniqueCodeID(UniqueCodeID newID) {
            SkASSERT(!fUniqueCodeID.isValid());
            fUniqueCodeID = newID;
        }

        UniqueCodeID fUniqueCodeID;
        SkCode fCode;
        SkSL::String fSkSL;
    };

    const FullCodeEntry *findOrCreateFull(FullCodeEntry *);

    const FullCodeEntry *exhaustiveSearch(const SkSL::String &code) const;

private:
    struct Hash {
        size_t operator()(SkCode) const;
    };

    std::unordered_map<SkCode, FullCodeEntry *, Hash> fFullCodeHash;
    std::vector<FullCodeEntry *> fVector;
};

} // namespace skgpu

#endif // skgpu_MagicCodeDictionary_DEFINED
