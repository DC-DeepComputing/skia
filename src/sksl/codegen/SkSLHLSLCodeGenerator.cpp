/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLHLSLCodeGenerator.h"

#include "include/private/base/SkDebug.h"
#include "src/core/SkTraceEvent.h"
#include "src/sksl/SkSLContext.h"  // IWYU pragma: keep
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/codegen/SkSLSPIRVCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <cstdint>
#include <memory>
#include <type_traits>

#if defined(SK_ENABLE_SPIRV_CROSS)
/*
 * SkSL uses SPIRV-Cross to translate SPIR-V into HLSL, but Skia and SPIRV-Cross are built with
 * different versions of spirv.h (or spirv.hpp). We must be careful not to #include both in the
 * same translation unit to avoid conflicts.
 */
#include <spirv_hlsl.hpp>
#endif

namespace SkSL {

static bool spirv_to_hlsl(const std::string& spirv, std::string* hlsl) {
#if defined(SK_ENABLE_SPIRV_CROSS)
    spirv_cross::CompilerHLSL hlslCompiler((const uint32_t*)spirv.c_str(),
                                           spirv.size() / sizeof(uint32_t));

    spirv_cross::CompilerGLSL::Options optionsGLSL;
    // Force all uninitialized variables to be 0, otherwise they will fail to compile by FXC.
    optionsGLSL.force_zero_initialized_variables = true;

    spirv_cross::CompilerHLSL::Options optionsHLSL;
    optionsHLSL.shader_model = 51;
    // PointCoord and PointSize are not supported in HLSL
    optionsHLSL.point_coord_compat = true;
    optionsHLSL.point_size_compat = true;

    hlslCompiler.set_common_options(optionsGLSL);
    hlslCompiler.set_hlsl_options(optionsHLSL);
    hlsl->assign(hlslCompiler.compile());
    return true;
#else
    // `skia_use_direct3d = true` was not set in our GN args.
    return false;
#endif
}

bool ToHLSL(Program& program, const ShaderCaps* caps, OutputStream& out) {
    TRACE_EVENT0("skia.shaders", "SkSL::ToHLSL");
    std::string hlsl;
    if (!ToHLSL(program, caps, &hlsl)) {
        return false;
    }
    out.writeString(hlsl);
    return true;
}

bool ToHLSL(Program& program, const ShaderCaps* caps, std::string* out) {
    std::string spirv;
    if (!ToSPIRV(program, caps, &spirv)) {
        return false;
    }
    if (!spirv_to_hlsl(spirv, out)) {
        program.fContext->fErrors->error(Position(), "HLSL cross-compilation not enabled");
        return false;
    }
    return true;
}

}  // namespace SkSL
