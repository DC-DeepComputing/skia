/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPIRVTOHLSL
#define SKSL_SPIRVTOHLSL

#include <cstdint>
#include <string>
#include <vector>

namespace SkSL {

void SPIRVtoHLSL(const std::vector<uint32_t>& spirv, std::string* hlsl);

}  // namespace SkSL

#endif
