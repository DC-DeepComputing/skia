/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "samplecode/Sample.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkVertices.h"
#include "include/private/SkTPin.h"

#include <unordered_set>

// TODO tasks
// 2. color-code the triangles based on linear, frag, or constant coverage
// 3. work out how to dynamically classify triangles if it can't be fixed assignment... -> I don't think we need it since I got the tangent cross over to collapse nicely
// 4. fix const color snapping on stroked rrects when one corner collapses but the other side doesn't
// 5. fix snapping/assignment for interior outsets when the insets collapse
// 6. improve colorization (black base shape, white lines?, dark gray background, cyan/magenta for inner/outer, lt vs dark for cov/linear?)




static SkPaint paint(SkColor color, float strokeWidth = -1.f, SkPaint::Join join = SkPaint::kMiter_Join) {
    SkPaint paint;
    paint.setColor(color);
    paint.setAntiAlias(true);
    if (strokeWidth >= 0.f) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(strokeWidth);
        paint.setStrokeJoin(join);
        paint.setStrokeMiter(SK_ScalarSqrt2);
    }
    return paint;
}

// Singular values for [a b][c d] 2x2 matrix.
static std::pair<float, float> minmax_scales(float a, float b, float c, float d) {
    float s1 = a*a + b*b + c*c + d*d;

    float e = a*a + b*b - c*c - d*d;
    float f = a*c + b*d;
    float s2 = SkScalarSqrt(e*e + 4*f*f);

    float singular1 = SkScalarSqrt(0.5f * (s1 + s2));
    float singular2 = SkScalarSqrt(0.5f * (s1 - s2));

    return {std::min(singular1, singular2), std::max(singular1, singular2)};
}

//              [m00 m01 * m03]                                 [f(u,v)]
// Assuming M = [m10 m11 * m13], define the projected p'(u,v) = [g(u,v)] where
//              [ *   *  *  * ]
//              [m30 m31 * m33]
//                                                        [x]     [u]
// f(u,v) = x(u,v) / w(u,v), g(u,v) = y(u,v) / w(u,v) and [y] = M*[v]
//                                                        [*] =   [0]
//                                                        [w]     [1]
//
// x(u,v) = m00*u + m01*v + m03
// y(u,v) = m10*u + m11*v + m13
// w(u,v) = m30*u + m31*v + m33
//
// dx/du = m00, dx/dv = m01,
// dy/du = m10, dy/dv = m11
// dw/du = m30, dw/dv = m31
//
// df/du = (dx/du*w - x*dw/du)/w^2 = (m00*w - m30*x)/w^2
// df/dv = (dx/dv*w - x*dw/dv)/w^2 = (m01*w - m31*x)/w^2
// dg/du = (dy/du*w - y*dw/du)/w^2 = (m10*w - m30*y)/w^2
// dg/dv = (dy/dv*w - y*dw/du)/w^2 = (m11*w - m31*y)/w^2
//
// Singular values of [df/du df/dv] define perspective correct minimum and maximum scale factors
//                    [dg/du dg/dv]
// for M evaluated at  (u,v)
static std::tuple<SkV3, float, float> minmax_scales(const SkM44& matrix, const SkV2& p) {
    SkV4 devP = matrix.map(p.x, p.y, 0.f, 1.f);

    const float dxdu = matrix.rc(0,0);
    const float dxdv = matrix.rc(0,1);
    const float dydu = matrix.rc(1,0);
    const float dydv = matrix.rc(1,1);
    const float dwdu = matrix.rc(3,0);
    const float dwdv = matrix.rc(3,1);

    float invW2 = 1.f / (devP.w * devP.w);
    // non-persp has invW2 = 1, devP.w = 1, dwdu = 0, dwdv = 0
    float dfdu = (devP.w*dxdu - devP.x*dwdu) * invW2; // non-persp -> dxdu -> m00
    float dfdv = (devP.w*dxdv - devP.x*dwdv) * invW2; // non-persp -> dxdv -> m01
    float dgdu = (devP.w*dydu - devP.y*dwdu) * invW2; // non-persp -> dydu -> m10
    float dgdv = (devP.w*dydv - devP.y*dwdv) * invW2; // non-persp -> dydv -> m11

    // no-persp, this is the singular values of [m00,m01][m10,m11], which is just the upper 2x2
    auto [min, max] = minmax_scales(dfdu, dfdv, dgdu, dgdv);
    return {SkV3{devP.x, devP.y, devP.w}, min, max};
}

static constexpr float kAARadius = 10.f;

static float local_aa_radius(const SkM44& matrix, const SkV2& p) {
    auto [p3, min, max] = minmax_scales(matrix, p);
    return kAARadius / min;
}

// Homogenous line and point operations
static SkV3 line_between_homogenous_points(const SkV3& p0, const SkV3& p1) {
    return p1.cross(p0);
}

static SkV3 line_intersection(const SkV3& l0, const SkV3& l1) {
    return l1.cross(l0);
}

static SkV3 shift_line(const SkV3& line, float distance) {
    float mag = SkV2{line.x, line.y}.length(); // (a,b) normalization factor
    return {line.x, line.y, line.z - distance * mag};
}

static SkV3 perpendicular_through_point(const SkV3& line, const SkV3& p) {
    float c = (line.x * p.y - line.y * p.x) / p.z; // What happens if w <= 0?
    return {line.y, -line.x, c};
}

// Return bisecting vector between the normals of l0 and l1.
static SkV2 bisector(const SkV3& l0, const SkV3& l1) {
    SkV2 n0{l0.x, l0.y};
    SkV2 n1{l1.x, l1.y};

    n0 = n0.normalize();
    n1 = n1.normalize();

    if (n0.dot(n1) < -0.8f) {
        // nearly parallel lines, so switch to bisecting vectors parallel to the lines, and then
        // flip so it points in a consistent direction. Given that the normals are interpreted as
        // tail-to-tail, and lines have consistent winding, then we negate e0 so that the computed
        // bisector points in the same direction as the normals.
        n0 = {-n0.y, n0.x};
        n1 = {n1.y, -n1.x};
    }

    return n0 + n1;
}

// Return a point outset from p along the given direction
static SkV3 shift_point(const SkV3& point, const SkV2 v, float distance) {
    // Line through point in the direction of 'v'
    SkV3 dir = {v.y, -v.x, (v.x*point.y - v.y * point.x) / point.z};
    // Line perpendicular to 'v' offset by distance from 'point'
    SkV3 shifted_perp = shift_line(perpendicular_through_point(dir, point), -distance);
    // Intersection of the two lines gives shifted point
    return line_intersection(dir, shifted_perp);
}

static SkV3 bisection(const SkV3& e0, const SkV3& e1, const SkV3& p, float distance) {
    return shift_point(p, bisector(e0, e1), distance);
}

static SkV3 map(const SkM44& matrix, SkV2 p) {
    SkV4 d = matrix.map(p.x, p.y, 0.f, 1.f);
    return SkV3{d.x, d.y, d.w};
}

struct CornerVerts {
    // Stroked round rect          | Stroked rect (round) | Stroked rect (bevel) | Stroked rect (miter)  | Filled rrect | Filled rect
    //    20 verts                      17 verts               11 verts               9 verts                 14 verts      6 verts
    SkV3 fOuterFrame0;           //     ""                     ""                     ""                      ""            ""
    SkV3 fOuterFrame0_outset;    //     ""                     ""                     ""                      ""            ""
    SkV3 fOuterFrame0_inset;     //     ""                     ""                  fOuterCorner_inset         ""          fOuterCorner_inset

