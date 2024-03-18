/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/tracing/SkSLDebugTracePriv.h"

namespace SkSL {
DebugTracePriv::DebugTracePriv() = default;

sk_sp<DebugTracePriv> DebugTracePriv::MakeStub() {
     return sk_sp<DebugTracePriv>(new DebugTracePriv());
}
}
