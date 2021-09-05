/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/SkSLErrorReporter.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

namespace SkSL {

static bool stringview_contains(skstd::string_view haystack, skstd::string_view needle) {
    for (size_t i = 0; i < haystack.length() - needle.length(); ++i) {
        if (!memcmp(haystack.data() + i, needle.data(), needle.length())) {
            return true;
        }
    }
    return false;
}

void ErrorReporter::error(skstd::string_view msg, PositionInfo position) {
    if (stringview_contains(msg.data(), Compiler::POISON_TAG)) {
        // don't report errors on poison values
        return;
    }
    ++fErrorCount;
    this->handleError(msg, position);
}

void ErrorReporter::error(int offset, skstd::string_view msg) {
    if (stringview_contains(msg.data(), Compiler::POISON_TAG)) {
        // don't report errors on poison values
        return;
    }
    if (offset == -1) {
        ++fErrorCount;
        fPendingErrors.push_back(SkString(msg));
    } else {
        this->error(msg, this->position(offset));
    }
}

PositionInfo ErrorReporter::position(int offset) const {
    if (fSource && offset >= 0) {
        int line = 1;
        for (int i = 0; i < offset; i++) {
            if (fSource[i] == '\n') {
                ++line;
            }
        }
        return PositionInfo(/*file=*/nullptr, line);
    } else {
        return PositionInfo();
    }
}

} // namespace SkSL