    SkV3 fOuterFrame1;           //     ""                     ""                  fOuterFrame0               ""          fOuterFrame0
    SkV3 fOuterFrame1_outset;    //     ""                     ""                     ""                      ""            ""
    SkV3 fOuterFrame1_inset;     //     ""                     ""                  fOuterCorner_inset         ""          fOuterCorner_inset

    SkV3 fOuterTangent0;         //     ""                   fOuterFrame0          fOuterFrame0               ""          fOuterFrame0
    SkV3 fOuterTangent0_outsetA; //     ""                   fOuterFrame0_bisector fOuterFrame0_outset        ""          fOuterFrame0_outset
    SkV3 fOuterTangent0_outsetB; //     ""                   fOuterFrame0_outset   fOuterCorner_bisector      ""          fOuterCorner_bisector

    SkV3 fOuterTangent1;         //     ""                   fOuterFrame1          fOuterFrame0               ""          fOuterFrame0
    SkV3 fOuterTangent1_outsetA; //     ""                   fOuterFrame1_bisector fOuterFrame1_outset        ""          fOuterFrame1_outset
    SkV3 fOuterTangent1_outsetB; //     ""                   fOuterFrame1_outset   fOuterCorner_bisector      ""          fOuterCorner_bisector

    SkV3 fOuterTangent_inset;    //     ""                     ""                  fOuterFrame0               ""          fOuterFrame0

    SkV3 fInnerFrame0_outset;    //     ""                     ""                  fInnerCorner_outsetA    fCenter        fCenter
    SkV3 fInnerFrame0_inset;     // fInnerFrame_inset        fInnerFrame_inset     fInnerCorner_inset      fCenter        fCenter

    SkV3 fInnerFrame1_outset;    //     ""                     ""                  fInnerCorner_outsetB    fCenter        fCenter
    SkV3 fInnerFrame1_inset;     // fInnerFrame_inset        fInnerFrame_inset     fInnerCorner_inset      fCenter        fCenter

    SkV3 fInnerTangent0_outset;  // fInnerFrame0_outset      fInnerFrame0_outset   fInnerBisector          fCenter        fCenter
    SkV3 fInnerTangent1_outset;  // fInnerFrame1_outset      fInnerFrame1_outset   fInnerBisector          fCenter        fCenter
    SkV3 fInnerTangent_inset;    // fInnerCorner_inset       fInnerCorner_inset    fInnerCorner_inset      fCenter        fCenter
};

// Vertices that are inset relative to the outer rectangle are calculated in local coordinates,
// resolving any self-intersections in the orthogonal local basis, before mapping to device space
// as (x,y,w).
struct CornerInsets {
    SkV2 fOuterFrameLR_inset; // pos on outer rectangle's left or right edge, depending on corner
    SkV2 fOuterFrameTB_inset; // pos on outer rectangle's top or bottom edge, depending on corner
    SkV2 fOuterTangent_inset;

    SkV2 fInnerFrameLR_outset; // pos connecting to fOuterFrameLR_inset
    SkV2 fInnerFrameTB_outset; // pos connecting to fOuterFrameTB_inset
    SkV2 fInnerFrameLR_inset;
    SkV2 fInnerFrameTB_inset;

    SkV2 fInnerTangentLR_outset; // pos connecting tangent to fInnerFrameLR_outset
    SkV2 fInnerTangentTB_outset; // pos connecting tangent to fInnerFrameTB_outset

    SkV2 fInnerTangent_inset;
};

static const uint16_t kTL = 0;
static const uint16_t kTR = 20;
static const uint16_t kBR = 40;
static const uint16_t kBL = 60;
// All indices (no vertex duplicates for other attributes, though).
static const uint16_t kIndices[] = {
    // TL
    // Exterior AA ramp to outer frame
    kTL+0,  kTL+1,  kTL+7,    kTL+0,  kTL+7,  kTL+6,    kTL+6,  kTL+7,  kTL+8,
    kTL+6,  kTL+8,  kTL+10,   kTL+6,  kTL+10, kTL+9,    kTL+9,  kTL+10, kTL+11,
    kTL+9,  kTL+11, kTL+3,    kTL+11, kTL+4,  kTL+3,
    // Interior AA ramp of outer frame
    kTL+2,  kTL+0,  kTL+6,    kTL+2,  kTL+6,  kTL+12,   kTL+6,  kTL+9, kTL+12,
    kTL+12, kTL+9,  kTL+5,    kTL+9,  kTL+3,  kTL+5,
    // Constant coverage between outer and inner frames
    kTL+13, kTL+2,  kTL+17,   kTL+17, kTL+2,  kTL+12,   kTL+17, kTL+12, kTL+18,
    kTL+18, kTL+12, kTL+5,    kTL+5,  kTL+15, kTL+18,
    // AA ramp around inner frame
    kTL+14, kTL+13, kTL+17,   kTL+14, kTL+17, kTL+19,   kTL+19, kTL+17, kTL+18,
    kTL+19, kTL+18, kTL+16,   kTL+15, kTL+16, kTL+18,

    // TR
    kTR+0,  kTR+1,  kTR+7,    kTR+0,  kTR+7,  kTR+6,    kTR+6,  kTR+7,  kTR+8,
    kTR+6,  kTR+8,  kTR+10,   kTR+6,  kTR+10, kTR+9,    kTR+9,  kTR+10, kTR+11,
    kTR+9,  kTR+11, kTR+3,    kTR+11, kTR+4,  kTR+3,
    kTR+2,  kTR+0,  kTR+6,    kTR+2,  kTR+6,  kTR+12,   kTR+6,  kTR+9,  kTR+12,
    kTR+12, kTR+9,  kTR+5,    kTR+9,  kTR+3,  kTR+5,
    kTR+13, kTR+2,  kTR+17,   kTR+17, kTR+2,  kTR+12,   kTR+17, kTR+12, kTR+18,
    kTR+18, kTR+12, kTR+5,    kTR+5,  kTR+15, kTR+18,
    kTR+14, kTR+13, kTR+17,   kTR+14, kTR+17, kTR+19,   kTR+19, kTR+17, kTR+18,
    kTR+19, kTR+18, kTR+16,   kTR+15, kTR+16, kTR+18,
    // // BR
    kBR+0,  kBR+1,  kBR+7,    kBR+0,  kBR+7,  kBR+6,    kBR+6,  kBR+7,  kBR+8,
    kBR+6,  kBR+8,  kBR+10,   kBR+6,  kBR+10, kBR+9,    kBR+9,  kBR+10, kBR+11,
    kBR+9,  kBR+11, kBR+3,    kBR+11, kBR+4,  kBR+3,
    kBR+2,  kBR+0,  kBR+6,    kBR+2,  kBR+6,  kBR+12,   kBR+6,  kBR+9, kBR+12,
    kBR+12, kBR+9,  kBR+5,    kBR+9,  kBR+3,  kBR+5,
    kBR+13, kBR+2,  kBR+17,   kBR+17, kBR+2,  kBR+12,   kBR+17, kBR+12, kBR+18,
    kBR+18, kBR+12, kBR+5,    kBR+5,  kBR+15, kBR+18,
    kBR+14, kBR+13, kBR+17,   kBR+14, kBR+17, kBR+19,   kBR+19, kBR+17, kBR+18,
    kBR+19, kBR+18, kBR+16,   kBR+15, kBR+16, kBR+18,
    // // BL
    kBL+0,  kBL+1,  kBL+7,    kBL+0,  kBL+7,  kBL+6,    kBL+6,  kBL+7,  kBL+8,
    kBL+6,  kBL+8,  kBL+10,   kBL+6,  kBL+10, kBL+9,    kBL+9,  kBL+10, kBL+11,
    kBL+9,  kBL+11, kBL+3,    kBL+11, kBL+4,  kBL+3,
    kBL+2,  kBL+0,  kBL+6,    kBL+2,  kBL+6,  kBL+12,   kBL+6,  kBL+9, kBL+12,
    kBL+12, kBL+9,  kBL+5,    kBL+9,  kBL+3,  kBL+5,
    kBL+13, kBL+2,  kBL+17,   kBL+17, kBL+2,  kBL+12,   kBL+17, kBL+12, kBL+18,
    kBL+18, kBL+12, kBL+5,    kBL+5,  kBL+15, kBL+18,
    kBL+14, kBL+13, kBL+17,   kBL+14, kBL+17, kBL+19,   kBL+19, kBL+17, kBL+18,
    kBL+19, kBL+18, kBL+16,   kBL+15, kBL+16, kBL+18,

    // // TL->TR
    kTL+3,  kTL+4,  kTR+0,    kTL+4,  kTR+1,  kTR+0,    kTL+3,  kTR+0,  kTR+2,
    kTL+3,  kTR+2,  kTL+5,    kTL+5,  kTR+2,  kTR+13,   kTL+5,  kTR+13, kTL+15,
    kTL+15, kTR+13, kTR+14,   kTL+15, kTR+14, kTL+16,

    // // TR->BR
    kTR+3,  kTR+4,  kBR+0,    kTR+4,  kBR+1,  kBR+0,    kTR+3,  kBR+0,  kBR+2,
    kTR+3,  kBR+2,  kTR+5,    kTR+5,  kBR+2,  kBR+13,   kTR+5,  kBR+13, kTR+15,
    kTR+15, kBR+13, kBR+14,   kTR+15, kBR+14, kTR+16,
    // // BR->BL
    kBR+3,  kBR+4,  kBL+0,    kBR+4,  kBL+1,  kBL+0,    kBR+3,  kBL+0,  kBL+2,
    kBR+3,  kBL+2,  kBR+5,    kBR+5,  kBL+2,  kBL+13,   kBR+5,  kBL+13, kBR+15,
    kBR+15, kBL+13, kBL+14,   kBR+15, kBL+14, kBR+16,
    // // BL->TL
    kBL+3,  kBL+4,  kTL+0,    kBL+4,  kTL+1,  kTL+0,    kBL+3,  kTL+0,  kTL+2,
    kBL+3,  kTL+2,  kBL+5,    kBL+5,  kTL+2,  kTL+13,   kBL+5,  kTL+13, kBL+15,
    kBL+15, kTL+13, kTL+14,   kBL+15, kTL+14, kBL+16,
};

