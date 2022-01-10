/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyHelpers.h"

#include "src/core/SkPaintParamsKey.h"

//--------------------------------------------------------------------------------------------------
namespace SolidShaderBlock {

void AddToKey(SkPaintParamsKey* key,
              std::vector<sk_sp<skgpu::UniformData>>*) {
    int blockIndex = key->writeBlockHeader(CodeSnippetID::kNoneShader);
    key->patchBlockHeader(blockIndex, CodeSnippetID::kNoneShader);
}

void Dump(const SkPaintParamsKey& key, uint32_t headerOffset) {
    SkASSERT(key.byte(headerOffset) == (uint8_t) CodeSnippetID::kNoneShader);
    SkASSERT(key.byte(headerOffset+1) == 2);

    SkDebugf("kNoneShader\n");
}

} // namespace SolidShaderBlock

//--------------------------------------------------------------------------------------------------
namespace GradientShaderBlocks {

void AddToKey(SkPaintParamsKey *key,
              std::vector <sk_sp<skgpu::UniformData>> *,
              SkShader::GradientType type,
              SkTileMode tm) {

    CodeSnippetID id = CodeSnippetID::kNoneShader;
    switch (type) {
        case SkShader::kLinear_GradientType:
            id = CodeSnippetID::kLinearGradientShader;
            break;
        case SkShader::kRadial_GradientType:
            id = CodeSnippetID::kRadialGradientShader;
            break;
        case SkShader::kSweep_GradientType:
            id = CodeSnippetID::kSweepGradientShader;
            break;
        case SkShader::GradientType::kConical_GradientType:
            id = CodeSnippetID::kConicalGradientShader;
            break;
        case SkShader::GradientType::kColor_GradientType:
        case SkShader::GradientType::kNone_GradientType:
        default:
            SkASSERT(0);
            break;
    }

    int blockIndex = key->writeBlockHeader(id);

    key->addByte(static_cast<uint8_t>(tm));

    key->patchBlockHeader(blockIndex, id);
}

void Dump(const SkPaintParamsKey& key, uint32_t headerOffset) {

}

} // namespace GradientShaderBlocks

//--------------------------------------------------------------------------------------------------
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

    SkDebugf("kSimpleBlendMode: %d\n", bm); // BlendModeToStr(bm));
}

//--------------------------------------------------------------------------------------------------
SkPaintParamsKey CreateKey(skgpu::ShaderCombo::ShaderType s, SkTileMode tm, SkBlendMode bm) {
    SkPaintParamsKey key;

    switch (s) {
        case skgpu::ShaderCombo::ShaderType::kNone:
            break;
        case skgpu::ShaderCombo::ShaderType::kSolidColor:
            SolidShaderBlock::AddToKey(&key, nullptr);
            break;
        case skgpu::ShaderCombo::ShaderType::kLinearGradient:
            GradientShaderBlocks::AddToKey(&key, nullptr, SkShader::kLinear_GradientType, tm);
            break;
        case skgpu::ShaderCombo::ShaderType::kRadialGradient:
            GradientShaderBlocks::AddToKey(&key, nullptr, SkShader::kRadial_GradientType, tm);
            break;
        case skgpu::ShaderCombo::ShaderType::kSweepGradient:
            GradientShaderBlocks::AddToKey(&key, nullptr, SkShader::kSweep_GradientType, tm);
            break;
        case skgpu::ShaderCombo::ShaderType::kConicalGradient:
            GradientShaderBlocks::AddToKey(&key, nullptr, SkShader::kConical_GradientType, tm);
            break;
    }

    BlendModeBlock::AddToKey(&key, nullptr, bm);
    return key;
}

} // namespace BlendModeBlock
