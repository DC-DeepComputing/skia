/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/benchmark_target/BenchmarkTarget.h"
#include "src/base/SkTime.h"
#include "tools/flags/CommandLineFlags.h"

DEFINE_int(loops,
           kAutoTuneLoops,
           SkStringPrintf("Number of times to run each bench. Set this to %d to auto-"
                          "tune for each bench. Timings are only reported when auto-tuning.",
                          kAutoTuneLoops)
                   .c_str());

DEFINE_int(maxLoops, 1000000, "Never run a bench more times than this.");

DECLARE_int(loops);
DECLARE_int(maxLoops);

void BenchmarkTarget::setup() const { fBenchmark->perCanvasPreDraw(getCanvas()); }

double BenchmarkTarget::time(int loops) const {
    SkCanvas* canvas = getCanvas();
    if (canvas) {
        canvas->clear(SK_ColorWHITE);
    }
    fBenchmark->preDraw(canvas);
    double start = nowMs();
    canvas = onBeforeDraw(canvas);

    fBenchmark->draw(loops, canvas);

    onAfterDraw();
    double elapsed = nowMs() - start;
    fBenchmark->postDraw(canvas);
    return elapsed;
}

void BenchmarkTarget::tearDown() const { fBenchmark->perCanvasPostDraw(getCanvas()); }

SkCanvas* BenchmarkTarget::getCanvas() const {
    if (!fSurfaceManager || !fSurfaceManager->getSurface()) {
        return nullptr;
    }
    return fSurfaceManager->getSurface()->getCanvas();
}

Benchmark* BenchmarkTarget::getBenchmark() const { return fBenchmark; }

double BenchmarkTarget::nowMs() const { return SkTime::GetNSecs() * 1e-6; }

// Based on nanobench's clamp_loops():
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#419.
int BenchmarkTarget::clampLoops(int loops) const {
    if (loops < 1) {
        SkDebugf(
                "ERROR: Clamping loops from %d to 1. "
                "There is probably something wrong with the benchmark.\n",
                loops);
        return 1;
    }
    if (loops > FLAGS_maxLoops) {
        SkDebugf("WARNING: Clamping loops from %d to --maxLoops (%d).\n", loops, FLAGS_maxLoops);
        return FLAGS_maxLoops;
    }
    return loops;
}

// Based on nanobench's detect_forever_loops():
// https://skia.googlesource.com/skia/+/a063eaeaf1e09e4d6f42e0f44a5723622a46d21c/bench/nanobench.cpp#411.
int BenchmarkTarget::detectForeverLoops(int loops) const {
    // Look for a magic run-forever value.
    if (loops < 0) {
        loops = SK_MaxS32;
    }
    return loops;
}
