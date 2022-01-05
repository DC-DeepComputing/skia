/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CodeDictionary_DEFINED
#define skgpu_CodeDictionary_DEFINED

#include "experimental/graphite/src/UniquePaintParamsID.h"

namespace skgpu {

class CodeDictionary {
public:
    CodeDictionary();

    struct Entry {
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

    const Entry* findOrCreateFull(Entry *);

    const Entry* lookup(UniquePaintParamsID) const;

private:
    struct Hash {
        size_t operator()(SkCode) const;
    };

    std::unordered_map<SkCode, Entry *, Hash> fFullCodeHash;
    std::vector<Entry *> fEntryVector;
};

} // namespace skgpu

#endif // skgpu_CodeDictionary_DEFINED
