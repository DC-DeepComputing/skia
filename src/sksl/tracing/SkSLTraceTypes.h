/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSLDEBUGTRACETYPES
#define SKSLDEBUGTRACETYPES

#include <cstdint>
#include <string>

namespace SkSL {
struct TraceInfo {
    enum class Op {
        kLine,  /** data: line number, (unused) */
        kVar,   /** data: slot, value */
        kEnter, /** data: function index, (unused) */
        kExit,  /** data: function index, (unused) */
        kScope, /** data: scope delta, (unused) */
    };
    Op op;
    int32_t data[2];
};

struct FunctionDebugInfo {
    /** Full function declaration: `float myFunction(half4 color)`) */
    std::string             name;
};
}

#endif
