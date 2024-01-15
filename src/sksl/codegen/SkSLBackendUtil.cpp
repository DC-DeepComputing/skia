/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLBackendUtil.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/private/base/SkDebug.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"  // IWYU pragma: keep
#include "src/utils/SkShaderUtils.h"

namespace SkSL {
enum class ProgramKind : int8_t;

bool ToBackend(Compiler* compiler,
               bool (*toBackend)(Compiler*, Program&, std::string*),
               const char* backendLabel,
               const std::string& sksl,
               ProgramKind programKind,
               const ProgramSettings& settings,
               std::string* output,
               Program::Interface* outInterface,
               skgpu::ShaderErrorHandler* errorHandler) {
#ifdef SK_DEBUG
    std::string src = SkShaderUtils::PrettyPrint(sksl);
#else
    const std::string& src = sksl;
#endif
    std::unique_ptr<Program> program = compiler->convertProgram(programKind, src, settings);
    if (!program || !toBackend(compiler, *program, output)) {
        errorHandler->compileError(
                src.c_str(), compiler->errorText().c_str(), /*shaderWasCached=*/false);
        return false;
    }

#if defined(SK_PRINT_SKSL_SHADERS)
    const bool kPrintSkSL = true;
#else
    const bool kPrintSkSL = false;
#endif
#if defined(SK_PRINT_NATIVE_SHADERS)
    const bool printBackendSL = (backendLabel != nullptr);
#else
    const bool printBackendSL = false;
#endif

    if (kPrintSkSL || printBackendSL) {
        SkShaderUtils::PrintShaderBanner(programKind);
        if (kPrintSkSL) {
            SkDebugf("SkSL:\n");
            SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(sksl));
        }
        if (printBackendSL) {
            SkDebugf("%s:\n", backendLabel);
            SkShaderUtils::PrintLineByLine(*output);
        }
    }

    if (outInterface) {
        *outInterface = program->fInterface;
    }
    return true;
}

}  // namespace SkSL
