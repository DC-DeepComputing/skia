/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"

#include <benchmark/benchmark.h>

static void matrix_transform(benchmark::State& state) {
    constexpr size_t n = 100;
    SkPoint src[n], dst[n];

    for (size_t i = 0; i < n; ++i) {
        src[i] = { (float)i, (float)i * 2 };
    }

    SkMatrix mx = SkMatrix::Scale(2, 3) * SkMatrix::RotateDeg(30);

    for (auto _ : state) {
        mx.mapPoints(dst, src);
    }
}
BENCHMARK(matrix_transform);
