/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTracedRuntimeEffect_DEFINED
#define SkTracedRuntimeEffect_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/private/base/SkAPI.h"
#include "include/sksl/SkSLDebugTrace.h"

struct SkIPoint;

namespace SkRuntimeEffects {
/**
 * Creates a new Runtime Effect patterned after an already-existing one. The new shader behaves
 * like the original, but also creates a debug trace of its execution at the requested
 * coordinate. After painting with this shader, the associated DebugTrace object will contain a
 * shader execution trace. Call `writeTrace` on the debug trace object to generate a full trace
 * suitable for a debugger, or call `dump` to emit a human-readable trace.
 *
 * Debug traces are only supported on a raster (non-GPU) canvas.

 * Debug traces are currently only supported on shaders. Color filter and blender tracing is a
 * work-in-progress.
 */
struct SK_API TracedShader {
    sk_sp<SkShader> shader;
    sk_sp<SkSL::DebugTrace> debugTrace;
};
SK_API TracedShader MakeTraced(sk_sp<SkShader> shader, const SkIPoint& traceCoord);
}  // namespace SkRuntimeShaders

#endif
