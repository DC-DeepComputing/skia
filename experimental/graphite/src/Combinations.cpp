/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Combinations.h"

#include "experimental/graphite/include/Encoding.h"

// remove this
#include "experimental/graphite/include/Context.h"

int FirstCombinableTileMode(CombinableTileMode combinableTileMode) {
    static_assert(0x1 == static_cast<int>(CombinableTileMode::kFirst));

    int initialTM = static_cast<int>(CombinableTileMode::kFirst);
    for ( ; initialTM <= static_cast<int>(CombinableTileMode::kLast); initialTM <<= 1) {
        if (initialTM & static_cast<int>(combinableTileMode)) {
            break;
        }
    }

    SkASSERT(initialTM <= static_cast<int>(CombinableTileMode::kLast));
    return initialTM;
}

int FirstCombinableGradientType(CombinableGradientType combinableGradientType) {
    static_assert(0x1 == static_cast<int>(CombinableGradientType::kFirst));

    int initialGT = static_cast<int>(CombinableGradientType::kFirst);
    for ( ; initialGT <= static_cast<int>(CombinableGradientType::kLast); initialGT <<= 1) {
        if (initialGT & static_cast<int>(combinableGradientType)) {
            break;
        }
    }

    SkASSERT(initialGT <= static_cast<int>(CombinableGradientType::kLast));
    return initialGT;
}

namespace skgpu {

void ImageShaderCombinations::addToCode(SkShaderCodeDictionary* dict,
                                        const Iter &iter,
                                        SkBackend backend,
                                        SkPaintParamsKey *key) const {
    SkASSERT(!iter.fDone);

    ImageShaderBlock::ImageData imgData{
            CombinableTMToTM((CombinableTileMode) iter.fIndices[0]),
            CombinableTMToTM((CombinableTileMode) iter.fIndices[1])
    };

    ImageShaderBlock::AddToKey(dict, backend, key, nullptr, imgData);
}

void GradientCombinations::addToCode(SkShaderCodeDictionary* dict,
                                     const Iter& iter,
                                     SkBackend backend,
                                     SkPaintParamsKey* key) const {
    SkASSERT(!iter.fDone);

    GradientShaderBlocks::AddToKey(dict, backend, key, nullptr,
                                   { CombinableGTToGT((CombinableGradientType) iter.fIndices[0]),
                                     CombinableTMToTM((CombinableTileMode) iter.fIndices[1]),
                                     iter.fIndices[2] });
}

}; // namespace skgpu