// Separated to draw with different colors (vs. duplicating vertices to change colors).
static const uint16_t kOuterCornerIndices[] = {
    kTL+0,  kTL+1,  kTL+7,    kTL+0,  kTL+7,  kTL+6,    kTL+6,  kTL+7,  kTL+8,
    kTL+6,  kTL+8,  kTL+10,   kTL+6,  kTL+10, kTL+9,    kTL+9,  kTL+10, kTL+11,
    kTL+9,  kTL+11, kTL+3,    kTL+11, kTL+4,  kTL+3,
    kTL+2,  kTL+0,  kTL+6,    kTL+2,  kTL+6,  kTL+12,   kTL+6,  kTL+9, kTL+12,
    kTL+12, kTL+9,  kTL+5,    kTL+9,  kTL+3,  kTL+5,

    kTR+0,  kTR+1,  kTR+7,    kTR+0,  kTR+7,  kTR+6,    kTR+6,  kTR+7,  kTR+8,
    kTR+6,  kTR+8,  kTR+10,   kTR+6,  kTR+10, kTR+9,    kTR+9,  kTR+10, kTR+11,
    kTR+9,  kTR+11, kTR+3,    kTR+11, kTR+4,  kTR+3,
    kTR+2,  kTR+0,  kTR+6,    kTR+2,  kTR+6,  kTR+12,   kTR+6,  kTR+9,  kTR+12,
    kTR+12, kTR+9,  kTR+5,    kTR+9,  kTR+3,  kTR+5,

    kBR+0,  kBR+1,  kBR+7,    kBR+0,  kBR+7,  kBR+6,    kBR+6,  kBR+7,  kBR+8,
    kBR+6,  kBR+8,  kBR+10,   kBR+6,  kBR+10, kBR+9,    kBR+9,  kBR+10, kBR+11,
    kBR+9,  kBR+11, kBR+3,    kBR+11, kBR+4,  kBR+3,
    kBR+2,  kBR+0,  kBR+6,    kBR+2,  kBR+6,  kBR+12,   kBR+6,  kBR+9, kBR+12,
    kBR+12, kBR+9,  kBR+5,    kBR+9,  kBR+3,  kBR+5,

    kBL+0,  kBL+1,  kBL+7,    kBL+0,  kBL+7,  kBL+6,    kBL+6,  kBL+7,  kBL+8,
    kBL+6,  kBL+8,  kBL+10,   kBL+6,  kBL+10, kBL+9,    kBL+9,  kBL+10, kBL+11,
    kBL+9,  kBL+11, kBL+3,    kBL+11, kBL+4,  kBL+3,
    kBL+2,  kBL+0,  kBL+6,    kBL+2,  kBL+6,  kBL+12,   kBL+6,  kBL+9, kBL+12,
    kBL+12, kBL+9,  kBL+5,    kBL+9,  kBL+3,  kBL+5,
};

static const uint16_t kInnerCornerIndices[] = {
    kTL+14, kTL+13, kTL+17,   kTL+14, kTL+17, kTL+19,   kTL+19, kTL+17, kTL+18,
    kTL+19, kTL+18, kTL+16,   kTL+15, kTL+16, kTL+18,

    kTR+14, kTR+13, kTR+17,   kTR+14, kTR+17, kTR+19,   kTR+19, kTR+17, kTR+18,
    kTR+19, kTR+18, kTR+16,   kTR+15, kTR+16, kTR+18,

    kBR+14, kBR+13, kBR+17,   kBR+14, kBR+17, kBR+19,   kBR+19, kBR+17, kBR+18,
    kBR+19, kBR+18, kBR+16,   kBR+15, kBR+16, kBR+18,

    kBL+14, kBL+13, kBL+17,   kBL+14, kBL+17, kBL+19,   kBL+19, kBL+17, kBL+18,
    kBL+19, kBL+18, kBL+16,   kBL+15, kBL+16, kBL+18,
};

static const uint16_t kConstantCoverageIndices[] = {
    kTL+13, kTL+2,  kTL+17,   kTL+17, kTL+2,  kTL+12,   kTL+17, kTL+12, kTL+18,
    kTL+18, kTL+12, kTL+5,    kTL+5,  kTL+15, kTL+18,

    kTR+13, kTR+2,  kTR+17,   kTR+17, kTR+2,  kTR+12,   kTR+17, kTR+12, kTR+18,
    kTR+18, kTR+12, kTR+5,    kTR+5,  kTR+15, kTR+18,

    kBR+13, kBR+2,  kBR+17,   kBR+17, kBR+2,  kBR+12,   kBR+17, kBR+12, kBR+18,
    kBR+18, kBR+12, kBR+5,    kBR+5,  kBR+15, kBR+18,

    kBL+13, kBL+2,  kBL+17,   kBL+17, kBL+2,  kBL+12,   kBL+17, kBL+12, kBL+18,
    kBL+18, kBL+12, kBL+5,    kBL+5,  kBL+15, kBL+18,

    kTL+5,  kTR+2,  kTR+13,   kTL+5,  kTR+13, kTL+15,
    kTR+5,  kBR+2,  kBR+13,   kTR+5,  kBR+13, kTR+15,
    kBR+5,  kBL+2,  kBL+13,   kBR+5,  kBL+13, kBR+15,
    kBL+5,  kTL+2,  kTL+13,   kBL+5,  kTL+13, kBL+15,
};

