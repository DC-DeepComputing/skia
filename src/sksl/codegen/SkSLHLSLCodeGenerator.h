/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_HLSLCODEGENERATOR
#define SKSL_HLSLCODEGENERATOR

#include "include/core/SkSpan.h"
#include "src/sksl/codegen/SkSLNativeShader.h"

#include <cstdint>
#include <string>

namespace SkSL {

class ErrorReporter;
class OutputStream;
struct Program;
struct ShaderCaps;

using ValidateSPIRVProc = bool (*)(ErrorReporter&, SkSpan<const uint32_t>);

/** Converts a Program into HLSL code. */
bool ToHLSL(Program& program,
            const ShaderCaps* caps,
            OutputStream& out,
            ValidateSPIRVProc = nullptr,
            uint32_t shaderModel = 51);
bool ToHLSL(Program& program,
            const ShaderCaps* caps,
            std::string* out,
            ValidateSPIRVProc,
            uint32_t shaderModel = 51);

// This explicit overload is used by SkSLToBackend (defaults to SM 5.1).
inline bool ToHLSL(Program& program, const ShaderCaps* caps, NativeShader* out) {
    return ToHLSL(program, caps, &out->fText, (ValidateSPIRVProc)nullptr, /*shaderModel=*/51);
}

// Explicit overload for Shader Model 5.0 (used by D3D11).
inline bool ToHLSL50(Program& program, const ShaderCaps* caps, NativeShader* out) {
    return ToHLSL(program, caps, &out->fText, (ValidateSPIRVProc)nullptr, /*shaderModel=*/50);
}

}  // namespace SkSL

#endif
