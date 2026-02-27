/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_SparseStripTypes_DEFINED
#define skgpu_graphite_SparseStripTypes_DEFINED

#include "include/core/SkPath.h"
#include "include/private/base/SkTDArray.h"

namespace skgpu::graphite {

enum class FlattenMode {
    kScalar,
    kSimd,
};

struct Point {
    float x;
    float y;
};

struct Line {
    Point p0;
    Point p1;
};

//TODO REFACTOR
struct Edge {
    SkPoint p0;
    SkPoint p1;
};

// The maximum number of quadratics evaluated in SIMD passes.
constexpr size_t kMaxQuads = 16;

// Parameters for evaluating subdivisions. Equivalent to Rust's `FlattenParams`.
struct FlattenParams {
    double fA0;
    double fA2;
    double fU0;
    double fUScale;
    double fVal; // The number of `subdivisions * 2 * sqrt_tol`.
};

struct FlattenCtx {
    std::array<SkPoint, kMaxQuads + 4> fEvenPts;
    std::array<SkPoint, kMaxQuads> fOddPts;
    std::array<float, kMaxQuads> fA0;
    std::array<float, kMaxQuads> fDa;
    std::array<float, kMaxQuads> fU0;
    std::array<float, kMaxQuads> fUScale;
    std::array<float, kMaxQuads> fVal;

    size_t fNumQuads = 0;

    SkTDArray<SkPoint> fFlattenedCubics;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_SparseStripTypes_DEFINED
