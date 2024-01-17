/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPIRVTOHLSL
#define SKSL_SPIRVTOHLSL

#include "src/sksl/ir/SkSLProgram.h"

#include <string>

namespace skgpu {
class ShaderErrorHandler;
}

namespace SkSL {
class Compiler;
class OutputStream;
enum class ProgramKind : int8_t;
struct ProgramSettings;

bool SPIRVtoHLSL(const std::string& spirv, std::string* hlsl);

bool ToHLSL(Compiler*, Program&, OutputStream& out);
bool ToHLSL(Compiler*, Program&, std::string* out);
bool ToHLSL(Compiler*,
            const std::string& sksl,
            ProgramKind programKind,
            const ProgramSettings& settings,
            std::string* hlsl,
            Program::Interface*,
            skgpu::ShaderErrorHandler* errorHandler);

}  // namespace SkSL

#endif
