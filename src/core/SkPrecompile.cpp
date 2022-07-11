/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPrecompile.h"

#include "src/core/SkKeyContext.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkPrecompileBasePriv.h"
#include "src/core/SkShaderCodeDictionary.h"

//--------------------------------------------------------------------------------------------------
void SkPrecompileBase::addToKey(const SkKeyContext& keyContext,
                                int desiredCombination,
                                SkPaintParamsKeyBuilder* keyBuilder) {
    SkASSERT(desiredCombination < this->numCombinations1());

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

//--------------------------------------------------------------------------------------------------
SkPrecompileShader::SkPrecompileShader(int numGraphiteIntrinsicCombinations,
                                       BeginBlockMethod graphiteBeginBlock)
        : SkPrecompileBase(Type::kShader) {
    fFoo[static_cast<int>(SkBackend::kGraphite)].fNumIntrinsicCombinations = numGraphiteIntrinsicCombinations;
    fFoo[static_cast<int>(SkBackend::kGraphite)].fBeginBlock = graphiteBeginBlock;
}

int SkPrecompileShader::numIntrinsicCombinations(SkBackend backend) const {
    return fFoo[static_cast<int>(backend)].fNumIntrinsicCombinations;
}

void SkPrecompileShader::beginBlock(const SkKeyContext& keyContext,
                                    int intrinsicCombination,
                                    SkPaintParamsKeyBuilder* builder) const {
    fFoo[static_cast<int>(builder->backend())].fBeginBlock(keyContext,
                                                           intrinsicCombination,
                                                           builder);
}

//--------------------------------------------------------------------------------------------------
int SkPaintOptions::numShaderCombinations() const {
    int numShaderCombinations = 0;
    for (const sk_sp<SkPrecompileShader>& s : fShaderOptions) {
        numShaderCombinations += s->priv().numCombinations1();
    }

    // If no shader option is specified we will add a solid color shader option
    return numShaderCombinations ? numShaderCombinations : 1;
}

int SkPaintOptions::numMaskFilterCombinations() const {
    int numMaskFilterCombinations = 0;
    for (const sk_sp<SkPrecompileMaskFilter>& mf : fMaskFilterOptions) {
        numMaskFilterCombinations += mf->priv().numCombinations1();
    }

    // If no mask filter options are specified we will use the geometry's coverage
    return numMaskFilterCombinations ? numMaskFilterCombinations : 1;
}

int SkPaintOptions::numColorFilterCombinations() const {
    int numColorFilterCombinations = 0;
    for (const sk_sp<SkPrecompileColorFilter>& cf : fColorFilterOptions) {
        numColorFilterCombinations += cf->priv().numCombinations1();
    }

    // If no color filter options are specified we will use the unmodified result color
    return numColorFilterCombinations ? numColorFilterCombinations : 1;
}

int SkPaintOptions::numBlendModeCombinations() const {
    int numBlenderCombinations = 0;
    for (const sk_sp<SkPrecompileBlender>& b : fBlenderOptions) {
        numBlenderCombinations += b->priv().numCombinations1();
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

void SkPaintOptions::createKey(const SkKeyContext& keyContext,
                               int desiredCombination,
                               SkPaintParamsKeyBuilder* keyBuilder) const {
    SkDEBUGCODE(keyBuilder->checkReset();)
    SkASSERT(desiredCombination < this->numCombinations());

    int numShaderCombinations = this->numShaderCombinations();
    int numMaskFilterCombinations = this->numMaskFilterCombinations();
    int numColorFilterCombinations = this->numColorFilterCombinations();
    int numBlendModeCombos = this->numBlendModeCombinations();

    int desiredBlendCombination = desiredCombination % numBlendModeCombos;
    int remainingCombinations = desiredCombination / numBlendModeCombos;

    int desiredColorFilterCombination = remainingCombinations % numColorFilterCombinations;
    remainingCombinations /= numColorFilterCombinations;

    int desiredMaskFilterCombination = remainingCombinations % numMaskFilterCombinations;
    remainingCombinations /= numMaskFilterCombinations;

    int desiredShaderCombination = remainingCombinations;

    for (const sk_sp<SkPrecompileShader>& shaderOption : fShaderOptions) {
        if (desiredShaderCombination < shaderOption->priv().numCombinations1()) {
            shaderOption->addToKey(keyContext, desiredShaderCombination, keyBuilder);
            break;
        }

        desiredShaderCombination -= shaderOption->priv().numCombinations1();
    }

    for (const sk_sp<SkPrecompileBlender>& blenderOption : fBlenderOptions) {
        if (desiredBlendCombination < blenderOption->priv().numCombinations1()) {
            blenderOption->addToKey(keyContext, desiredBlendCombination, keyBuilder);
        }

        desiredBlendCombination -= blenderOption->priv().numCombinations1();
    }
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
