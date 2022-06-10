/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNewCombinationAPI_DEFINED
#define SkNewCombinationAPI_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"

#include <optional>
#include <vector>

//--------------------------------------------------------------------------------------------------
class SkPrecompileShader : public SkRefCnt {
public:

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
class PaintOptions {
public:
    void setShaders(std::vector<sk_sp<SkPrecompileShader>> shaders) {
        fShaders = shaders;
    }

    void setMaskFilters(std::vector<sk_sp<SkPrecompileMaskFilter>> maskFilters) {
        fMaskFilters = maskFilters;
    }

    void setColorFilters(std::vector<sk_sp<SkPrecompileColorFilter>> colorFilters) {
        fColorFilters = colorFilters;
    }

    void setImageFilters(std::vector<sk_sp<SkPrecompileImageFilter>> imageFilters) {
        fImageFilters = imageFilters;
    }

    void setBlendModes(std::vector<SkBlendMode> blendModes);
    void setBlenders(std::vector<sk_sp<SkPrecompileBlender>> blenders) {
        fBlenders = blenders;
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
    //static sk_sp<SkPrecompileShader> Empty();
    static sk_sp<SkPrecompileShader> Color();
    static sk_sp<SkPrecompileShader> Blend(std::vector<sk_sp<SkPrecompileBlender>> blenders,
                                           std::vector<sk_sp<SkPrecompileShader>> dsts,
                                           std::vector<sk_sp<SkPrecompileShader>> srcs);
    static sk_sp<SkPrecompileShader> Blend(std::vector<SkBlendMode> blendModes,
                                           std::vector<sk_sp<SkPrecompileShader>> dsts,
                                           std::vector<sk_sp<SkPrecompileShader>> srcs);
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
    // TODO: change SkMaskFilter::MakeBlur to match this and SkImageFilters?
    static sk_sp<SkPrecompileMaskFilter> Blur();

private:
    SkPrecompileMaskFilters() = delete;
};

//--------------------------------------------------------------------------------------------------
class SkPrecompileColorFilters {
public:
//    static sk_sp<SkPrecompileImageFilter> Blur();
//    static sk_sp<SkPrecompileImageFilter> Image();

private:
    SkPrecompileColorFilters() = delete;
};

//--------------------------------------------------------------------------------------------------
class SkPrecompileImageFilters {
public:
    static sk_sp<SkPrecompileImageFilter> Blur();
    static sk_sp<SkPrecompileImageFilter> Image();

private:
    SkPrecompileImageFilters() = delete;
};

#endif // SkNewCombinationAPI_DEFINED
