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

// `result` contains the HLSL shader text on success and an error text on failure.
bool SPIRVtoHLSL(const std::string& spirv, std::string* result);

}  // namespace SkSL

#endif
