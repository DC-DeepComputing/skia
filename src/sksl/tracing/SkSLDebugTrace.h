/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DEBUG_TRACE
#define SKSL_DEBUG_TRACE

#include "include/core/SkRefCnt.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstdint>
#include <string>

class SkWStream;

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

struct SlotDebugInfo {
    /** The full name of this variable (without component), e.g. `myArray[3].myStruct.myVector` */
    std::string             name;
    /** The dimensions of this variable: 1x1 is a scalar, Nx1 is a vector, NxM is a matrix. */
    uint8_t                 columns = 1, rows = 1;
    /** Which component of the variable is this slot? (e.g. `vec4.z` is component 2) */
    uint8_t                 componentIndex = 0;
    /** Complex types (arrays/structs) can be tracked as a "group" of adjacent slots. */
    int                     groupIndex = 0;
    /** What kind of numbers belong in this slot? */
    SkSL::Type::NumberKind  numberKind = SkSL::Type::NumberKind::kNonnumeric;
    /** Where is this variable located in the program? */
    int                     line = 0;
    Position                pos = {};
    /** If this slot holds a function's return value, contains 1; if not, -1. */
    int                     fnReturnValue = -1;
};

struct FunctionDebugInfo {
    /** Full function declaration: `float myFunction(half4 color)`) */
    std::string             name;
};

class DebugTrace : public SkRefCnt {
public:
    /** Serializes a debug trace to JSON which can be parsed by our debugger. */
    virtual void writeTrace(SkWStream* w) const = 0;

    /** Generates a human-readable dump of the debug trace. */
    virtual void dump(SkWStream* o) const = 0;
};

} // namespace SkSL

#endif
