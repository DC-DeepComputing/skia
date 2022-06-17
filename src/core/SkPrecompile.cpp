/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPrecompile.h"

void SkPaintOptions::buildCombinations(SkShaderCodeDictionary*,
                                       const std::function<void(SkUniquePaintParamsID)>& func) const {

    for (sk_sp<SkPrecompileShader> s : fShaders) {


    }

}
