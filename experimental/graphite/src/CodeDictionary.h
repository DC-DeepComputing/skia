/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CodeDictionary_DEFINED
#define skgpu_CodeDictionary_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkCode.h"

class UniquePaintParamsID {
public:
    static UniquePaintParamsID InvalidID() { return UniquePaintParamsID(); }

    UniquePaintParamsID() : fID(SK_InvalidUniqueID) {}

    uint32_t asUInt() const { return fID; }

    bool operator==(const UniquePaintParamsID &that) const { return fID == that.fID; }

    bool operator!=(const UniquePaintParamsID &that) const { return !(*this == that); }

    bool isValid() const { return fID != SK_InvalidUniqueID; }

    explicit UniquePaintParamsID(uint32_t id) : fID(id) {}

private:
    uint32_t fID;
};

namespace skgpu {

class CodeDictionary {
public:
    CodeDictionary();

    struct FullCodeEntry {
        UniquePaintParamsID uniqueCodeID() const {
            SkASSERT(fUniqueCodeID.isValid());
            return fUniqueCodeID;
        }

        void setUniqueCodeID(uint32_t newID) {
            SkASSERT(!fUniqueCodeID.isValid());
            fUniqueCodeID = UniquePaintParamsID(newID);
        }

        UniquePaintParamsID fUniqueCodeID;
        SkCode fCode;
        // TODO: we could also lazily cache the generated SkSL here
    };

    const FullCodeEntry* findOrCreateFull(FullCodeEntry *);

    const FullCodeEntry* lookup(UniquePaintParamsID) const;

private:
    struct Hash {
        size_t operator()(SkCode) const;
    };

    std::unordered_map<SkCode, FullCodeEntry *, Hash> fFullCodeHash;
    std::vector<FullCodeEntry *> fEntryVector;
};

} // namespace skgpu

#endif // skgpu_CodeDictionary_DEFINED
