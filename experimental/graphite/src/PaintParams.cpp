/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/PaintParams.h"

#include "include/core/SkShader.h"
#include "include/private/SkPaintParamsKey.h"

#include "experimental/graphite/include/Encoding.h"

namespace skgpu {

PaintParams::PaintParams(const SkColor4f& color,
                         SkBlendMode blendMode,
                         sk_sp<SkShader> shader)
        : fColor(color)
        , fBlendMode(blendMode)
        , fShader(std::move(shader)) {}

PaintParams::PaintParams(const SkPaint& paint)
        : fColor(paint.getColor4f())
        , fBlendMode(paint.getBlendMode_or(SkBlendMode::kSrcOver))
        , fShader(paint.refShader()) {}

PaintParams::PaintParams(const PaintParams& other) = default;
PaintParams::~PaintParams() = default;
PaintParams& PaintParams::operator=(const PaintParams& other) = default;

sk_sp<SkShader> PaintParams::refShader() const { return fShader; }

void PaintParams::toCodeAndUnis(ShaderCodeDictionary* dict,
                                Mapping mapping,
                                SkPaintParamsKey* key,
                                std::vector<sk_sp<UniformData>>* uniforms) const {

    if (fShader) {
        fShader->addToKeyS(dict, mapping, key, uniforms);
    }
//    if (fColorFilter) {
//        fColorFilter->toCodeCF(dict, mapping, code);
//    }
//    if (fBlender) {
//        fBlender->addToKeyB(dict, mapping, code);
//    } else

    {
        BlendMode::addToKeyB(dict, mapping, key, uniforms, fBlendMode);
    }

    SkASSERT(key->sizeInBytes() > 0);
}

} // namespace skgpu
