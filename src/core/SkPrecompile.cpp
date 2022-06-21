/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPrecompile.h"

#include "src/core/SkKeyContext.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkShaderCodeDictionary.h"

int SkPaintOptions::numShaderCombinations() const {
    int numShaderCombinations = 0;
    for (const sk_sp<SkPrecompileShader>& s : fShaderOptions) {
        numShaderCombinations += s->numCombinations();
    }

    // If no shader option is specified we will add a solid color shader option
    return numShaderCombinations ? numShaderCombinations : 1;
}

int SkPaintOptions::numMaskFilterCombinations() const {
    int numMaskFilterCombinations = 0;
    for (const sk_sp<SkPrecompileMaskFilter>& mf : fMaskFilterOptions) {
        numMaskFilterCombinations += mf->numCombinations();
    }

    // If no mask filter options are specified we will use the geometry's coverage
    return numMaskFilterCombinations ? numMaskFilterCombinations : 1;
}

int SkPaintOptions::numColorFilterCombinations() const {
    int numColorFilterCombinations = 0;
    for (const sk_sp<SkPrecompileColorFilter>& cf : fColorFilterOptions) {
        numColorFilterCombinations += cf->numCombinations();
    }

    // If no color filter options are specified we will use the unmodified result color
    return numColorFilterCombinations ? numColorFilterCombinations : 1;
}

int SkPaintOptions::numBlendModeCombinations() const {
    int numBlenderCombinations = 0;
    for (const sk_sp<SkPrecompileBlender>& b : fBlenderOptions) {
        numBlenderCombinations += b->numCombinations();
    }

    // If no blend mode options are specified then we will use kSrcOver
    return numBlenderCombinations ? numBlenderCombinations : 1;
}

int SkPaintOptions::numCombinations() const {
    return this->numShaderCombinations() *
           this->numMaskFilterCombinations() *
           this->numColorFilterCombinations() *
           this->numBlendModeCombinations();
}

void SkPaintOptions::buildCombinations(SkShaderCodeDictionary* dict,
                                       const std::function<void(SkUniquePaintParamsID)>& func) const {
    SkKeyContext keyContext(dict);
    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);

    int numCombinations = this->numCombinations();
    for (int i = 0; i < numCombinations; ++i) {
        this->createKey(keyContext, i, &builder);

        auto entry = dict->findOrCreate(&builder);

        func(entry->uniqueID());
    }

}
