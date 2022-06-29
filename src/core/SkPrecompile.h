/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPrecompile_DEFINED
#define SkPrecompile_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_ENABLE_PRECOMPILE

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"

#include <optional>
#include <vector>

class SkKeyContext;
class SkPaintParamsKeyBuilder;
class SkPrecompileBasePriv;
class SkRuntimeEffect;
class SkShaderCodeDictionary;
class SkUniquePaintParamsID;

class SkPrecompileBase : public SkRefCnt {
public:
    enum class Type {
        kBlender,
        kColorFilter,
        kImageFilter,
        kMaskFilter,
        kShader,
        // TODO: add others: kDrawable, kPathEffect (?!)
    };

    SkPrecompileBase(Type type) : fType(type) {}

    Type type() const { return fType; }

    // Provides access to functions that aren't part of the public API.
    SkPrecompileBasePriv priv();
    const SkPrecompileBasePriv priv() const;  // NOLINT(readability-const-return-type)

private:
    friend class SkPrecompileBasePriv;

    virtual int numIntrinsicCombinations() const = 0;

    virtual int numChildCombinations() const = 0;

//        int numChildCombinations = 1;
//        for (int i = 0; i < fNumSlots; ++i) {
//            numChildCombinations *= fSlots[i].numCombinations();
//        }
//        return numChildCombinations;
//    }

    int numCombinations1() const {
        return this->numIntrinsicCombinations() * this->numChildCombinations();
    }

    Type fType;
};

//--------------------------------------------------------------------------------------------------
class SkPrecompileShader : public SkPrecompileBase {
public:
    SkPrecompileShader() : SkPrecompileBase(Type::kShader) {}

private:
    // TODO: remove these defaults in favor of having the specific type of shader
    // impact the result(s).
    int numIntrinsicCombinations() const override { return 1; }
    int numChildCombinations() const override { return 1; }
};

class SkPrecompileMaskFilter : public SkPrecompileBase {
public:
    SkPrecompileMaskFilter() : SkPrecompileBase(Type::kMaskFilter) {}

private:
    // TODO: remove these defaults in favor of having the specific type of shader
    // impact the result(s).
    int numIntrinsicCombinations() const override { return 1; }
    int numChildCombinations() const override { return 1; }
};

class SkPrecompileColorFilter : public SkPrecompileBase {
public:
    SkPrecompileColorFilter() : SkPrecompileBase(Type::kColorFilter) {}

private:
    // TODO: remove these defaults in favor of having the specific type of shader
    // impact the result(s).
    int numIntrinsicCombinations() const override { return 1; }
    int numChildCombinations() const override { return 1; }
};

class SkPrecompileImageFilter : public SkPrecompileBase {
public:
    SkPrecompileImageFilter() : SkPrecompileBase(Type::kImageFilter) {}

private:
    // TODO: remove these defaults in favor of having the specific type of shader
    // impact the result(s).
    int numIntrinsicCombinations() const override { return 1; }
    int numChildCombinations() const override { return 1; }
};

class SkPrecompileBlender : public SkPrecompileBase {
public:
    SkPrecompileBlender() : SkPrecompileBase(Type::kBlender) {}

    virtual std::optional<SkBlendMode> asBlendMode() const { return {}; }

    static sk_sp<SkPrecompileBlender> Mode(SkBlendMode blendMode);
};

//--------------------------------------------------------------------------------------------------
class SkPaintOptions {
public:
    void setShaders(SkSpan<const sk_sp<SkPrecompileShader>> shaders) {
        fShaderOptions.assign(shaders.begin(), shaders.end());
    }

    void setMaskFilters(SkSpan<const sk_sp<SkPrecompileMaskFilter>> maskFilters) {
        fMaskFilterOptions.assign(maskFilters.begin(), maskFilters.end());
    }

    void setColorFilters(SkSpan<const sk_sp<SkPrecompileColorFilter>> colorFilters) {
        fColorFilterOptions.assign(colorFilters.begin(), colorFilters.end());
    }

    void setImageFilters(SkSpan<const sk_sp<SkPrecompileImageFilter>> imageFilters) {
        fImageFilterOptions.assign(imageFilters.begin(), imageFilters.end());
    }

    void setBlendModes(SkSpan<SkBlendMode> blendModes) {
        fBlenderOptions.reserve(blendModes.size());
        for (SkBlendMode bm : blendModes) {
            fBlenderOptions.emplace_back(SkPrecompileBlender::Mode(bm));
        }
    }
    void setBlenders(SkSpan<const sk_sp<SkPrecompileBlender>> blenders) {
        fBlenderOptions.assign(blenders.begin(), blenders.end());
    }

    void buildCombinations(SkShaderCodeDictionary*,
                           const std::function<void(SkUniquePaintParamsID)>& func) const;

private:
    int numShaderCombinations() const;
    int numMaskFilterCombinations() const;
    int numColorFilterCombinations() const;
    // TODO: need to decompose imagefilters into component draws
    int numBlendModeCombinations() const;

    int numCombinations() const;

    // 'desiredCombination' must be less than the result of the numCombinations call
    void createKey(const SkKeyContext&, int desiredCombination, SkPaintParamsKeyBuilder*) const;

    std::vector<sk_sp<SkPrecompileShader>> fShaderOptions;
    std::vector<sk_sp<SkPrecompileMaskFilter>> fMaskFilterOptions;
    std::vector<sk_sp<SkPrecompileColorFilter>> fColorFilterOptions;
    std::vector<sk_sp<SkPrecompileImageFilter>> fImageFilterOptions;
    std::vector<sk_sp<SkPrecompileBlender>> fBlenderOptions;
};

#endif // SK_ENABLE_PRECOMPILE

#endif // SkPrecompile_DEFINED
