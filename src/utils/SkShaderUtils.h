/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaderUtils_DEFINED
#define SkShaderUtils_DEFINED

#include "include/private/base/SkDebug.h"

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

namespace SkSL { enum class ProgramKind : int8_t; }

namespace SkShaderUtils {

std::string PrettyPrint(std::string_view shader);

void VisitLineByLine(std::string_view text,
                     const std::function<void(int lineNumber, std::string_view lineText)>&);

// Prints shaders one line at the time. This ensures they don't get truncated by the adb log.
inline void PrintLineByLine(std::string_view text) {
    VisitLineByLine(text, [](int lineNumber, std::string_view lineText) {
        SkDebugf("%4i\t%.*s\n", lineNumber, (int)lineText.size(), lineText.data());
    });
}

// Combines raw shader and error text into an easier-to-read error message with line numbers.
std::string BuildShaderErrorMessage(std::string_view shader, std::string_view errors);

void PrintShaderBanner(SkSL::ProgramKind programKind);

}  // namespace SkShaderUtils

#endif