static const uint16_t kLinearIndices[] = {
    kTL+3,  kTL+4,  kTR+0,    kTL+4,  kTR+1,  kTR+0,    kTL+3,  kTR+0,  kTR+2,
    kTL+3,  kTR+2,  kTL+5,


    kTL+15, kTR+13, kTR+14,   kTL+15, kTR+14, kTL+16,

    kTR+3,  kTR+4,  kBR+0,    kTR+4,  kBR+1,  kBR+0,    kTR+3,  kBR+0,  kBR+2,
    kTR+3,  kBR+2,  kTR+5,
    kTR+15, kBR+13, kBR+14,   kTR+15, kBR+14, kTR+16,

    kBR+3,  kBR+4,  kBL+0,    kBR+4,  kBL+1,  kBL+0,    kBR+3,  kBL+0,  kBL+2,
    kBR+3,  kBL+2,  kBR+5,
    kBR+15, kBL+13, kBL+14,   kBR+15, kBL+14, kBR+16,

    kBL+3,  kBL+4,  kTL+0,    kBL+4,  kTL+1,  kTL+0,    kBL+3,  kTL+0,  kTL+2,
    kBL+3,  kTL+2,  kBL+5,
    kBL+15, kTL+13, kTL+14,   kBL+15, kTL+14, kBL+16,
};

static bool check_crossover(float test, float limit, float sign) {
    return (sign * test) >= (sign * limit);
}
static bool clamp_if_crossed(SkV2& a, SkV2& b, const SkV2& xySign) {
    auto clamp = [](float a, float b, float sign) {
        return sign * std::max(sign*a, sign*b);
    };
    if (check_crossover(a.x, b.x, xySign.x) || check_crossover(b.y, a.y, xySign.y)) {
        SkV2 clamped = {clamp(a.x, b.x, xySign.x), clamp(a.y, b.y, xySign.y)};
        a = clamped;
        b = clamped;
        return true;
    } else {
        return false;
    }
}
static bool point_crosses_line(const SkV2& l0, const SkV2& l1, const SkV2& p, float sign) {
    if (l0 == l1) { return false; }
    float a = l0.y - l1.y;
    float b = l1.x - l0.x;
    float c = l0.x*l1.y - l0.y*l1.x;
    float d = a*p.x + b*p.y + c;
    return sign * d <= 0.f;
}

static void fill_outer_frame_insets(const SkM44& m,
                                    const SkV2& frameLR, const SkV2& frameTB,
                                    const SkV2& xySign,
                                    bool bevel,
                                    CornerInsets* out) {
    out->fOuterFrameLR_inset = frameLR + local_aa_radius(m, frameLR) * SkV2{xySign.x, 0.f};
    out->fOuterFrameTB_inset = frameTB + local_aa_radius(m, frameTB) * SkV2{0.f, xySign.y};

    if (clamp_if_crossed(out->fOuterFrameLR_inset, out->fOuterFrameTB_inset, xySign)) {
        // Crossed over, so clamp the tangent to the same position
        out->fOuterTangent_inset = out->fOuterFrameLR_inset;
    } else {
        // No cross over, so there's a valid tangent direction that we can inset from
        // (note that for rectangle corners, we always "cross" over so never calculate the tangent).
        float tangentOffset = bevel ? 0.5f : SK_ScalarRoot2Over2;
        SkV2 tangentPoint = SkV2{frameTB.x, frameLR.y} -
                            tangentOffset * SkV2{frameTB.x - frameLR.x, frameLR.y - frameTB.y};
        float tangentRadius = local_aa_radius(m, tangentPoint);

        out->fOuterTangent_inset = tangentPoint + tangentRadius * SK_ScalarRoot2Over2 * xySign;

        if (bevel) {
            // Adjust the outer frame insets instead of the tangent(?) or in addition to?
            SkV2 adjustedLRInset = frameLR + SK_ScalarSqrt2 * tangentRadius * SkV2{xySign.x, 0.f};
            SkV2 adjustedTBInset = frameTB + SK_ScalarSqrt2 * tangentRadius * SkV2{0.f, xySign.y};
            if (check_crossover(adjustedLRInset.x, out->fOuterFrameLR_inset.x, xySign.x)) {
                out->fOuterFrameLR_inset = adjustedLRInset;
            }
            if (check_crossover(adjustedTBInset.y, out->fOuterFrameTB_inset.y, xySign.y)) {
                out->fOuterFrameTB_inset = adjustedTBInset;
            }

            if (clamp_if_crossed(out->fOuterFrameLR_inset, out->fOuterFrameTB_inset, xySign)) {
                out->fOuterTangent_inset = out->fOuterFrameLR_inset;
            }
        }
    }
}

static void fill_inner_with_center(const SkV2& center, CornerInsets* out) {
    out->fInnerFrameLR_inset = center;
    out->fInnerFrameLR_outset = center;
    out->fInnerFrameTB_inset = center;
    out->fInnerFrameTB_outset = center;

    out->fInnerTangent_inset = center;
    out->fInnerTangentLR_outset = center;
    out->fInnerTangentTB_outset = center;
}

static std::pair<bool, SkV2> fill_inner_insets(const SkM44& m,
                              const SkV2& frameLR, const SkV2& frameTB,
                              const SkV2& xySign, const SkV2& xyInset,
                              CornerInsets* out) {
    SkV2 innerFrameLR = frameLR + SkV2{xySign.x * xyInset.x, 0.f};
    SkV2 innerFrameTB = frameTB + SkV2{0.f, xySign.y * xyInset.y};
    bool interiorIsRect = clamp_if_crossed(innerFrameLR, innerFrameTB, xySign);

    CornerInsets inner;
    fill_outer_frame_insets(m, innerFrameLR, innerFrameTB, xySign, /*bevel=*/false, &inner);
    out->fInnerFrameLR_inset = inner.fOuterFrameLR_inset;
    out->fInnerFrameTB_inset = inner.fOuterFrameTB_inset;
    // does the right thing even if interior is rect
    out->fInnerTangent_inset = inner.fOuterTangent_inset;

    out->fInnerFrameLR_outset = innerFrameLR - local_aa_radius(m, innerFrameLR) * SkV2{xySign.x, 0.f};
    out->fInnerFrameTB_outset = innerFrameTB - local_aa_radius(m, innerFrameTB) * SkV2{0.f, xySign.y};

    if (interiorIsRect) {
        // The inner tangent outsets are co-located at outer corner formed by innerFrameLR/TB_outset
        SkV2 corner = {out->fInnerFrameLR_outset.x, out->fInnerFrameTB_outset.y};
        out->fInnerTangentLR_outset = out->fInnerTangentTB_outset = corner;
        out->fInnerFrameLR_outset = out->fInnerFrameTB_outset = corner;
        return {true, {innerFrameLR.x, innerFrameTB.y}};
    } else {
        // The interior has a curve, so we can calculate the tangent intersections
        SkV2 innerCorner = {innerFrameTB.x, innerFrameLR.y};
        float xRadius = std::abs(innerCorner.x - innerFrameLR.x);
        float yRadius = std::abs(innerCorner.y - innerFrameTB.y);
        // TODO: Assert? This math really only works if the curve was circular anyways.
        SkV2 tangentPoint = {innerCorner.x - xySign.x * xRadius * SK_ScalarRoot2Over2,
                             innerCorner.y - xySign.y * yRadius * SK_ScalarRoot2Over2};
        float tangentRadius = local_aa_radius(m, tangentPoint);

        float lrRadius = local_aa_radius(m, out->fInnerFrameLR_outset);
        float tbRadius = local_aa_radius(m, out->fInnerFrameTB_outset);

        float lrOffset = (SK_ScalarRoot2Over2 - 1.f) * xRadius + SK_ScalarSqrt2 * tangentRadius - lrRadius + SK_ScalarRoot2Over2 * yRadius;
        float tbOffset = (SK_ScalarRoot2Over2 - 1.f) * yRadius + SK_ScalarSqrt2 * tangentRadius - tbRadius + SK_ScalarRoot2Over2 * xRadius;

        out->fInnerTangentLR_outset = out->fInnerFrameLR_outset - lrOffset * SkV2{0.f, xySign.y};
        out->fInnerTangentTB_outset = out->fInnerFrameTB_outset - tbOffset * SkV2{xySign.x, 0.f};
        return {false, SkV2()};
    }
}

