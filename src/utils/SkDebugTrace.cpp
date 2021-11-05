/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkDebugTrace.h"

const char* GetTraceOpName(SkVMTraceOp op) {
    #define SKVM_TRACE_OP(OP) case k##OP: return #OP ;

    switch (op) {
        SKVM_TRACE_OP_LIST
    }

    #undef SKVM_TRACE_OP
}
