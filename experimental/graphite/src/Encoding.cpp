/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Encoding.h"

#include "include/private/SkPaintParamsKey.h"

class SkShaderCodeDictionary;

#include "src/core/SkKeyHelpers.h"

void BlendMode::addToKeyB(SkShaderCodeDictionary* dict,
                          SkBackend backend,
                          SkPaintParamsKey* key,
                          SkUniformBlock* uniformBlock,
                          SkBlendMode bm) {
    BlendModeBlock::AddToKey(dict, backend, key, uniformBlock, bm);
}
