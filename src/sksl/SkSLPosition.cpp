/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/SkSLPosition.h"

#include "src/sksl/SkSLThreadContext.h"

namespace SkSL {

#if __has_builtin(__builtin_FILE) && __has_builtin(__builtin_LINE)
    Position Position::Capture(const char* file, int line) {
        ThreadContext::SetFilename(file);
        return Position::Line(line);
    }
#endif

} // namespace SkSL
