/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyHelpers.h"

#include "src/core/SkPaintParamsKey.h"

//--------------------------------------------------------------------------------------------------
namespace StencilOnlyBlock {

void AddToKey(SkPaintParamsKey* key,
              std::vector<sk_sp<skgpu::UniformData>>*) {
    int blockIndex = key->writeBlockHeader(CodeSnippetID::kStencilOnlyDraw);
    key->patchBlockHeader(blockIndex, CodeSnippetID::kStencilOnlyDraw);
}

void Dump(const SkPaintParamsKey& key, uint32_t headerOffset) {
    SkASSERT(key.byte(headerOffset) == (uint8_t) CodeSnippetID::kStencilOnlyDraw);
    SkASSERT(key.byte(headerOffset+1) == 2);

    SkDebugf("kStencilOnlyDraw\n");
}

} // namespace StencilOnlyBlock

//--------------------------------------------------------------------------------------------------
namespace SolidShaderBlock {

void AddToKey(SkPaintParamsKey* key,
              std::vector<sk_sp<skgpu::UniformData>>*) {
    int blockIndex = key->writeBlockHeader(CodeSnippetID::kNoneShader1);
    key->patchBlockHeader(blockIndex, CodeSnippetID::kNoneShader1);
}

void Dump(const SkPaintParamsKey& key, uint32_t headerOffset) {
    SkASSERT(key.byte(headerOffset) == (uint8_t) CodeSnippetID::kNoneShader1);
    SkASSERT(key.byte(headerOffset+1) == 2);

    SkDebugf("kNoneShader1\n");
}

} // namespace SolidShaderBlock

//--------------------------------------------------------------------------------------------------
namespace GradientShaderBlocks {

std::pair<CodeSnippetID, SkTileMode> ExtractFromKey(const SkPaintParamsKey& key,
                                                    uint32_t headerOffset) {
    CodeSnippetID id = static_cast<CodeSnippetID>(key.byte(headerOffset));

    SkASSERT(id == CodeSnippetID::kLinearGradientShader ||
             id == CodeSnippetID::kRadialGradientShader ||
             id == CodeSnippetID::kSweepGradientShader ||
             id == CodeSnippetID::kConicalGradientShader);
    SkASSERT(key.byte(headerOffset+1) == 3);

    SkTileMode tm = static_cast<SkTileMode>(key.byte(headerOffset+2));

    return { id, tm };
}

void AddToKey(SkPaintParamsKey *key,
              std::vector <sk_sp<skgpu::UniformData>> *,
              SkShader::GradientType type,
              SkTileMode tm) {

    CodeSnippetID id = CodeSnippetID::kNoneShader1;
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
    auto [id, tm] =  ExtractFromKey(key, headerOffset);

    switch (id) {
        case CodeSnippetID::kLinearGradientShader:
            SkDebugf("kLinearGradientShader: %d\n", tm);
            break;
        case CodeSnippetID::kRadialGradientShader:
            SkDebugf("kRadialGradientShader: %d\n", tm);
            break;
        case CodeSnippetID::kSweepGradientShader:
            SkDebugf("kSweepGradientShader: %d\n", tm);
            break;
        case CodeSnippetID::kConicalGradientShader:
            SkDebugf("kConicalGradientShader: %d\n", tm);
            break;
        default:
            SkDebugf("Unknown!!\n");
            break;
    }
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