static CornerInsets compute_insets(const SkM44& m, const SkV2& frameLR, const SkV2& frameTB,
                                   const SkV2& xySign, const SkV2& xyInset, const SkV2& center,
                                   bool bevelOuterTangentInset) {
    CornerInsets insets;
    fill_outer_frame_insets(m, frameLR, frameTB, xySign, bevelOuterTangentInset, &insets);
    bool interiorIsRect = false;
    SkV2 interiorRectCorner;
    if (xyInset.x >= 0.f && xyInset.y >= 0.f) {
        // A stroked interior
        std::tie(interiorIsRect, interiorRectCorner) =
                fill_inner_insets(m, frameLR, frameTB, xySign, xyInset, &insets);
        if (check_crossover(insets.fInnerTangent_inset.x, center.x, xySign.x) ||
            check_crossover(insets.fInnerTangent_inset.y, center.y, xySign.y)) {
            // Interior collapses to center
            fill_inner_with_center(center, &insets);
            interiorIsRect = false;
        }
    } else {
        fill_inner_with_center(center, &insets);
    }

    // Now check if the outer frame's insets overlap with the inner insets, and if so, snap
    // everything to the interior (which also handles the center overlap since the inner
    // has already been snapped to the center).
    float sign = xySign.x * xySign.y;
    if (check_crossover(insets.fOuterFrameLR_inset.x, insets.fInnerFrameLR_outset.x, xySign.x) ||
        check_crossover(insets.fOuterFrameTB_inset.y, insets.fInnerFrameTB_outset.y, xySign.y)) {


        insets.fInnerTangentLR_outset = insets.fInnerTangent_inset;
        insets.fInnerTangentTB_outset = insets.fInnerTangent_inset;

        if (interiorIsRect) {
            insets.fOuterFrameLR_inset = interiorRectCorner;
            insets.fOuterFrameTB_inset = interiorRectCorner;
            insets.fInnerFrameLR_outset = interiorRectCorner;
            insets.fInnerFrameTB_outset = interiorRectCorner;

            insets.fOuterTangent_inset = interiorRectCorner;
        } else {
            insets.fOuterFrameLR_inset = insets.fInnerFrameLR_inset;
            insets.fOuterFrameTB_inset = insets.fInnerFrameTB_inset;
            insets.fInnerFrameLR_outset = insets.fInnerFrameLR_inset;
            insets.fInnerFrameTB_outset = insets.fInnerFrameTB_inset;

            insets.fOuterTangent_inset = insets.fInnerTangent_inset;
        }
    } else if (point_crosses_line(insets.fOuterFrameLR_inset, insets.fOuterTangent_inset, insets.fInnerTangentLR_outset, sign) ||
               point_crosses_line(insets.fOuterTangent_inset, insets.fOuterFrameTB_inset, insets.fInnerTangentTB_outset, sign)) {
        // Collapse interior of corner in such a way to preserve the uncollapsed edges between
        // outer frame insets and inner frame outsets.
        insets.fInnerTangentLR_outset = insets.fOuterTangent_inset;
        insets.fInnerTangentTB_outset = insets.fOuterTangent_inset;
        insets.fInnerFrameLR_outset = insets.fOuterFrameLR_inset;
        insets.fInnerFrameTB_outset = insets.fOuterFrameTB_inset;
    }

    return insets;
}

static void compute_outer_rect_corner(const SkM44& m,
                                      const SkV2& corner,
                                      const SkV2& prevCorner,
                                      const SkV2& nextCorner,
                                      CornerVerts* out) {
    SkV3 v = map(m, corner);
    SkV3 e0 = line_between_homogenous_points(map(m, prevCorner), v); // opposite direction to localE0
    SkV3 e1 = line_between_homogenous_points(v, map(m, nextCorner));

    // Shift both edges out by 10 pixels (for increased visibility)
    SkV3 outsetE0 = shift_line(e0, kAARadius);
    SkV3 outsetE1 = shift_line(e1, kAARadius);

    // Compute the normals for each edge through the specific corner.
    SkV3 n0 = perpendicular_through_point(e0, v);
    SkV3 n1 = perpendicular_through_point(e1, v);

    // Outset vertices are the intersection between the normal line and the outset edge
    SkV3 outsetV0 = line_intersection(outsetE0, n0);
    SkV3 outsetV1 = line_intersection(outsetE1, n1);

    // Outer frame and tangent coords are at rectangle corner
    out->fOuterFrame0 = v;
    out->fOuterFrame1 = v;
    out->fOuterTangent0 = v;
    out->fOuterTangent1 = v;

    // Outer frame/tangent outsets are outset along normal vector
    out->fOuterFrame0_outset = outsetV0;
    out->fOuterTangent0_outsetA = outsetV0;
    out->fOuterFrame1_outset = outsetV1;
    out->fOuterTangent1_outsetA = outsetV1;

    // Secondary outer tangent outsets are bisector between normal vectors
    out->fOuterTangent0_outsetB = out->fOuterTangent1_outsetB = bisection(e0, e1, v, kAARadius);
}

