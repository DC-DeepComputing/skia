/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PaintOptions_DEFINED
#define skgpu_graphite_precompile_PaintOptions_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"

namespace skgpu::graphite {

class PrecompileBlender;
class PrecompileColorFilter;
class PrecompileImageFilter;
class PrecompileMaskFilter;
class PrecompileShader;

enum class Coverage;
enum DrawTypeFlags : uint8_t;
enum class PrecompileImageFilterFlags : uint32_t;

class KeyContext;
class PaintOptionsPriv;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
class UniquePaintParamsID;

/** \class PaintOptions
    This is the Precompilation analog to SkPaint. It encapsulates a set of options for each
    field of the SkPaint (e.g., colorFilters, imageFilters, etc). Many of the specific details
    of an SkPaint that are irrelevant to the final compiled Pipelines are abstracted away
    (e.g., the SkPaint's color field).
*/
class PaintOptions {
public:
    /** Constructs a PaintOptions object with default values. It is equivalent to a default
     *  initialized SkPaint.

        @return  default initialized PaintOptions
    */
    PaintOptions() = default;
//    PaintOptions(const PaintOptions&) = default;
//    PaintOptions& operator=(const PaintOptions&) = default;

    void setShaders(SkSpan<const sk_sp<PrecompileShader>> shaders) {
        fShaderOptions.clear();
        fShaderOptions.push_back_n(shaders.size(), shaders.data());
    }
    SkSpan<const sk_sp<PrecompileShader>> getShaders() const {
        return SkSpan<const sk_sp<PrecompileShader>>(fShaderOptions);
    }

    void setImageFilters(SkSpan<const sk_sp<PrecompileImageFilter>> imageFilters) {
        fImageFilterOptions.clear();
        fImageFilterOptions.push_back_n(imageFilters.size(), imageFilters.data());
    }
    SkSpan<const sk_sp<PrecompileImageFilter>> getImageFilters() const {
        return SkSpan<const sk_sp<PrecompileImageFilter>>(fImageFilterOptions);
    }

    void setMaskFilters(SkSpan<const sk_sp<PrecompileMaskFilter>> maskFilters) {
        fMaskFilterOptions.clear();
        fMaskFilterOptions.push_back_n(maskFilters.size(), maskFilters.data());
    }
    SkSpan<const sk_sp<PrecompileMaskFilter>> getMaskFilters() const {
        return SkSpan<const sk_sp<PrecompileMaskFilter>>(fMaskFilterOptions);
    }

    void setColorFilters(SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters) {
        fColorFilterOptions.clear();
        fColorFilterOptions.push_back_n(colorFilters.size(), colorFilters.data());
    }
    void addColorFilter(sk_sp<PrecompileColorFilter> cf) {
        fColorFilterOptions.push_back(std::move(cf));
    }
    SkSpan<const sk_sp<PrecompileColorFilter>> getColorFilters() const {
        return SkSpan<const sk_sp<PrecompileColorFilter>>(fColorFilterOptions);
    }

    void setBlendModes(SkSpan<const SkBlendMode> blendModes) {
        fBlendModeOptions.clear();
        fBlendModeOptions.append(blendModes.size(), blendModes.data());
    }
    void addBlendMode(SkBlendMode bm) {
        fBlendModeOptions.push_back(bm);
    }
    SkSpan<const SkBlendMode> getBlendModes() const {
        return SkSpan<const SkBlendMode>(fBlendModeOptions.data(), fBlendModeOptions.size());
    }

    void setBlenders(SkSpan<const sk_sp<PrecompileBlender>> blenders);
    SkSpan<const sk_sp<PrecompileBlender>> getBlenders() const {
        return SkSpan<const sk_sp<PrecompileBlender>>(fBlenderOptions);
    }

    void setClipShaders(SkSpan<const sk_sp<PrecompileShader>> clipShaders);

    void setDither(bool dither) { fDither = dither; }
    bool isDither() const { return fDither; }

    typedef std::function<void(UniquePaintParamsID id,
                               DrawTypeFlags,
                               bool withPrimitiveBlender,
                               Coverage)> ProcessCombination;

    // Provides access to functions that aren't part of the public API.
    PaintOptionsPriv priv();
    const PaintOptionsPriv priv() const;  // NOLINT(readability-const-return-type)

private:
    friend class PaintOptionsPriv;

    int numShaderCombinations() const;
    int numColorFilterCombinations() const;
    int numBlendCombinations() const;
    int numClipShaderCombinations() const;

    int numCombinations() const;
    // 'desiredCombination' must be less than the result of the numCombinations call
    void createKey(const KeyContext&,
                   PaintParamsKeyBuilder*,
                   PipelineDataGatherer*,
                   int desiredCombination,
                   bool addPrimitiveBlender,
                   Coverage coverage) const;

    void buildCombinations(const KeyContext&,
                           PipelineDataGatherer*,
                           DrawTypeFlags drawTypes,
                           bool addPrimitiveBlender,
                           Coverage coverage,
                           const ProcessCombination& processCombination) const;

    skia_private::TArray<sk_sp<PrecompileShader>> fShaderOptions;
    skia_private::TArray<sk_sp<PrecompileColorFilter>> fColorFilterOptions;
    SkTDArray<SkBlendMode> fBlendModeOptions;
    skia_private::TArray<sk_sp<PrecompileBlender>> fBlenderOptions;
    skia_private::TArray<sk_sp<PrecompileShader>> fClipShaderOptions;

    skia_private::TArray<sk_sp<PrecompileImageFilter>> fImageFilterOptions;
    skia_private::TArray<sk_sp<PrecompileMaskFilter>> fMaskFilterOptions;

    bool fDither = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PaintOptions_DEFINED
