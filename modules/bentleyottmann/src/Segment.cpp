// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Segment.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"

#include <algorithm>
#include <cmath>
#include <optional>

namespace bentleyottmann {

// -- Segment --------------------------------------------------------------------------------------
Point Segment::upper() const {
    return std::min(p0, p1);
}

Point Segment::lower() const {
    return std::max(p0, p1);
}

// Use auto [l, t, r, b] = s.bounds();
std::tuple<int32_t, int32_t, int32_t, int32_t> Segment::bounds() const {
    auto [l, r] = std::minmax(p0.x, p1.x);
    auto [t, b] = std::minmax(p0.y, p1.y);
    return std::make_tuple(l, t, r, b);
}

bool no_intersection_by_bounding_box(const Segment& s0, const Segment& s1) {
    auto [left0, top0, right0, bottom0] = s0.bounds();
    auto [left1, top1, right1, bottom1] = s1.bounds();
    // If the sides of the box touch, then there is no new intersection.
    return right0 <= left1 || right1 <= left0 || bottom0 <= top1 || bottom1 <= top0;
}

std::optional<Point> intersect(const Segment& s0, const Segment& s1) {

    // Check if the bounds intersect.
    if (no_intersection_by_bounding_box(s0, s1)) {
        return std::nullopt;
    }

    // Create the Q, R, and S vectors rooted at s0.p0.
    Point O = s0.p0,
            Q = s0.p1 - O,
            R = s1.p0 - O,
            S = s1.p1 - O;

    // 64-bit cross product.
    auto cross = [](const Point& v0, const Point& v1) {
        int64_t x0 = SkToS64(v0.x),
                y0 = SkToS64(v0.y),
                x1 = SkToS64(v1.x),
                y1 = SkToS64(v1.y);
        return x0 * y1 - y0 * x1;
    };

    // Actually calculate the cross products.
    int64_t QXR = cross(Q, R),
            QXS = cross(Q, S);

    // If the endpoint is on Q, then there is no crossing. Only true intersections are returned.
    // For the intersection calculation, line segments do not include their end-points.
    if (QXR == 0 || QXS == 0) {
        return std::nullopt;
    }

    // The cross products have the same sign, so no intersection. There is no roundoff error in
    // QXR or QXS. This ensures that there is really an intersection.
    if ((QXR ^ QXS) >= 0) {
        return std::nullopt;
    }

    // TODO: this calculation probably needs to use 32-bit x 64-bit -> 96-bit multiply and
    // 96-bit / 64-bit -> 32-bit quotient and a 64-bit remainder.
    // N / D constitute a value on [0, 1], where the intersection I is
    //     I = s0.p0 + (s0.p1 - s0.p0) * N/D.
    double N = QXR,
            D = QXR - QXS,
            t = N / D;

    SkASSERT(0 <= t && t <= 1);

    // Calculate the intersection using doubles.
    int32_t x = std::round(t * (s1.p1.x - s1.p0.x) + s1.p0.x),
            y = std::round(t * (s1.p1.y - s1.p0.y) + s1.p0.y);

    return Point{x, y};
}
}  // namespace bentleyottmann
