/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SkSLSPIRVUtilsPriv_DEFINED
#define skgpu_graphite_SkSLSPIRVUtilsPriv_DEFINED

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/utils/SkShaderUtils.h"

namespace skgpu::graphite {

// Print the source code for all shaders generated.
#ifdef SK_PRINT_SKSL_SHADERS
static constexpr bool gPrintSKSL  = true;
#else
static constexpr bool gPrintSKSL  = false;
#endif
bool SkSLToSPIRV(SkSL::Compiler* compiler,
                 const std::string& sksl,
                 SkSL::ProgramKind programKind,
                 const SkSL::ProgramSettings& settings,
                 std::string* spirv,
                 SkSL::Program::Interface* outInterface,
                 ShaderErrorHandler* errorHandler) {
#ifdef SK_DEBUG
    std::string src = SkShaderUtils::PrettyPrint(sksl);
#else
    const std::string& src = sksl;
#endif
    std::unique_ptr<SkSL::Program> program = compiler->convertProgram(programKind,
                                                                      src,
                                                                      settings);
    if (!program || !compiler->toSPIRV(*program, spirv)) {
        errorHandler->compileError(src.c_str(), compiler->errorText().c_str());
        return false;
    }

    if (gPrintSKSL) {
        SkShaderUtils::PrintShaderBanner(programKind);
        SkDebugf("SKSL:\n");
        SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(sksl));
    }

    *outInterface = program->fInterface;
    return true;
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_SkSLSPIRVUtilsPriv_DEFINED
