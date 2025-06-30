/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PaintOptionsBuilder_DEFINED
#define PaintOptionsBuilder_DEFINED

#include "include/gpu/graphite/precompile/PaintOptions.h"

namespace PaintOptionUtils {

enum ImgColorInfo {
    kAlpha,
    kAlphaSRGB,
    kPremul,
    kSRGB,
};

enum ImgOptions {
    kNone,
    kClamp,
    kRepeat,
};

enum YUVSamplingOptions {
    kNoCubic,
    kHWAndShader
};

enum GradientOptions {
    kSmall,
    kComplex,
};

class Builder {
public:
    Builder() {}

    // Shaders
    Builder& hwImg(ImgColorInfo ci, ImgOptions options = kNone);
    Builder& yuv(YUVSamplingOptions options);
    Builder& linearGrad(GradientOptions options);

    // ColorFilters
    Builder& matrixCF();
    Builder& porterDuffCF();

    // Blendmodes/Blenders
    Builder& clear()   { return this->addBlendMode(SkBlendMode::kClear);   }
    Builder& dstIn()   { return this->addBlendMode(SkBlendMode::kDstIn);   }
    Builder& src()     { return this->addBlendMode(SkBlendMode::kSrc);     }
    Builder& srcOver() { return this->addBlendMode(SkBlendMode::kSrcOver); }

    // Misc settings
    Builder& transparent() { fPaintOptions.setPaintColorIsOpaque(false); return *this; }
    Builder& dither()      { fPaintOptions.setDither(true);              return *this; }

    operator skgpu::graphite::PaintOptions() const { return fPaintOptions; }

private:
    skgpu::graphite::PaintOptions fPaintOptions;

    Builder& addBlendMode(SkBlendMode bm) {
        fPaintOptions.addBlendMode(bm);
        return *this;
    }
};

} // namespace PaintOptionUtils

#endif // PaintOptionsBuilder_DEFINED
