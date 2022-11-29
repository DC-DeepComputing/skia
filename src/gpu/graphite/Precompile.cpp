/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#ifdef SK_ENABLE_PRECOMPILE

#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PrecompileBasePriv.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

namespace skgpu::graphite {


#if 0
void PrecompileBase::addToKey(const KeyContext& keyContext,
                              int desiredCombination,
                              PaintParamsKeyBuilder* keyBuilder) {
    SkASSERT(desiredCombination < this->numCombinations());

    int intrinsicCombination = desiredCombination / this->numChildCombinations();
    int childCombination = desiredCombination % this->numChildCombinations();

    this->beginBlock(keyContext, intrinsicCombination, keyBuilder);

#if 0
    if (fNumSlots) {
    int numCombinationsSeen = 0;
    for (int slotIndex = 0; slotIndex < fNumSlots; ++slotIndex) {
        SkSlot* slot = this->getSlot(slotIndex);

        numCombinationsSeen += slot->numCombinations();
        int numCombosLeft = this->numChildCombinations() / numCombinationsSeen;

        int slotCombination;
        if (slotIndex+1 < fNumSlots) {
            slotCombination = childCombination / (numCombosLeft ? numCombosLeft : 1);
            childCombination %= numCombosLeft;
        } else {
            slotCombination = childCombination;
        }

        slot->addToKey(keyContext, slotCombination, keyBuilder);
    }
}
#endif

    keyBuilder->endBlock();
}
#endif

//--------------------------------------------------------------------------------------------------
int PaintOptions::numShaderCombinations() const {
    int numShaderCombinations = 0;
    for (const sk_sp<PrecompileShader>& s : fShaderOptions) {
        numShaderCombinations += s->priv().numCombinations();
    }

    // If no shader option is specified we will add a solid color shader option
    return numShaderCombinations ? numShaderCombinations : 1;
}

int PaintOptions::numMaskFilterCombinations() const {
    int numMaskFilterCombinations = 0;
    for (const sk_sp<PrecompileMaskFilter>& mf : fMaskFilterOptions) {
        numMaskFilterCombinations += mf->priv().numCombinations();
    }

    // If no mask filter options are specified we will use the geometry's coverage
    return numMaskFilterCombinations ? numMaskFilterCombinations : 1;
}

int PaintOptions::numColorFilterCombinations() const {
    int numColorFilterCombinations = 0;
    for (const sk_sp<PrecompileColorFilter>& cf : fColorFilterOptions) {
        numColorFilterCombinations += cf->priv().numCombinations();
    }

    // If no color filter options are specified we will use the unmodified result color
    return numColorFilterCombinations ? numColorFilterCombinations : 1;
}

int PaintOptions::numBlendModeCombinations() const {
    return CountBlenderCombos(fBlenderOptions);
}

int PaintOptions::numCombinations() const {
    // TODO: we need to handle ImageFilters separately
    return this->numShaderCombinations() *
           this->numMaskFilterCombinations() *
           this->numColorFilterCombinations() *
           this->numBlendModeCombinations();
}

void PaintOptions::createKey(const KeyContext& keyContext,
                             int desiredCombination,
                             PaintParamsKeyBuilder* keyBuilder) const {
    SkDEBUGCODE(keyBuilder->checkReset();)
    SkASSERT(desiredCombination < this->numCombinations());

    int numBlendModeCombos = this->numBlendModeCombinations();
    int numColorFilterCombinations = this->numColorFilterCombinations();
    int numMaskFilterCombinations = this->numMaskFilterCombinations();

    int desiredBlendCombination = desiredCombination % numBlendModeCombos;
    int remainingCombinations = desiredCombination / numBlendModeCombos;

    int desiredColorFilterCombination = remainingCombinations % numColorFilterCombinations;
    remainingCombinations /= numColorFilterCombinations;

    int desiredMaskFilterCombination = remainingCombinations % numMaskFilterCombinations;
    remainingCombinations /= numMaskFilterCombinations;

    int desiredShaderCombination = remainingCombinations;
    SkASSERT(desiredShaderCombination < this->numShaderCombinations());

    PrecompileBase::AddToKey(keyContext, keyBuilder, fShaderOptions, desiredShaderCombination);
    PrecompileBase::AddToKey(keyContext, keyBuilder, fMaskFilterOptions,
                            desiredMaskFilterCombination);
    PrecompileBase::AddToKey(keyContext, keyBuilder, fColorFilterOptions,
                             desiredColorFilterCombination);
    PrecompileBase::AddToKey(keyContext, keyBuilder, fBlenderOptions, desiredBlendCombination);
}

void PaintOptions::buildCombinations(ShaderCodeDictionary* dict) const {
    KeyContext keyContext(dict);
    PaintParamsKeyBuilder builder(dict);

    int numCombinations = this->numCombinations();
    for (int i = 0; i < numCombinations; ++i) {
        this->createKey(keyContext, i, &builder);

        auto entry = dict->findOrCreate(&builder);

//        func(entry->uniqueID());
    }
}

//--------------------------------------------------------------------------------------------------
int CountBlenderCombos(const std::vector<sk_sp<PrecompileBlender>>& blenders) {
    bool bmBased = false;
    int numBlendCombos = 0;
    for (auto b: blenders) {
        if (b->asBlendMode().has_value()) {
            bmBased = true;
        } else {
            numBlendCombos += b->priv().numChildCombinations();
        }
    }

    if (bmBased || !numBlendCombos) {
        // If numBlendCombos is zero we will fallback to kSrcOver blending
        ++numBlendCombos;
    }

    return numBlendCombos;
}

} // namespace skgpu::graphite

#endif // SK_ENABLE_PRECOMPILE
