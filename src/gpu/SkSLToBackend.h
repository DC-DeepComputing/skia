/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_SkSLToBackend_DEFINED
#define skgpu_SkSLToBackend_DEFINED

#include <cstdint>
#include <string>
#include <vector>

namespace SkSL {

enum class ProgramKind : int8_t;
struct Program;
struct ProgramInterface;
struct ProgramSettings;
struct ShaderCaps;

}  // namespace SkSL

namespace skgpu {

struct NativeShader;
class ShaderErrorHandler;

/**
 * Wrapper for the SkSL compiler with useful logging and error handling.
 * Depending on whether the output is text or binary, either output->fText or output->fBinary is
 * filled.
 */
bool SkSLToBackend(const SkSL::ShaderCaps* caps,
                   bool (*toBackend)(SkSL::Program&,
                                     const SkSL::ShaderCaps*,
                                     std::string*,
                                     std::vector<uint32_t>*),
                   const char* backendLabel,
                   const std::string& sksl,
                   SkSL::ProgramKind programKind,
                   const SkSL::ProgramSettings& settings,
                   NativeShader* output,
                   SkSL::ProgramInterface* outInterface,
                   ShaderErrorHandler* errorHandler);

}  // namespace skgpu

#endif // skgpu_SkSLToBackend_DEFINED
