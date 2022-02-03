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

enum class CodeSnippetID : uint8_t;
class SkShaderCodeDictionary;
class SkUniform;
class SkUniformBlock;
class SkUniquePaintParamsID;

namespace skgpu {

class PaintParams;

std::tuple<SkUniquePaintParamsID, std::unique_ptr<SkUniformBlock>> ExtractPaintData(
        SkShaderCodeDictionary*, const PaintParams&);

SkSpan<const SkUniform> GetUniforms(CodeSnippetID);

class ShaderInfo {
public:
    void add(SkSpan<const SkUniform> uniforms, const char* name, const char* snippet) {
        fUniforms.push_back(uniforms);
        fNames.push_back(name);
        fPreamble.push_back(snippet);
    }

    void setWritesColor() { fWritesColor = true; }
    bool writesColor() const { return fWritesColor; }

    std::string flatten() const;

private:
    std::vector<SkSpan<const SkUniform>> fUniforms;
    std::vector<const char*> fNames;
    std::vector<const char*> fPreamble;
    bool fWritesColor = false;
};

void GetShaderInfo(SkShaderCodeDictionary*, SkUniquePaintParamsID, ShaderInfo*);

} // namespace skgpu

#endif // skgpu_ContextUtils_DEFINED
