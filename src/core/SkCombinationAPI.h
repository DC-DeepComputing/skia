/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCombinationAPI_DEFINED
#define SkCombinationAPI_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"

#include <optional>
#include <vector>

class SkRuntimeEffect;

//--------------------------------------------------------------------------------------------------
class SkPrecompileShader : public SkRefCnt {
public:
    // TODO: add a type enum to each of these base classes
};

class SkPrecompileMaskFilter : public SkRefCnt {
public:

};

class SkPrecompileColorFilter : public SkRefCnt {
public:

};

class SkPrecompileImageFilter : public SkRefCnt {
public:

};

class SkPrecompileBlender : public SkRefCnt {
public:
    virtual std::optional<SkBlendMode> asBlendMode() const { return {}; }

    static sk_sp<SkPrecompileBlender> Mode(SkBlendMode blendMode);

};

//--------------------------------------------------------------------------------------------------
class SkPaintOptions {
public:
    void setShaders(SkSpan<const sk_sp<SkPrecompileShader>> shaders) {
        fShaders.assign(shaders.begin(), shaders.end());
    }

    void setMaskFilters(SkSpan<sk_sp<SkPrecompileMaskFilter>> maskFilters) {
        fMaskFilters.assign(maskFilters.begin(), maskFilters.end());
    }

    void setColorFilters(SkSpan<sk_sp<SkPrecompileColorFilter>> colorFilters) {
        fColorFilters.assign(colorFilters.begin(), colorFilters.end());
    }

    void setImageFilters(SkSpan<sk_sp<SkPrecompileImageFilter>> imageFilters) {
        fImageFilters.assign(imageFilters.begin(), imageFilters.end());
    }

    void setBlendModes(SkSpan<SkBlendMode> blendModes) {
        fBlenders.reserve(blendModes.size());
        for (SkBlendMode bm : blendModes) {
            fBlenders.emplace_back(SkPrecompileBlender::Mode(bm));
        }
    }
    void setBlenders(SkSpan<sk_sp<SkPrecompileBlender>> blenders) {
        fBlenders.assign(blenders.begin(), blenders.end());
    }

private:
    std::vector<sk_sp<SkPrecompileShader>> fShaders;
    std::vector<sk_sp<SkPrecompileMaskFilter>> fMaskFilters;
    std::vector<sk_sp<SkPrecompileColorFilter>> fColorFilters;
    std::vector<sk_sp<SkPrecompileImageFilter>> fImageFilters;
    std::vector<sk_sp<SkPrecompileBlender>> fBlenders;
};

//--------------------------------------------------------------------------------------------------
class SkPrecompileShaders {
public:
    //TODO: Empty?
    static sk_sp<SkPrecompileShader> Color();
    static sk_sp<SkPrecompileShader> Blend(SkSpan<sk_sp<SkPrecompileBlender>> blenders,
                                           SkSpan<const sk_sp<SkPrecompileShader>> dsts,
                                           SkSpan<const sk_sp<SkPrecompileShader>> srcs);
    static sk_sp<SkPrecompileShader> Blend(SkSpan<SkBlendMode> blendModes,
                                           SkSpan<const sk_sp<SkPrecompileShader>> dsts,
                                           SkSpan<const sk_sp<SkPrecompileShader>> srcs);
    // TODO: add an SkShaders::Image to match this and SkImageFilters?
    static sk_sp<SkPrecompileShader> Image();

private:
    SkPrecompileShaders() = delete;
};

//--------------------------------------------------------------------------------------------------
class SkPrecompileGradientShader {
public:
    static sk_sp<SkPrecompileShader> MakeLinear();
    static sk_sp<SkPrecompileShader> MakeRadial();
    static sk_sp<SkPrecompileShader> MakeTwoPointConical();
    static sk_sp<SkPrecompileShader> MakeSweep();

private:
    SkPrecompileGradientShader() = delete;
};

//--------------------------------------------------------------------------------------------------
class SkPrecompileMaskFilters {
public:
    // TODO: change SkMaskFilter::MakeBlur to match this and SkImageFilters::Blur
    static sk_sp<SkPrecompileMaskFilter> Blur();

private:
    SkPrecompileMaskFilters() = delete;
};

//--------------------------------------------------------------------------------------------------
class SkPrecompileColorFilters {
public:
    static sk_sp<SkPrecompileColorFilter> Matrix();
    // TODO: Compose, Blend, HSLAMatrix, LinearToSRGBGamma, SRGBToLinearGamma, Lerp

private:
    SkPrecompileColorFilters() = delete;
};

//--------------------------------------------------------------------------------------------------
class SkPrecompileImageFilters {
public:
    static sk_sp<SkPrecompileImageFilter> Blur();
    static sk_sp<SkPrecompileImageFilter> Image();
    // TODO: AlphaThreshold, Arithmetic, Blend (2 kinds), ColorFilter, Compose, DisplacementMap,
    // DropShadow, DropShadowOnly, Magnifier, MatrixConvolution, MatrixTransform, Merge, Offset,
    // Picture, Runtime, Shader, Tile, Dilate, Erode, DistantLitDiffuse, PointLitDiffuse,
    // SpotLitDiffuse, DistantLitSpecular, PointLitSpecular, SpotLitSpecular

private:
    SkPrecompileImageFilters() = delete;
};

// TODO: move these to be on SkRuntimeEffect to parallel makeShader, makeColorFilter and
// makeBlender
sk_sp<SkPrecompileShader> MakeShader(sk_sp<SkRuntimeEffect> effect,
                                     SkSpan<const sk_sp<SkPrecompileShader>> children);

sk_sp<SkPrecompileColorFilter> MakePrecompileColorFilter(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const sk_sp<SkPrecompileColorFilter>> children = {});

sk_sp<SkPrecompileBlender> MakePrecompileBlender(sk_sp<SkRuntimeEffect> effect,
                                                 SkSpan<const sk_sp<SkPrecompileBlender>> children);

#endif // SkCombinationAPI_DEFINED
