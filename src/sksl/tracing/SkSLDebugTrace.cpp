/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint.h"
#include "include/sksl/SkSLDebugTrace.h"
#include "src/sksl/tracing/SkSLTraceHook.h"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifdef SKSL_ENABLE_TRACING

namespace SkSL {

DebugTrace::DebugTrace() = default;
DebugTrace::~DebugTrace() = default;

void DebugTrace::setTraceCoord(const SkIPoint& coord) {
    fTraceCoord = coord;
}

void DebugTrace::setSource(std::string source) {
    fSource.clear();
    std::stringstream stream{std::move(source)};
    while (stream.good()) {
        fSource.push_back({});
        std::getline(stream, fSource.back(), '\n');
    }
}

}  // namespace SkSL

#else // SKSL_ENABLE_TRACING

namespace SkSL {

DebugTrace::DebugTrace() = default;
DebugTrace::~DebugTrace() = default;
void DebugTrace::setTraceCoord(const SkIPoint &coord) {}
void DebugTrace::setSource(std::string source) {}

}  // namespace SkSL

#endif
