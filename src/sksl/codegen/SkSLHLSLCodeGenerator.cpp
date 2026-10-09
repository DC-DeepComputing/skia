/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLHLSLCodeGenerator.h"

#include "include/private/SkDebug.h"
#include "src/core/SkTraceEvent.h"
#include "src/sksl/SkSLContext.h"  // IWYU pragma: keep
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/codegen/SkSLSPIRVCodeGenerator.h"
#include "src/sksl/codegen/SkSLSPIRVtoHLSL.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <cstdint>
#include <memory>
#include <type_traits>

namespace SkSL {

bool ToHLSL(Program& program,
            const ShaderCaps* caps,
            OutputStream& out,
            ValidateSPIRVProc validateSPIRV,
            uint32_t shaderModel) {
    TRACE_EVENT0("skia.shaders", "SkSL::ToHLSL");
    std::string hlsl;
    if (!ToHLSL(program, caps, &hlsl, validateSPIRV, shaderModel)) {
        return false;
    }
    out.writeString(hlsl);
    return true;
}

bool ToHLSL(Program& program,
            const ShaderCaps* caps,
            std::string* out,
            ValidateSPIRVProc validateSPIRV,
            uint32_t shaderModel) {
    std::vector<uint32_t> spirv;
    if (!ToSPIRV(program, caps, &spirv, validateSPIRV)) {
        return false;
    }
    SPIRVtoHLSL(spirv, out, shaderModel);
    return true;
}

}  // namespace SkSL
