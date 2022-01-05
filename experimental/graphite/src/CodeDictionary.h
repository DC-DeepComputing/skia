/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CodeDictionary_DEFINED
#define skgpu_CodeDictionary_DEFINED

#include <unordered_map>
#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/UniquePaintParamsID.h"

namespace skgpu {

class CodeDictionary {
public:
    CodeDictionary();

    struct Entry {
    public:
        Entry(Combination combo) : fCombo(combo) {}

        void setUniqueID(uint32_t newID) {
            SkASSERT(!fUniqueID.isValid());
            fUniqueID = UniquePaintParamsID(newID);
        }

        UniquePaintParamsID uniqueID() const {
            SkASSERT(fUniqueID.isValid());
            return fUniqueID;
        }
        Combination combo() const { return fCombo; }

    private:
        UniquePaintParamsID fUniqueID;
        // TODO: replace all uses of Combination with the variable length key
        Combination fCombo;
    };

    const Entry* findOrCreate(Combination);

    const Entry* lookup(UniquePaintParamsID) const;

private:
    struct Hash {
        size_t operator()(Combination) const;
    };

    std::unordered_map<Combination, Entry*, Hash> fHash;
    std::vector<Entry *> fEntryVector;
};

} // namespace skgpu

#endif // skgpu_CodeDictionary_DEFINED
