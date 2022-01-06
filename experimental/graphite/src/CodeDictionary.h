/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CodeDictionary_DEFINED
#define skgpu_CodeDictionary_DEFINED

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

    explicit UniqueCodeID(uint32_t id) : fID(id) {}

private:
    uint32_t fID;
};

namespace skgpu {

class CodeDictionary {
public:
    CodeDictionary();

    struct FullCodeEntry {
        UniqueCodeID uniqueCodeID() const {
            SkASSERT(fUniqueCodeID.isValid());
            return fUniqueCodeID;
        }

        void setUniqueCodeID(uint32_t newID) {
            SkASSERT(!fUniqueCodeID.isValid());
            fUniqueCodeID = UniqueCodeID(newID);
        }

        UniqueCodeID fUniqueCodeID;
        SkCode fCode;
        // TODO: we could also lazily cache the generated SkSL here
    };

    const FullCodeEntry* findOrCreateFull(FullCodeEntry *);

    const FullCodeEntry* lookup(UniqueCodeID) const;

private:
    struct Hash {
        size_t operator()(SkCode) const;
    };

    std::unordered_map<SkCode, FullCodeEntry *, Hash> fFullCodeHash;
    std::vector<FullCodeEntry *> fEntryVector;
};

} // namespace skgpu

#endif // skgpu_CodeDictionary_DEFINED
