/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDebugTrace_DEFINED
#define SkDebugTrace_DEFINED

#include <cstdint>

// A list containing every debugger trace op
// Using an X-Macro (https://en.wikipedia.org/wiki/X_Macro) to manage the list.
#define SKVM_TRACE_OP_LIST   \
    SKVM_TRACE_OP(Line)      \
    SKVM_TRACE_OP(FuncEnter) \
    SKVM_TRACE_OP(FuncExit)

#define SKVM_TRACE_OP(OP) k##OP,
enum SkVMTraceOp {
    SKVM_TRACE_OP_LIST
};
#undef SKVM_TRACE_OP

const char* GetTraceOpName(SkVMTraceOp op);

struct SkVMTrace {
    SkVMTraceOp  op;
    int32_t      mask;
    int32_t      data[2];
};

#endif // SkDebugTrace_DEFINED
