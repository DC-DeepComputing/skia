/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SkSLSPIRVUtilsPriv_DEFINED
#define skgpu_graphite_SkSLSPIRVUtilsPriv_DEFINED

#include "include/gpu/ShaderErrorHandler.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/utils/SkShaderUtils.h"

namespace skgpu::graphite {

// Print the source code for all shaders generated.
#ifdef SK_PRINT_SKSL_SHADERS
static constexpr bool gPrintSKSL  = true;
#else
static constexpr bool gPrintSKSL  = false;
#endif

bool SkSLToSPIRV(SkSL::Compiler*,
                 const std::string& sksl,
                 SkSL::ProgramKind,
                 const SkSL::ProgramSettings&,
                 std::string* spirv,
                 SkSL::Program::Interface*,
                 ShaderErrorHandler*);

} // namespace skgpu::graphite

#endif // skgpu_graphite_SkSLSPIRVUtilsPriv_DEFINED
