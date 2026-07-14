/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ShaderErrorHandler.h"

#include "include/private/SkDebug.h"
#include "src/utils/SkShaderUtils.h"

#include <functional>
#include <string>

#if defined(UNSAFE_BUFFERS_BUILD) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
// TODO(crbug.com/40285824): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#pragma clang diagnostic pop
#endif

namespace skgpu {

ShaderErrorHandler* DefaultShaderErrorHandler() {
    class DefaultShaderErrorHandler : public ShaderErrorHandler {
    public:
        void compileError(const char* shader, const char* errors) override {
            std::string message = SkShaderUtils::BuildShaderErrorMessage(shader, errors);
            SkShaderUtils::VisitLineByLine(message, [](int, const char* lineText) {
                SkDebugf("%s\n", lineText);
            });
            SkDEBUGFAILF("Shader compilation failed!\n\n%s", message.c_str());
        }
    };

    static DefaultShaderErrorHandler gHandler;
    return &gHandler;
}

}  // namespace SkShaderUtils
