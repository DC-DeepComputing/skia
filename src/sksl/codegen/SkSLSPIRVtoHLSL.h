/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPIRVTOHLSL
#define SKSL_SPIRVTOHLSL

#include <string>

namespace SkSL {
class Compiler;
struct Program;
class OutputStream;

bool SPIRVtoHLSL(const std::string& spirv, std::string* hlsl);

bool ToHLSL(Compiler&, Program&, OutputStream& out);
bool ToHLSL(Compiler&, Program&, std::string* out);

}  // namespace SkSL

#endif
