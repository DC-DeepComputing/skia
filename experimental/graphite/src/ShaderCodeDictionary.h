/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CodeDictionary_DEFINED
#define skgpu_CodeDictionary_DEFINED

#include "experimental/graphite/src/UniquePaintParamsID.h"
#include "include/core/SkTypes.h"
#include "include/private/SkPaintParamsKey.h"

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
        SkPaintParamsKey fCode;
        // TODO: we could also lazily cache the generated SkSL here
    };

    const FullCodeEntry* findOrCreateFull(FullCodeEntry *);

    const FullCodeEntry* lookup(UniquePaintParamsID) const;

private:
    struct Hash {
        size_t operator()(SkPaintParamsKey) const;
    };

    std::unordered_map<SkPaintParamsKey, FullCodeEntry *, Hash> fFullCodeHash;
    std::vector<FullCodeEntry *> fEntryVector;
};

} // namespace skgpu

#endif // skgpu_CodeDictionary_DEFINED
