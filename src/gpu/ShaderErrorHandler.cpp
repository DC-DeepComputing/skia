/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ShaderErrorHandler.h"

#include "src/utils/SkShaderUtils.h"

#include <string_view>

namespace skgpu {

ShaderErrorHandler* DefaultShaderErrorHandler() {
    class DefaultShaderErrorHandler : public ShaderErrorHandler {
    public:
        void compileError(std::string_view shader, std::string_view errors) override {
            std::string message = SkShaderUtils::BuildShaderErrorMessage(shader, errors);
            SkShaderUtils::VisitLineByLine(message, [](int, std::string_view lineText) {
                SkDebugf("%.*s\n", (int)lineText.size(), lineText.data());
            });
            SkDEBUGFAIL("Shader compilation failed!");
        }
    };

    static DefaultShaderErrorHandler gHandler;
    return &gHandler;
}

}  // namespace SkShaderUtils
