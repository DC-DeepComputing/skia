/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Encoding.h"

#include "include/private/SkPaintParamsKey.h"

class ShaderCodeDictionary;

SkBlendMode ExtractBlendMode(uint32_t code) {
    code &= kBlendModeMask;
    SkASSERT(code < kSkBlendModeCount);
    return static_cast<SkBlendMode>(code);
}

int BlendMode::Encode2(skgpu::ShaderCodeDictionary*, SkBackend backend, SkPaintParamsKey* key, SkBlendMode bm) {
    key->addInt1(static_cast<int>(bm));
    return 1;
}

int BlendMode::addToKeyB(skgpu::ShaderCodeDictionary* dict, SkBackend backend, SkPaintParamsKey* key, std::vector<sk_sp<skgpu::UniformData>>*, SkBlendMode bm) {
    int index = key->writeBlockHeader(CodeSnippetID::kSimpleBlendMode);

    int size = Encode2(dict, backend, key, bm);

    key->patchBlockHeader(index, CodeSnippetID::kSimpleBlendMode, size);

    return size + 1;
}

void BlendMode::Dump(const uint32_t* data, int maxIndex) {
    SkASSERT(maxIndex == 2);
    SkASSERT(CodeSnippetID::kSimpleBlendMode == ExtractCodeSnippetID(data[0]));

    SkBlendMode bm = ExtractBlendMode(data[1]);
    SkDebugf("%s ", SkBlendMode_Name(bm));
}