static std::pair<SkV2, SkV2> compute_outer_fancy_corner(const SkM44& m, const SkV2& corner, const SkV2& xySign, const SkV2& xyRadii, float tangentOffset, CornerVerts* out) {
    SkV2 lrOffset = {0.f, xySign.y * xyRadii.y};
    SkV2 tbOffset = {xySign.x * xyRadii.x, 0.f};
    SkV2 localOuterFrameLR = corner + lrOffset;
    SkV2 localOuterFrameTB = corner + tbOffset;

    SkV2 localOuterTangentLR = localOuterFrameLR - tangentOffset * lrOffset;
    SkV2 localOuterTangentTB = localOuterFrameTB - tangentOffset * tbOffset;

    if (xySign.x * xySign.y >= 0.f) {
        // LR is v0 and TB is v1
        out->fOuterFrame0 = map(m, localOuterFrameLR);
        out->fOuterFrame1 = map(m, localOuterFrameTB);
        out->fOuterTangent0 = map(m, localOuterTangentLR);
        out->fOuterTangent1 = map(m, localOuterTangentTB);
    } else {
        // TB is v0 and LR is v1
        out->fOuterFrame0 = map(m, localOuterFrameTB);
        out->fOuterFrame1 = map(m, localOuterFrameLR);
        out->fOuterTangent0 = map(m, localOuterTangentTB);
        out->fOuterTangent1 = map(m, localOuterTangentLR);
    }

    SkV3 outerCorner = map(m, corner);
    SkV3 e0 = line_between_homogenous_points(out->fOuterFrame0, outerCorner);
    SkV3 e1 = line_between_homogenous_points(outerCorner, out->fOuterFrame1);
    SkV3 tangent = line_between_homogenous_points(out->fOuterTangent0, out->fOuterTangent1);
    // Shift both edges out by 10 pixels (for increased visibility)
    SkV3 outsetE0 = shift_line(e0, kAARadius);
    SkV3 outsetE1 = shift_line(e1, kAARadius);
    SkV3 outsetTangent = shift_line(tangent, kAARadius);

    // Normal outsets
    out->fOuterFrame0_outset = line_intersection(outsetE0, perpendicular_through_point(e0, out->fOuterFrame0));
    out->fOuterFrame1_outset = line_intersection(outsetE1, perpendicular_through_point(e1, out->fOuterFrame1));
    if (tangentOffset <= 0.f) {
        out->fOuterTangent0_outsetA = bisection(outsetE0, outsetTangent, out->fOuterTangent0, kAARadius);
        out->fOuterTangent1_outsetA = bisection(outsetTangent, outsetE1, out->fOuterTangent1, kAARadius);
    } else {
        out->fOuterTangent0_outsetA = line_intersection(outsetE0, perpendicular_through_point(e0, out->fOuterTangent0));
        out->fOuterTangent1_outsetA = line_intersection(outsetE1, perpendicular_through_point(e1, out->fOuterTangent1));
    }
    out->fOuterTangent0_outsetB = line_intersection(outsetTangent, perpendicular_through_point(tangent, out->fOuterTangent0));
    out->fOuterTangent1_outsetB = line_intersection(outsetTangent, perpendicular_through_point(tangent, out->fOuterTangent1));

    return {localOuterFrameLR, localOuterFrameTB};
}

static std::pair<SkV2, SkV2> compute_outer_round_corner(const SkM44& m, const SkV2& corner, const SkV2& xySign, const SkV2& xyRadii, CornerVerts* out) {
    return compute_outer_fancy_corner(m, corner, xySign, xyRadii, SK_FloatSqrt2 - 1.f, out);
}
static std::pair<SkV2, SkV2> compute_outer_bevel_corner(const SkM44& m, const SkV2& corner, const SkV2& xySign, const SkV2& xyRadii, CornerVerts* out) {
    return compute_outer_fancy_corner(m, corner, xySign, xyRadii, 0.f, out);
}

// Assumes corners are referring to the exterior bounding box, so it includes the stroke radius
// already if it's stroked.
// TODO: Need a way to report how the outer and inner corner verts collapse to change coverage type.
static CornerVerts compute_corner(const SkM44& m,
                                  const SkV2& corner,
                                  const SkV2& prevCorner,
                                  const SkV2& nextCorner,
                                  const SkV2& center,
                                  const SkV2& xyRadii,
                                  float strokeRadius,
                                  float joinOrMiterLimit) {
    auto [v, min, max] = minmax_scales(m, corner);
    const bool rectCorner = SkScalarNearlyZero(xyRadii.x * max) ||
                            SkScalarNearlyZero(xyRadii.y * max);

    const SkV2 strokeInset{strokeRadius, strokeRadius};
    const SkV2 xySign = {std::abs(corner.x - prevCorner.x) > std::abs(corner.x - nextCorner.x) ?
                                (corner.x > prevCorner.x ? -1.f : 1.f) :
                                (corner.x > nextCorner.x ? -1.f : 1.f),
                         std::abs(corner.y - prevCorner.y) > std::abs(corner.y - nextCorner.y) ?
                                (corner.y > prevCorner.y ? -1.f : 1.f) :
                                (corner.y > nextCorner.y ? -1.f : 1.f)};

    SkV2 frameLR, frameTB;
    CornerVerts out;
    bool bevel = false;
    if (strokeRadius >= 0.f) {
        // Stroked geometry
        if (rectCorner) {
            // Have to handle miter, bevel, or round joins for outer geometry, inner geometry is
            // always rectangular.
            if (joinOrMiterLimit < 0.f) {
                // Round joins have a circular corner with radius equal to stroke radius
                std::tie(frameLR, frameTB) =
                        compute_outer_round_corner(m, corner, xySign, strokeInset, &out);
            } else if (joinOrMiterLimit >= SK_ScalarSqrt2) {
                // Mitered joins have a rectangular corner that's outset by stroke radius
                compute_outer_rect_corner(m, corner, prevCorner, nextCorner, &out);
                frameLR = frameTB = corner;
            } else {
                // Beveled join, which has unique geometry that's somewhat similar to rounded case
                // FIXME hairline bevel is undefined and produces bad business
                std::tie(frameLR, frameTB) =
                        compute_outer_bevel_corner(m, corner, xySign, strokeInset, &out);
                bevel = true;
            }
        } else {
            // Outer geometry is a curve with radius inflated by stroke radius. Inner geometry is
            // either a curve deflated by stroke radius, or rectangular.
            std::tie(frameLR, frameTB) =
                    compute_outer_round_corner(m, corner, xySign, xyRadii + strokeInset, &out);
        }
    } else {
        // Filled geometry (ignore stroke radius and join/miter limits)
        if (rectCorner) {
            // Rectangular corner
            compute_outer_rect_corner(m, corner, prevCorner, nextCorner, &out);
            frameLR = frameTB = corner;
        } else {
            // Rounded corner
            std::tie(frameLR, frameTB) =
                    compute_outer_round_corner(m, corner, xySign, xyRadii, &out);
        }
    }

    // Calculate local insets in XY orientation, then swizzle to maintain expected CW winding
    // for the output vertices (depending on corner), and transform by m, storing in 'out'.
    CornerInsets insets = compute_insets(m, frameLR, frameTB, xySign, 2 * strokeInset, center, bevel);
    if (xySign.x * xySign.y > 0.f) {
        // LR refers to v0 and TB refers to v1
        out.fOuterFrame0_inset = map(m, insets.fOuterFrameLR_inset);
        out.fOuterFrame1_inset = map(m, insets.fOuterFrameTB_inset);
        out.fInnerFrame0_inset = map(m, insets.fInnerFrameLR_inset);
        out.fInnerFrame1_inset = map(m, insets.fInnerFrameTB_inset);
        out.fInnerFrame0_outset = map(m, insets.fInnerFrameLR_outset);
        out.fInnerFrame1_outset = map(m, insets.fInnerFrameTB_outset);
        out.fInnerTangent0_outset = map(m, insets.fInnerTangentLR_outset);
        out.fInnerTangent1_outset = map(m, insets.fInnerTangentTB_outset);
    } else {
        // TB refers to v0 and LR refers to v1
        out.fOuterFrame0_inset = map(m, insets.fOuterFrameTB_inset);
        out.fOuterFrame1_inset = map(m, insets.fOuterFrameLR_inset);
        out.fInnerFrame0_inset = map(m, insets.fInnerFrameTB_inset);
        out.fInnerFrame1_inset = map(m, insets.fInnerFrameLR_inset);
        out.fInnerFrame0_outset = map(m, insets.fInnerFrameTB_outset);
        out.fInnerFrame1_outset = map(m, insets.fInnerFrameLR_outset);
        out.fInnerTangent0_outset = map(m, insets.fInnerTangentTB_outset);
        out.fInnerTangent1_outset = map(m, insets.fInnerTangentLR_outset);
    }
    out.fInnerTangent_inset = map(m, insets.fInnerTangent_inset);
    out.fOuterTangent_inset = map(m, insets.fOuterTangent_inset);
    return out;
}

