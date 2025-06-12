/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_HLSLCODEGENERATOR
#define SKSL_HLSLCODEGENERATOR

#include <cstdint>
#include <string>
#include <vector>

namespace SkSL {

class ErrorReporter;
class OutputStream;
struct Program;
struct ShaderCaps;

using ValidateSPIRVProc = bool (*)(ErrorReporter&, const std::vector<uint32_t>&);

/** Converts a Program into HLSL code. */
bool ToHLSL(Program& program,
            const ShaderCaps* caps,
            OutputStream& out,
            ValidateSPIRVProc = nullptr);
bool ToHLSL(Program& program, const ShaderCaps* caps, std::string* out, ValidateSPIRVProc);

// This explicit overload is used by SkSLToBackend.
inline bool ToHLSL(Program& program,
                   const ShaderCaps* caps,
                   std::string* textOut,
                   std::vector<uint32_t>* binaryOut = nullptr) {
    return ToHLSL(program, caps, textOut, (ValidateSPIRVProc) nullptr);
}

}  // namespace SkSL

#endif
