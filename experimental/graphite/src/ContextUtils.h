/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ContextUtils_DEFINED
#define skgpu_ContextUtils_DEFINED

#include "experimental/graphite/include/Context.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"
#include "include/private/SkSLString.h"

enum class CodeSnippetID : uint8_t;
class SkShaderCodeDictionary;
class SkUniform;
class SkUniformBlock;
class SkUniquePaintParamsID;

namespace skgpu {

#if 0
class PaintParams;

std::tuple<SkUniquePaintParamsID, std::unique_ptr<SkUniformBlock>> GetUniqueIDAndUniformData(
        SkShaderCodeDictionary*,
        const PaintParams&,
        SkBackend);

std::tuple<std::string, std::vector<SkSpan<const SkUniform>>> GetSkSLAndUniformDescription(
        SkShaderCodeDictionary*,
        SkUniquePaintParamsID);

std::tuple<SkUniquePaintParamsID, std::unique_ptr<SkUniformBlock>> ExtractPaintData(
        SkShaderCodeDictionary*, const PaintParams&);
#endif

} // namespace skgpu

#endif // skgpu_ContextUtils_DEFINED