class GraphitePrimitivesSample : public Sample {
    static constexpr float kControlPointRadius = 3.f;
    static constexpr float kBaseScale = 50.f;
public:
    GraphitePrimitivesSample()
        : fOrigin{300.f, 300.f}
        , fXAxisPoint{300.f + kBaseScale, 300.f}
        , fYAxisPoint{300.f, 300.f + kBaseScale}
        , fStrokeWidth{10.f}
        , fJoinMode(SkPaint::kMiter_Join)
        , fMode(PrimitiveMode::kFillRect) {}

    void onDrawContent(SkCanvas* canvas) override {
        canvas->save();
        canvas->concat(this->basisMatrix());

        SkM44 totalMatrix = canvas->getLocalToDevice();

            // Base shape + style
            canvas->drawRRect(this->primitiveShape(), paint(SK_ColorBLUE, this->strokeWidth(), fJoinMode));

            // Local circle (for visualization purposes, the smaller the radius, the more accurate the
            // perspective derivatives remain at the edge).
            // auto [devCoord, min, max] = minmax_scales(totalMatrix, {0.f, 0.f});
            // canvas->drawCircle({0.f, 0.f}, 10.f / min, paint(SK_ColorDKGRAY));
            // canvas->drawCircle({0.f, 0.f}, 10.f, paint(SK_ColorDKGRAY));
        canvas->restore();

        canvas->save();
        canvas->resetMatrix();
            // Visualize minmax scales
            // canvas->drawCircle({devCoord.x / devCoord.z, devCoord.y / devCoord.z}, 10.f * min, paint(SK_ColorLTGRAY, 0.f));
            // canvas->drawCircle({devCoord.x / devCoord.z, devCoord.y / devCoord.z}, 10.f * max, paint(SK_ColorLTGRAY, 0.f));
            // canvas->drawCircle({devCoord.x / devCoord.w, devCoord.y / devCoord.w}, 10.f, paint(SK_ColorRED, 0.f));
            this->drawVertices(canvas, totalMatrix);
        canvas->restore();

        // Axes
        canvas->drawLine({fOrigin.x, fOrigin.y}, {fXAxisPoint.x, fXAxisPoint.y}, paint(SK_ColorBLACK, 2.f));
        canvas->drawLine({fOrigin.x, fOrigin.y}, {fYAxisPoint.x, fYAxisPoint.y}, paint(SK_ColorBLACK, 2.f));

        // Clickable control points for axes
        canvas->drawCircle({fOrigin.x, fOrigin.y}, kControlPointRadius, paint(SK_ColorBLACK));
        canvas->drawCircle({fXAxisPoint.x, fXAxisPoint.y}, kControlPointRadius, paint(SK_ColorBLACK));
        canvas->drawCircle({fYAxisPoint.x, fYAxisPoint.y}, kControlPointRadius, paint(SK_ColorBLACK));
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override;
    bool onClick(Sample::Click*) override;
    bool onChar(SkUnichar) override;
    SkString name() override { return SkString("GraphitePrimitives"); }

private:
    class Click;

    enum class PrimitiveMode {
        kFillRect,
        kFillRRect,
        kStrokeRect,
        kStrokeRRect
    };

    // Computed from 3 control points. Concat with CTM to get total matrix.
    SkM44 basisMatrix() const {
        SkV2 xAxis = (fXAxisPoint - fOrigin) / kBaseScale;
        SkV2 yAxis = (fYAxisPoint - fOrigin) / kBaseScale;

        return SkM44::Cols({xAxis.x, xAxis.y, 0.f, 0.f},
                           {yAxis.x, yAxis.y, 0.f, 0.f},
                           {0.f, 0.f, 1.f, 0.f},
                           {fOrigin.x, fOrigin.y, 0.f, 1.f});
    }

    float strokeWidth() const {
        if (fMode == PrimitiveMode::kFillRect || fMode == PrimitiveMode::kFillRRect) {
            return -1.f;
        }
        return fStrokeWidth;
    }

    SkRRect primitiveShape() const {
        static const SkRect kOuterBounds = SkRect::MakeLTRB(-kBaseScale, -kBaseScale,
                                                            kBaseScale, kBaseScale);
        // Filled rounded rects can have arbitrary corners
        static const SkVector kOuterRadii[4] = { { 0.25f * kBaseScale, 0.75f * kBaseScale },
                                                 { 0.f, 0.f},
                                                 { 0.5f * kBaseScale, 0.5f * kBaseScale },
                                                 { 0.75f * kBaseScale, 0.25f * kBaseScale } };
        // Stroked rounded rects will only have circular corners
        static const SkVector kStrokeRadii[4] = { { 0.25f * kBaseScale, 0.25f * kBaseScale },
                                                  { 0.f, 0.f },
                                                  { 0.5f * kBaseScale, 0.5f * kBaseScale },
                                                  { 0.75f * kBaseScale, 0.75f * kBaseScale } };

        float strokeRadius = 0.5f * fStrokeWidth;
        switch(fMode) {
            case PrimitiveMode::kFillRect:
                return SkRRect::MakeRect(kOuterBounds.makeOutset(strokeRadius, strokeRadius));
            case PrimitiveMode::kFillRRect: {
                SkRRect rrect;
                rrect.setRectRadii(kOuterBounds, kOuterRadii);
                rrect.outset(strokeRadius, strokeRadius);
                return rrect; }
            case PrimitiveMode::kStrokeRect:
                return SkRRect::MakeRect(kOuterBounds);
            case PrimitiveMode::kStrokeRRect: {
                SkRRect rrect;
                rrect.setRectRadii(kOuterBounds, kStrokeRadii);
                return rrect;
            }
        }

        SkUNREACHABLE;
    }



    void drawVertices(SkCanvas* canvas, const SkM44& ctm) {
        auto projectP3 = [](const SkV3& p) {
            return SkPoint{p.x/p.z, p.y/p.z};
        };

        SkRRect rrect = this->primitiveShape();
        SkRect outer = rrect.getBounds();
        float strokeRadius = 0.5f * this->strokeWidth();
        if (strokeRadius > 0.f) {
            // Everything is computed relative to the outer bounding box, including the stroke.
            outer.outset(strokeRadius, strokeRadius);
        }

        float joinType;
        if (strokeRadius >= 0.f) {
            joinType = (fJoinMode == SkPaint::kRound_Join ? -1.f :
                        (fJoinMode == SkPaint::kBevel_Join ? 0.f : SK_ScalarSqrt2));
        } else {
            joinType = -1.f; // will be filled, so join type is ignored in compute_corner
        }

        SkV2 center = {outer.centerX(), outer.centerY()};
        SkV2 localCorners[4] = {{outer.fLeft, outer.fTop}, {outer.fRight, outer.fTop},
                                {outer.fRight, outer.fBottom}, {outer.fLeft, outer.fBottom}};
        CornerVerts corners[4]; // TL, TR, BR, BL
        corners[0] = compute_corner(ctm, localCorners[0], localCorners[3], localCorners[1], center,
                {rrect.radii(SkRRect::kUpperLeft_Corner).fX, rrect.radii(SkRRect::kUpperLeft_Corner).fY}, strokeRadius, joinType);
        corners[1] = compute_corner(ctm, localCorners[1], localCorners[0], localCorners[2], center,
                {rrect.radii(SkRRect::kUpperRight_Corner).fX, rrect.radii(SkRRect::kUpperRight_Corner).fY}, strokeRadius, joinType);
        corners[2] = compute_corner(ctm, localCorners[2], localCorners[1], localCorners[3], center,
                {rrect.radii(SkRRect::kLowerRight_Corner).fX, rrect.radii(SkRRect::kLowerRight_Corner).fY}, strokeRadius, joinType);
        corners[3] = compute_corner(ctm, localCorners[3], localCorners[2], localCorners[0], center,
                {rrect.radii(SkRRect::kLowerLeft_Corner).fX, rrect.radii(SkRRect::kLowerLeft_Corner).fY}, strokeRadius, joinType);

        SkPoint vertices[80];
        size_t i = 0;
        for (auto c : corners) {
            vertices[i + 0] = projectP3(c.fOuterFrame0);
            vertices[i + 1] = projectP3(c.fOuterFrame0_outset);
            vertices[i + 2] = projectP3(c.fOuterFrame0_inset);
            vertices[i + 3] = projectP3(c.fOuterFrame1);
            vertices[i + 4] = projectP3(c.fOuterFrame1_outset);
            vertices[i + 5] = projectP3(c.fOuterFrame1_inset);
            vertices[i + 6] = projectP3(c.fOuterTangent0);
            vertices[i + 7] = projectP3(c.fOuterTangent0_outsetA);
            vertices[i + 8] = projectP3(c.fOuterTangent0_outsetB);
            vertices[i + 9] = projectP3(c.fOuterTangent1);
            vertices[i + 10] = projectP3(c.fOuterTangent1_outsetB);
            vertices[i + 11] = projectP3(c.fOuterTangent1_outsetA);
            vertices[i + 12] = projectP3(c.fOuterTangent_inset);
            vertices[i + 13] = projectP3(c.fInnerFrame0_outset);
            vertices[i + 14] = projectP3(c.fInnerFrame0_inset);
            vertices[i + 15] = projectP3(c.fInnerFrame1_outset);
            vertices[i + 16] = projectP3(c.fInnerFrame1_inset);
            vertices[i + 17] = projectP3(c.fInnerTangent0_outset);
            vertices[i + 18] = projectP3(c.fInnerTangent1_outset);
            vertices[i + 19] = projectP3(c.fInnerTangent_inset);
            i += 20;
        }

        auto drawMeshSubset = [vertices, canvas](SkColor color, const uint16_t* indices, size_t indexCount) {
            sk_sp<SkVertices> mesh = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, 80, vertices, nullptr, nullptr, (int) indexCount, indices);
            SkPaint meshPaint;
            meshPaint.setColor(color);
            meshPaint.setAlphaf(0.75f);
            canvas->drawVertices(mesh, SkBlendMode::kSrc, meshPaint);
        };
        drawMeshSubset(SK_ColorLTGRAY, kLinearIndices, SK_ARRAY_COUNT(kLinearIndices));
        drawMeshSubset(SK_ColorDKGRAY, kConstantCoverageIndices, SK_ARRAY_COUNT(kConstantCoverageIndices));
        drawMeshSubset(SK_ColorMAGENTA, kInnerCornerIndices, SK_ARRAY_COUNT(kInnerCornerIndices));
        drawMeshSubset(SK_ColorCYAN, kOuterCornerIndices, SK_ARRAY_COUNT(kOuterCornerIndices));

        std::unordered_set<uint32_t> edges;
        auto drawEdge = [&edges, vertices, canvas](uint16_t e0, uint16_t e1) {
            uint32_t edgeID = (std::max(e0, e1) << 16) | std::min(e0, e1);
            if (edges.find(edgeID) == edges.end()) {
                edges.insert(edgeID);
                if (SkScalarNearlyEqual(vertices[e0].fX, vertices[e1].fX) &&
                    SkScalarNearlyEqual(vertices[e0].fY, vertices[e1].fY)) {
                    return;
                }
                canvas->drawLine(vertices[e0], vertices[e1], paint(SK_ColorBLACK, 0.f));
            }
        };
        for (i = 0; i < SK_ARRAY_COUNT(kIndices); i += 3) {
            drawEdge(kIndices[i+0], kIndices[i+1]);
            drawEdge(kIndices[i+1], kIndices[i+2]);
            drawEdge(kIndices[i+2], kIndices[i+0]);
        }
    }

