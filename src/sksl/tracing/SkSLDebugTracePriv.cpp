/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/tracing/SkSLDebugTracePriv.h"

namespace SkSL {
DebugTracePriv::DebugTracePriv() = default;

sk_sp<DebugTracePriv> DebugTracePriv::MakeNoOpImpl() {
     return sk_sp<DebugTracePriv>(new DebugTracePriv());
}
}

namespace {
SkSL::DebugTraceFactory debugTraceFactory = SkSL::DebugTracePriv::MakeNoOpImpl;
}

namespace SkSL {

sk_sp<SkSL::DebugTracePriv> MakeDebugTrace() {
    SkASSERT(debugTraceFactory);
    auto rv = debugTraceFactory();
    SkASSERT(rv);
    return rv;
}

void SetDebugTraceFactory(DebugTraceFactory newFact) {
    SkASSERT(newFact);
    debugTraceFactory = newFact;
}

}
