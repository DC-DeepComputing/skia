/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPIRVCODEGENERATOR
#define SKSL_SPIRVCODEGENERATOR

#include <cstdint>
#include <string>
#include <vector>

namespace SkSL {

class ErrorReporter;
class OutputStream;
struct Program;
struct ShaderCaps;

using ValidateSPIRVProc = bool (*)(ErrorReporter&, const std::vector<uint32_t>&);

/**
 * Converts a Program into a SPIR-V binary.
 */
bool ToSPIRV(Program& program, const ShaderCaps* caps, OutputStream& out, ValidateSPIRVProc = nullptr);
bool ToSPIRV(Program& program,
             const ShaderCaps* caps,
             std::vector<uint32_t>* out,
             ValidateSPIRVProc);

// This explicit overload is used by SkSLToBackend.
inline bool ToSPIRV(Program& program,
                    const ShaderCaps* caps,
                    std::string*,
                    std::vector<uint32_t>* binaryOut) {
    return ToSPIRV(program, caps, binaryOut, (ValidateSPIRVProc) nullptr);
}
}  // namespace SkSL

#endif
