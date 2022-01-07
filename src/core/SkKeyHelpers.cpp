/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/KeyHelpers.h"

#include "src/core/SkPaintParamsKey.h"

namespace BlendModeBlock {

SkBlendMode ExtractFromKey(const SkPaintParamsKey& key, uint32_t headerOffset) {
    SkASSERT(key.byte(headerOffset) == (uint8_t) CodeSnippetID::kSimpleBlendMode);
    SkASSERT(key.byte(headerOffset+1) == 3);
    return static_cast<SkBlendMode>(key.byte(headerOffset+2));
}

void AddToKey(SkPaintParamsKey *key,
              std::vector <sk_sp<skgpu::UniformData>> *,
              SkBlendMode bm) {
    int blockIndex = key->writeBlockHeader(CodeSnippetID::kSimpleBlendMode);

    key->addByte(static_cast<uint8_t>(bm));

    key->patchBlockHeader(blockIndex, CodeSnippetID::kSimpleBlendMode);
}

void Dump(const SkPaintParamsKey& key, uint32_t headerOffset) {
    SkBlendMode bm = ExtractFromKey(key, headerOffset);
    SkDebugf("%s ", BlendModeToStr(bm));
}

} // namespace BlendModeBlock
