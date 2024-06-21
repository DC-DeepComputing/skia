/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPIRVVALIDATOR
#define SKSL_SPIRVVALIDATOR

#include <string_view>

namespace SkSL {

class ErrorReporter;

bool ValidateSPIRV(ErrorReporter&, std::string_view);
bool ValidateSPIRVAndDissassemble(ErrorReporter&, std::string_view);

}  // namespace SkSL

#endif