    // This Sample is responsive to the entire transform of the viewer slide, including the
    // transform (rotation, scale, and perspective) selected from the widget. The 3 points below
    // define the location and basis of the local coordinate space, relative to the viewer's
    // coordinate space. This is used instead of the root canvas coordinate space because it aligns
    // with the coordinate space that the click handler operates in.
    SkV2 fOrigin;
    SkV2 fXAxisPoint;
    SkV2 fYAxisPoint;

    float fStrokeWidth;
    SkPaint::Join fJoinMode;
    PrimitiveMode fMode;
};

class GraphitePrimitivesSample::Click : public Sample::Click {
public:
    Click(SkV2* point) : fPoint(point) {}

    void drag() {
        SkVector delta = fCurr - fPrev;
        *fPoint += {delta.fX, delta.fY};
    }

private:
    SkV2* fPoint;
};

Sample::Click* GraphitePrimitivesSample::onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) {
    auto selected = [x,y](const SkV2& p) {
        return ((p - SkV2{x,y}).length() < kControlPointRadius);
    };

    if (selected(fOrigin)) {
        return new Click(&fOrigin);
    } else if (selected(fXAxisPoint)) {
        return new Click(&fXAxisPoint);
    } else if (selected(fYAxisPoint)) {
        return new Click(&fYAxisPoint);
    } else {
        return nullptr;
    }
}

bool GraphitePrimitivesSample::onClick(Sample::Click* click) {
    Click* myClick = (Click*) click;
    myClick->drag();
    return true;
}

bool GraphitePrimitivesSample::onChar(SkUnichar code) {
        switch(code) {
            case '1':
                fMode = PrimitiveMode::kFillRect;
                return true;
            case '2':
                fMode = PrimitiveMode::kFillRRect;
                return true;
            case '3':
                fMode = PrimitiveMode::kStrokeRect;
                return true;
            case '4':
                fMode = PrimitiveMode::kStrokeRRect;
                return true;
            case '-':
                fStrokeWidth = std::max(0.f, fStrokeWidth - 0.4f);
                return true;
            case '=':
                fStrokeWidth = std::min(5 * kBaseScale, fStrokeWidth + 0.4f);
                return true;
            case 'q':
                fJoinMode = SkPaint::kRound_Join;
                return true;
            case 'w':
                fJoinMode = SkPaint::kBevel_Join;
                return true;
            case 'e':
                fJoinMode = SkPaint::kMiter_Join;
                return true;
            case 'r':
                fStrokeWidth = 10.f;
                fOrigin = {300.f, 300.f};
                fXAxisPoint = {300.f + kBaseScale, 300.f};
                fYAxisPoint = {300.f, 300.f + kBaseScale};
                return true;
        }
        return false;
}

DEF_SAMPLE(return new GraphitePrimitivesSample();)
