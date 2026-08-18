/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED
#define skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkString.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <cmath>
#include <cstdint>
#include <vector>

namespace skgpu::graphite {

class ScanlineOracle8x {
public:
    struct CurveIntersection {
        skvx::float8 x;
        skvx::int8 dir;  // +1 for downward, -1 for upward, 0 for no hit in this lane
    };

    struct RowWindingInterval {
        int32_t x;
        skvx::int8 winding;  // 8-element subsample winding vector starting at x until next interval
    };

    static void IntersectLine(
            const SkPoint pts[2], skvx::float8 targetY, std::vector<CurveIntersection>* out);
    static void IntersectQuad(
            const SkPoint pts[3], skvx::float8 targetY, std::vector<CurveIntersection>* out);
    static void IntersectConic(
            const SkPoint pts[3], float weight, skvx::float8 targetY, std::vector<CurveIntersection>* out);
    static void IntersectCubic(
            const SkPoint pts[4], skvx::float8 targetY, std::vector<CurveIntersection>* out);

    static void CollectPathIntersections(
            const SkPath& path, skvx::float8 targetY, std::vector<CurveIntersection>* out);

    static skvx::int8 EvaluatePixelWinding(
            float px, const std::vector<CurveIntersection>& hits);

    static std::vector<RowWindingInterval> BuildRowIntervals(int py, const SkPath& path);

    static skvx::int8 Evaluate(SkPoint pt, const SkPath& path);
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED
