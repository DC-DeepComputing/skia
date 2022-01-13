/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Encoding.h"

#include "include/private/SkPaintParamsKey.h"

class ShaderCodeDictionary;

#include "src/core/SkKeyHelpers.h"

int BlendMode::addToKeyB(SkShaderCodeDictionary* dict, SkBackend backend, SkPaintParamsKey* key, std::vector<sk_sp<skgpu::UniformData>>*, SkBlendMode bm) {

    BlendModeBlock::AddToKey(key, bm);

    return 0;
}
