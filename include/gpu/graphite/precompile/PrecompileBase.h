/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileBase_DEFINED
#define skgpu_graphite_precompile_PrecompileBase_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"

namespace skgpu::graphite {

class KeyContext;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
class PrecompileBasePriv;

class PrecompileBase : public SkRefCnt {
public:
    enum class Type {
        kBlender,
        kColorFilter,
        kImageFilter,
        kMaskFilter,
        kShader,
    };

    Type type() const { return fType; }

    int numCombinations() const {
        return this->numIntrinsicCombinations() * this->numChildCombinations();
    }

    // Provides access to functions that aren't part of the public API.
    PrecompileBasePriv priv();
    const PrecompileBasePriv priv() const;  // NOLINT(readability-const-return-type)

protected:
    PrecompileBase(Type type) : fType(type) {}

    virtual int numIntrinsicCombinations() const { return 1; }
    virtual int numChildCombinations() const { return 1; }

    // This returns the desired option along with the child options.
    template<typename T>
    static std::pair<sk_sp<T>, int> SelectOption(SkSpan<const sk_sp<T>> options,
                                                 int desiredOption);

    // In general, derived classes should use AddToKey to select the desired child option from
    // a span and then have it added to the key with its reduced/nested child option.
    template<typename T>
    static void AddToKey(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         SkSpan<const sk_sp<T>> options,
                         int desiredOption);

private:
    friend class PaintOptions;
    friend class PrecompileBasePriv;

    virtual bool isALocalMatrixShader() const { return false; }

    virtual void addToKey(const KeyContext&,
                          PaintParamsKeyBuilder*,
                          PipelineDataGatherer*,
                          int desiredCombination) const = 0;

    Type fType;
};

}

#endif // skgpu_graphite_precompile_PrecompileBase_DEFINED
