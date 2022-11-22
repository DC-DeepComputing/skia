/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLSPIRVtoHLSL.h"

#if defined(SK_GENERATE_HLSL_WITH_TINT)

#include "tint/tint.h"

/*
 * This translation unit serves as a bridge between Skia/SkSL and tint.
 */

namespace SkSL {

bool SPIRVtoHLSL(const std::string& spirv, std::string* outResult) {
    const uint32_t* spirvWords = reinterpret_cast<const uint32_t*>(spirv.c_str());
    // TODO(armansito): IWBN if the tint SPIR-V reader accepted a raw pointer (or something
    // like std::span) instead of a std::vector, which requires a copy from `spirv`.
    const std::vector<uint32_t> spirvParserInput(spirvWords, spirvWords + spirv.size() / 4);
    tint::Program program(tint::reader::spirv::Parse(spirvParserInput));
    if (program.Diagnostics().count() > 0) {
        tint::diag::Formatter diagFormatter;
        *outResult = diagFormatter.format(program.Diagnostics());
        return false;
    }

    tint::writer::hlsl::Result tintResult = tint::writer::hlsl::Generate(&program, {});
    if (!tintResult.success) {
        *outResult = std::move(tintResult.error);
        return false;
    }

    // TODO(armansito): The spirv-cross version of this code used some compiler settings that are
    // not supported in Tint's HLSL writer:
    // - (GLSL) force_zero_initialized_variables = true
    // - (HLSL) shader_model = 51
    // - (HLSL) point_coord_compat = true
    // - (HLSL) point_size_compat = true
    *outResult = std::move(tintResult.hlsl);
    return true;
}

}  // namespace SkSL

#else

namespace SkSL {

bool SPIRVtoHLSL(const std::string&, std::string* outResult) {
    *outResult = "HLSL cross-compilation not enabled";
    return false;
}

}  // namespace SkSL

#endif
