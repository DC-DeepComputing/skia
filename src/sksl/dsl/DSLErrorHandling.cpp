/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLErrorHandling.h"

#include "src/sksl/dsl/priv/DSLWriter.h"

namespace SkSL {

namespace dsl {

void ErrorHandler::error(const char* msg, PositionInfo position) {
    ++fErrorCount;
    this->handleError(msg, position);
}

void ErrorHandler::error(int offset, const char* msg) {
    if (offset == -1) {
        ++fErrorCount;
        fPendingErrors.push_back(std::string(msg));
    } else {
        this->error(msg, this->position(offset));
    }
}

dsl::PositionInfo ErrorHandler::position(int offset) const {
    if (fSource && offset >= 0) {
        int line = 1;
        for (int i = 0; i < offset; i++) {
            if (fSource[i] == '\n') {
                ++line;
            }
        }
        return dsl::PositionInfo(fFilename, line);
    } else {
        return dsl::PositionInfo(fFilename, -1);
    }
}

} // namespace dsl

} // namespace SkSL
