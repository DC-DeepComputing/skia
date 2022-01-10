/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKeyHelpers_DEFINED
#define SkKeyHelpers_DEFINED

#include "experimental/graphite/include/Context.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"

class SkPaintParamsKey;

namespace skgpu {
    class UniformData;
};

namespace SolidShaderBlock {

    void AddToKey(SkPaintParamsKey*,
                  std::vector<sk_sp<skgpu::UniformData>>*);
    void Dump(const SkPaintParamsKey&, uint32_t headerOffset);

} // namespace NoShaderBlock

// TODO: move this functionality to the SkLinearGradient, SkRadialGradient, etc classes
namespace GradientShaderBlocks {

    void AddToKey(SkPaintParamsKey*,
                  std::vector <sk_sp<skgpu::UniformData>> *,
                  SkShader::GradientType,
                  SkTileMode);
    void Dump(const SkPaintParamsKey&, uint32_t headerOffset);

} // namespace GradientShaderBlocks

namespace BlendModeBlock {

    void AddToKey(SkPaintParamsKey*,
                  std::vector<sk_sp<skgpu::UniformData>>*,
                  SkBlendMode);
    void Dump(const SkPaintParamsKey&, uint32_t headerOffset);

} // namespace BlendModeBlock

SkPaintParamsKey CreateKey(skgpu::ShaderCombo::ShaderType, SkTileMode, SkBlendMode);

#endif // SkKeyHelpers_DEFINED
