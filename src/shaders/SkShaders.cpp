/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShader.h"

#include "include/core/SkColorSpace.h"
#include "src/shaders/SkColorShader.h"
#include "src/shaders/SkComposeShader.h"
#include "src/shaders/SkEmptyShader.h"

sk_sp<SkShader> SkShaders::Empty() { return sk_make_sp<SkEmptyShader>(); }

sk_sp<SkShader> SkShaders::Color(SkColor color) { return sk_make_sp<SkColorShader>(color); }

sk_sp<SkShader> SkShaders::Color(const SkColor4f& color, sk_sp<SkColorSpace> space) {
    if (!SkScalarsAreFinite(color.vec(), 4)) {
        return nullptr;
    }
    return sk_make_sp<SkColor4Shader>(color, std::move(space));
}

sk_sp<SkShader> SkShaders::Blend(SkBlendMode mode, sk_sp<SkShader> dst, sk_sp<SkShader> src) {
    if (!src || !dst) {
        return nullptr;
    }
    switch (mode) {
        case SkBlendMode::kClear: return Color(0);
        case SkBlendMode::kDst:   return dst;
        case SkBlendMode::kSrc:   return src;
        default: break;
    }
    return sk_sp<SkShader>(new SkShader_Blend(mode, std::move(dst), std::move(src)));
}

sk_sp<SkShader> SkShaders::Blend(sk_sp<SkBlender> blender, sk_sp<SkShader> dst, sk_sp<SkShader> src) {
    if (!src || !dst) {
        return nullptr;
    }
    if (!blender) {
        return SkShaders::Blend(SkBlendMode::kSrcOver, std::move(dst), std::move(src));
    }
    return sk_sp<SkShader>(new SkShader_Blend(std::move(blender), std::move(dst), std::move(src)));
}
