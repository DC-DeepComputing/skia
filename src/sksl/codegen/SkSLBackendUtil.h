/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/sksl/ir/SkSLProgram.h"

#include <string>

namespace skgpu {
class ShaderErrorHandler;
}

namespace SkSL {
class Compiler;
enum class ProgramKind : int8_t;
struct Program;
struct ProgramSettings;
struct ShaderCaps;

bool ToBackend(Compiler* compiler,
               bool (*toBackend)(Compiler*, Program&, std::string*),
               const char* backendLabel,
               const std::string& sksl,
               ProgramKind programKind,
               const ProgramSettings& settings,
               std::string* output,
               Program::Interface* outInterface,
               skgpu::ShaderErrorHandler* errorHandler);

}  // namespace SkSL
