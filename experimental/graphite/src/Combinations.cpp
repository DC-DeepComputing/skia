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

void ImageShaderCombinations::addToCode(const Iter &iter, SkPaintParamsKey *key) const {
    SkASSERT(!iter.fDone);

    int index = key->writeBlockHeader(CodeSnippetID::kImageShader);

    SkImageShader::ImageData data{
            CombinableTMToTM((CombinableTileMode) iter.fIndices[0]),
            CombinableTMToTM((CombinableTileMode) iter.fIndices[1])
    };
    int numIntsWritten = SkImageShader::Encode1(nullptr, SkBackend::kGanesh, key, data);

    key->patchBlockHeader(index, CodeSnippetID::kImageShader, numIntsWritten);
}

void GradientCombinations::addToCode(const Iter& iter, SkPaintParamsKey* key) const {
    SkASSERT(!iter.fDone);

    int index = key->writeBlockHeader(CodeSnippetID::kLinearGradientShader);

    int numIntsWritten = Encode1(nullptr, SkBackend::kGanesh, key,
                                 { CombinableGTToGT((CombinableGradientType) iter.fIndices[0]),
                                   CombinableTMToTM((CombinableTileMode) iter.fIndices[1]),
                                   iter.fIndices[2] });

    key->patchBlockHeader(index, CodeSnippetID::kLinearGradientShader, numIntsWritten);
}

}; // namespace skgpu
