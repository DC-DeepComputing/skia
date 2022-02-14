/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This is an experimental (and probably temporary) solution that allows
 * to compare performance SkVM blitters vs RasterPipeline blitters.
 * In addition to measuring performance (which is questionable) it also produces
 * other counts (pixels, scanlines) and more detailed traces that
 * can explain the current results (SkVM is slower) and help improve it.
 * The entire code is hidden under build flag skia_compare_vm_vs_rp=true
 * and will not appear at all without it.
 */

#ifndef SkBlitterTrace_DEFINED
#define SkBlitterTrace_DEFINED

#include <inttypes.h>
#include <unordered_map>
#include "src/utils/SkBlitterTraceCommon.h"
#include "src/utils/SkCycles.h"

#define NO_CODE do {} while (0)

#ifdef SKIA_COMPARE_VM_VS_RP

#ifdef SKVM_BLITTER_TRACE_ON
SkBlitterTrace SKVM_BLITTER_TRACE_NAME("VM", false);
#undef STEP_BLITTER_TRACE
#define STEP_BLITTER_TRACE(name, trace, scanlines, pixels) \
    SkBlitterTrace::Step name(trace ? &SKVM_BLITTER_TRACE_NAME : nullptr, \
                              #name,     \
                              scanlines, \
                              pixels)
#endif

#ifdef RASTER_PIPELINE_BLITTER_TRACE_ON
SkBlitterTrace RP_BLITTER_TRACE_NAME("RP", false);
#undef STEP_BLITTER_TRACE
#define STEP_BLITTER_TRACE(name, trace, scanlines, pixels) \
    SkBlitterTrace::Step name(trace ? &RP_BLITTER_TRACE_NAME : nullptr, \
                              #name,     \
                              scanlines, \
                              pixels)
#endif

#define START_BLITTER_TRACE \
extern SkBlitterTrace SKVM_BLITTER_TRACE_NAME;                  \
extern SkBlitterTrace RP_BLITTER_TRACE_NAME;                    \
                                                                \
static SkBlitterTrace SKVM_BLITTER_TRACE_CAPTURE("VM", false);  \
static SkBlitterTrace RP_BLITTER_TRACE_CAPTURE("RP", false);

#define ADD_TO_STEP_BLITTER_TRACE(step, scanlines, pixels) \
        step.add(/*scanlines=*/0, /*pixels=*/run)
#else
#define INITIATE_BLITTER_TRACE(type) NO_CODE
#define STEP_BLITTER_TRACE(name, trace, scanlines, pixels) NO_CODE
#define ADD_TO_STEP_BLITTER_TRACE(step, scanlines, pixels) NO_CODE
#endif

#endif // SkBlitterTrace_DEFINED
