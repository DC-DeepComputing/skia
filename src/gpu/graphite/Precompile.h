/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Precompile_DEFINED
#define skgpu_graphite_Precompile_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_ENABLE_PRECOMPILE

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"

#include <optional>
#include <vector>

class SkRuntimeEffect;

namespace skgpu::graphite {

class KeyContext;
class PaintParamsKeyBuilder;
class PrecompileBasePriv;
class ShaderCodeDictionary;

class PrecompileBase : public SkRefCnt {
public:
    enum class Type {
        kBlender,
        kColorFilter,
        kImageFilter,
        kMaskFilter,
        kShader,
        // TODO: add others: kDrawable, kPathEffect (?!)
    };

    PrecompileBase(Type type) : fType(type) {}

    Type type() const { return fType; }

    // Provides access to functions that aren't part of the public API.
    PrecompileBasePriv priv();
    const PrecompileBasePriv priv() const;  // NOLINT(readability-const-return-type)

protected:
    template<typename T>
    static void AddToKey(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         const std::vector<sk_sp<T>>& options,
                         int desiredOption);

    // TODO: some of this should be protected NOT private
private:
    friend class PaintOptions;
    friend class PrecompileBasePriv;

    virtual int numIntrinsicCombinations() const = 0; /*{ return 1; } */
    virtual int numChildCombinations() const = 0; /*{ return 1; }*/

    int numCombinations() const {
        return this->numIntrinsicCombinations() * this->numChildCombinations();
    }

    virtual void addToKey(const KeyContext&,
                          int desiredCombination,
                          PaintParamsKeyBuilder*) const = 0;

    Type fType;
};

//--------------------------------------------------------------------------------------------------
class PrecompileShader : public PrecompileBase {
public:
    PrecompileShader() : PrecompileBase(Type::kShader) {}
};

class PrecompileMaskFilter : public PrecompileBase {
public:
    PrecompileMaskFilter() : PrecompileBase(Type::kMaskFilter) {}
};

class PrecompileColorFilter : public PrecompileBase {
public:
    PrecompileColorFilter() : PrecompileBase(Type::kColorFilter) {}
};

class PrecompileImageFilter : public PrecompileBase {
public:
    PrecompileImageFilter() : PrecompileBase(Type::kImageFilter) {}
};

class PrecompileBlender : public PrecompileBase {
public:
    PrecompileBlender() : PrecompileBase(Type::kBlender) {}

    virtual std::optional<SkBlendMode> asBlendMode() const { return {}; }

    static sk_sp<PrecompileBlender> Mode(SkBlendMode);
};

//--------------------------------------------------------------------------------------------------
class PaintOptionsPriv;

class PaintOptions {
public:
    void setShaders(SkSpan<const sk_sp<PrecompileShader>> shaders) {
        fShaderOptions.assign(shaders.begin(), shaders.end());
    }

    void setMaskFilters(SkSpan<const sk_sp<PrecompileMaskFilter>> maskFilters) {
        fMaskFilterOptions.assign(maskFilters.begin(), maskFilters.end());
    }

    void setColorFilters(SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters) {
        fColorFilterOptions.assign(colorFilters.begin(), colorFilters.end());
    }

    void setImageFilters(SkSpan<const sk_sp<PrecompileImageFilter>> imageFilters) {
        fImageFilterOptions.assign(imageFilters.begin(), imageFilters.end());
    }

    void setBlendModes(SkSpan<SkBlendMode> blendModes) {
        fBlenderOptions.reserve(blendModes.size());
        for (SkBlendMode bm : blendModes) {
            fBlenderOptions.emplace_back(PrecompileBlender::Mode(bm));
        }
    }
    void setBlenders(SkSpan<const sk_sp<PrecompileBlender>> blenders) {
        fBlenderOptions.assign(blenders.begin(), blenders.end());
    }

    // Provides access to functions that aren't part of the public API.
    PaintOptionsPriv priv();
    const PaintOptionsPriv priv() const;  // NOLINT(readability-const-return-type)

private:
    friend class PaintOptionsPriv;

    int numShaderCombinations() const;
    int numMaskFilterCombinations() const;
    int numColorFilterCombinations() const;
    // TODO: need to decompose imagefilters into component draws
    int numBlendModeCombinations() const;

    int numCombinations() const;
    // 'desiredCombination' must be less than the result of the numCombinations call
    void createKey(const KeyContext&, int desiredCombination, PaintParamsKeyBuilder*) const;
    void buildCombinations(ShaderCodeDictionary*) const;

    std::vector<sk_sp<PrecompileShader>> fShaderOptions;
    std::vector<sk_sp<PrecompileMaskFilter>> fMaskFilterOptions;
    std::vector<sk_sp<PrecompileColorFilter>> fColorFilterOptions;
    std::vector<sk_sp<PrecompileImageFilter>> fImageFilterOptions;
    std::vector<sk_sp<PrecompileBlender>> fBlenderOptions;
};

} // namespace skgpu::graphite

#endif // SK_ENABLE_PRECOMPILE

#endif // skgpu_graphite_Precompile_DEFINED
