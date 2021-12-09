/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkCode.h"

void SkCode::dump() const {
    SkASSERT(this->numInts() == 2);

    SkDebugf("Combo: ");
    int iFoo = fInts[0] & kShaderCodeMask;
    switch (iFoo) {
        case ShaderCode::kNone_ShaderCode:
            NoShaderCombo::Dump(*this);
            break;
        case ShaderCode::kImage_ShaderCode:
            ImageShaderCombinations::Dump(*this);
            break;
        case ShaderCode::kGradient_ShaderCode:
            GradientCombinations::Dump(*this);
            break;
    }

    SkBlendMode bm = (SkBlendMode) fInts[1];
    SkDebugf(" %s\n", BMToStr(bm));
}
