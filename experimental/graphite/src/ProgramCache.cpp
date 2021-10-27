/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ProgramCache.h"

#include "experimental/graphite/src/ContextUtils.h"

namespace skgpu {

static const uint32_t kSolidColorFS = 1;
static const uint32_t kGradientFS   = 2;

uint32_t ProgramCache::findOrCreateProgram(Combination c) {
    if (c.fShaderType == ShaderCombo::ShaderType::kNone) {
        return kSolidColorFS;
    } else {
        return kGradientFS;
    }
}

} // namespace skgpu
