/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkCode.h"

#include "experimental/graphite/include/Context.h"

void SkCode::dump() const {
    SkASSERT(this->numInts() == 2);

    SkDebugf("Combo: ");
    int iFoo = fInts[0] & skgpu::kShaderCodeMask;
    switch (iFoo) {
        case skgpu::ShaderCode::kNone_ShaderCode:
            skgpu::NoShaderCombo::Dump(*this);
            break;
        case skgpu::ShaderCode::kImage_ShaderCode:
            skgpu::ImageShaderCombinations::Dump(*this);
            break;
        case skgpu::ShaderCode::kGradient_ShaderCode:
            skgpu::GradientCombinations::Dump(*this);
            break;
    }

    SkBlendMode bm = (SkBlendMode) fInts[1];
    SkDebugf(" %s\n", skgpu::BMToStr(bm));
}
