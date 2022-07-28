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
#include "include/private/SkTPin.h"

static SkPaint paint(SkColor color, float strokeWidth = -1.f) {
    SkPaint paint;
    paint.setColor(color);
    paint.setAntiAlias(true);
    if (strokeWidth >= 0.f) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(strokeWidth);
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
    SkV3 fOuterTangent0_outsetA; //     ""                   fOuterFrame0_outset   fOuterFrame0_outset        ""          fOuterFrame0_outset
    SkV3 fOuterTangent0_outsetB; //     ""                   fOuterFrame0_outset   fOuterCorner_bisector      ""          fOuterCorner_bisector

    SkV3 fOuterTangent1;         //     ""                   fOuterFrame1          fOuterFrame0               ""          fOuterFrame0
    SkV3 fOuterTangent1_outsetA; //     ""                   fOuterFrame1_outset   fOuterFrame1_outset        ""          fOuterFrame1_outset
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

static constexpr float kAARadius = 10.f;

static void compute_outer_rect_corner(const SkM44& m, const SkV2& corner, const SkV2& prevCorner, const SkV2& nextCorner,
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
    out.fOuterFrame0 = v;
    out.fOuterFrame1 = v;
    out.fOuterTangent0 = v;
    out.fOuterTangent1 = v;
    out.fOuterTangent_inset = v;

    // Outer frame/tangent outsets are outset along normal vector
    out.fOuterFrame0_outset = outsetV0;
    out.fOuterTangent0_outsetA = outsetV0;
    out.fOuterFrame1_outset = outsetV1;
    out.fOuterTangent1_outsetA = outsetV1;

    // Secondary outer tangent outsets are bisector between normal vectors
    out.fOuterTangent0_outsetB = bisection(e0, e1, v, kAARadius);
    out.fOuterTangent1_outsetB = out.fOuterTangent0_outsetB;

    // FIXME how do we incorporate calculation of outer frame insets when the cross-over check
    // depends on either checking the stroked rect lines or the center (for fills)?
    // We could fill in the interior first and then base it on checking the V3s that were in the
    // calculated vertices, but the test is easier locally.
    //
    // Perhaps we need a intermediate struct that tracks all the SkV2 local positions that we want
    // to build up, and handle collapses on, and then that gets written to the final output by
    // simply mapping all the points?
}

static void compute_outer_round_corner(const SkM44& m, const SkV2& corner, const SkV2& prevCorner, const SkV2& nextCorner, float openingRadius, float closingRadius,
    CornerVerts* out) {

}

static CornerVerts compute_corner(const SkM44& m,
                                  const SkV2& corner,
                                  const SkV2& prevCorner,
                                  const SkV2& nextCorner,
                                  const SkV2& center,
                                  float openingRadius,
                                  float closingRadius,
                                  float strokeRadius,
                                  float joinOrMiterLimit) {
    static constexpr float kAARadius = 10.f;
    static constexpr float kTangentOffset = SK_FloatSqrt2 - 1.f;

    const SkV2 localE0 = (prevCorner - corner).normalize();
    const SkV2 localE1 = (nextCorner - corner).normalize();

    auto [v, min, max] = minmax_scales(m, corner);
    const bool rectCorner = SkScalarNearlyZero(openingRadius * max) || SkScalarNearlyZero(closingRadius * max);

    auto check_crossover = [](const SkV2& test, const SkV2& limit, const SkV2& direction) {
        float testDot = test.dot(direction);
        float limitDot = limit.dot(direction);
        return testDot > limitDot;
    };

    CornerVerts out;
    if (strokeRadius >= 0.f) {
        // Stroked geometry
        // TODO: Compute the outer corners that bound the stroked shape and use those instead

        if (rectCorner) {
            // Have to handle miter, bevel, or round joins for outer geometry, inner geometry is
            // always rectangular.
            if (joinOrMiterLimit == 0.f) {
                // Round joins have a circular corner with radius equal to stroke radius
                compute_outer_round_corner(m, corner, prevCorner, nextCorner, strokeRadius, strokeRadius, &out);
            } else if (joinOrMiterLimit >= SK_ScalarRoot2Over2) {
                // Mitered joins have a rectangular corner that's outset by stroke radius

                compute_outer_rect_corner(m, corner - localE0 * strokeRadius - localE1 * strokeRadius,
                                          prevCorner - localE1 * strokeRadius,
                                          nextCorner + localE0 * strokeRadius, &out);
            } else {
                // Beveled join, which has unique geometry that's somewhat similar to rounded case
            }
        } else {
            // Outer geometry is a curve with radius inflated by stroke radius. Inner geometry is
            // either a curve deflated by stroke radius, or rectangular.
            compute_outer_round_corner(m, corner, prevCorner, nextCorner, openingRadius + strokeRadius, closingRadius + strokeRadius, &out);
        }
    } else {
        // Filled geometry (ignore stroke radius and join/miter limits)
        SkV3 e0 = line_between_homogenous_points(map(m, prevCorner), v); // opposite direction to localE0
        SkV3 e1 = line_between_homogenous_points(v, map(m, nextCorner));
        // Shift both edges out by 10 pixels (for increased visibility)
        SkV3 outsetE0 = shift_line(e0, kAARadius);
        SkV3 outsetE1 = shift_line(e1, kAARadius);

        if (rectCorner) {
            // Rectangular corner



            // Interior collapses to the center of the rectangle
            out.fInnerFrame0_inset = map(m, center);
            out.fInnerFrame0_outset = out.fInnerFrame0_inset;
            out.fInnerFrame1_inset = out.fInnerFrame0_inset;
            out.fInnerFrame1_outset = out.fInnerFrame0_inset;
            out.fInnerTangent0_outset = out.fInnerFrame0_inset;
            out.fInnerTangent1_outset = out.fInnerFrame0_inset;
            out.fInnerTangent_inset = out.fInnerFrame0_inset;

            // Insets of outer frame colocated at local pixel inset unless we would pass the center
            float localAARadius = kAARadius / min;

            SkV2 frameInset = corner + localAARadius * localE0 + localAARadius * localE1;
            if (check_crossover(frameInset, center, localE0) ||
                check_crossover(frameInset, center, localE1)) {
                out.fOuterFrame0_inset = out.fInnerFrame0_inset;
                out.fOuterFrame1_inset = out.fInnerFrame0_inset;
            } else {
                out.fOuterFrame0_inset = map(m, frameInset);
                out.fOuterFrame1_inset = out.fOuterFrame0_inset;
            }
        } else {
            // Rounded corner
            SkV2 localOuterFrame0 = corner + closingRadius * localE0;
            SkV2 localOuterFrame1 = corner + openingRadius * localE1;

            SkV2 localOuterTangent0 = localOuterFrame0 - closingRadius * kTangentOffset * localE0;
            SkV2 localOuterTangent1 = localOuterFrame1 - openingRadius * kTangentOffset * localE1;

            // Outer frame and normal outsets
            float frame0_min, frame1_min, _;
            std::tie(out.fOuterFrame0, frame0_min, _) = minmax_scales(m, localOuterFrame0);
            out.fOuterFrame0_outset = line_intersection(outsetE0, perpendicular_through_point(e0, out.fOuterFrame0));
            std::tie(out.fOuterFrame1, frame1_min, _) = minmax_scales(m, localOuterFrame1);
            out.fOuterFrame1_outset = line_intersection(outsetE1, perpendicular_through_point(e1, out.fOuterFrame1));

            // Tangent frame and outsets
            out.fOuterTangent0 = map(m, localOuterTangent0);
            out.fOuterTangent1 = map(m, localOuterTangent1);
            SkV3 tangent = line_between_homogenous_points(out.fOuterTangent0, out.fOuterTangent1);
            SkV3 outsetTangent = shift_line(tangent, kAARadius);
            out.fOuterTangent0_outsetA = line_intersection(outsetE0, perpendicular_through_point(e0, out.fOuterTangent0));
            out.fOuterTangent0_outsetB = line_intersection(outsetTangent, perpendicular_through_point(tangent, out.fOuterTangent0));
            out.fOuterTangent1_outsetA = line_intersection(outsetE1, perpendicular_through_point(e1, out.fOuterTangent1));
            out.fOuterTangent1_outsetB = line_intersection(outsetTangent, perpendicular_through_point(tangent, out.fOuterTangent1));

            // Interior collapses to the center of the round rectangle
            out.fInnerFrame0_inset = map(m, center);
            out.fInnerFrame0_outset = out.fInnerFrame0_inset;
            out.fInnerFrame1_inset = out.fInnerFrame0_inset;
            out.fInnerFrame1_outset = out.fInnerFrame0_inset;
            out.fInnerTangent0_outset = out.fInnerFrame0_inset;
            out.fInnerTangent1_outset = out.fInnerFrame0_inset;
            out.fInnerTangent_inset = out.fInnerFrame0_inset;

            SkV2 tangentPoint = corner + closingRadius * (1.f - SK_ScalarRoot2Over2) * localE0
                                       + openingRadius * (1.f - SK_ScalarRoot2Over2) * localE1;
            float tangent_min;
            std::tie(out.fOuterTangent_inset, tangent_min, _) = minmax_scales(m, tangentPoint);
            SkV2 tangentInset = tangentPoint + kAARadius / tangent_min * SK_ScalarRoot2Over2 * localE0
                                             + kAARadius / tangent_min * SK_ScalarRoot2Over2 * localE1;

            SkV2 frame0Inset = localOuterFrame0 + kAARadius / frame0_min * localE1;
            SkV2 frame1Inset = localOuterFrame1 + kAARadius / frame1_min * localE0;
            SkV2 innermostInset = std::max(frame0Inset.dot(localE0), frame1Inset.dot(localE0)) * localE0 +
                                  std::max(frame0Inset.dot(localE1), frame1Inset.dot(localE1)) * localE1;
            if (check_crossover(frame0Inset, frame1Inset, localE1) ||
                check_crossover(frame1Inset, frame0Inset, localE0) ||
                check_crossover(tangentInset, frame1Inset, localE1) ||
                check_crossover(tangentInset, frame0Inset, localE0)) {
                // Collapse all insets to the innermost coordinate of the 3 insets
                frame0Inset = frame1Inset = tangentInset = innermostInset;
            }

            if (check_crossover(innermostInset, center, localE0) ||
                check_crossover(innermostInset, center, localE1)) {
                // Collapse to the center
                out.fOuterFrame0_inset = out.fInnerFrame0_inset;
                out.fOuterFrame1_inset = out.fInnerFrame0_inset;
                out.fOuterTangent_inset = out.fInnerFrame0_inset;
            } else {
                out.fOuterFrame0_inset = map(m, frame0Inset);
                out.fOuterFrame1_inset = map(m, frame1Inset);
                out.fOuterTangent_inset = map(m, tangentInset);
            }
        }
    }

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
        , fMode(PrimitiveMode::kFillRect) {}

    void onDrawContent(SkCanvas* canvas) override {
        canvas->save();
        canvas->concat(this->basisMatrix());

        SkM44 totalMatrix = canvas->getLocalToDevice();

        // Base shape
        canvas->drawRRect(this->primitiveShape(), paint(SK_ColorBLUE, this->strokeWidth()));
        canvas->drawRRect(this->primitiveShape(), paint(SK_ColorBLACK, 0.f));

        // Local circle (for visualization purposes, the smaller the radius, the more accurate the
        // perspective derivatives remain at the edge).
        auto [devCoord, min, max] = minmax_scales(totalMatrix, {0.f, 0.f});
        // canvas->drawCircle({0.f, 0.f}, 10.f / min, paint(SK_ColorDKGRAY));
        canvas->drawCircle({0.f, 0.f}, 10.f, paint(SK_ColorDKGRAY));
        canvas->restore();

        // Visualize minmax scales
        canvas->save();
        canvas->resetMatrix();
        canvas->drawCircle({devCoord.x / devCoord.z, devCoord.y / devCoord.z}, 10.f * min, paint(SK_ColorLTGRAY, 0.f));
        canvas->drawCircle({devCoord.x / devCoord.z, devCoord.y / devCoord.z}, 10.f * max, paint(SK_ColorLTGRAY, 0.f));
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
        static const SkVector kOuterRadii[4] = { { 0.25f * kBaseScale, 0.75f * kBaseScale },
                                                 {0.f, 0.f},
                                                 { 0.5f * kBaseScale, 0.5f * kBaseScale },
                                                 { 0.75f * kBaseScale, 0.25f * kBaseScale } };

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
                rrect.setRectRadii(kOuterBounds, kOuterRadii);
                return rrect;
            }
        }

        SkUNREACHABLE;
    }

    void drawVertices(SkCanvas* canvas, const SkM44& ctm) {
        auto drawLine = [canvas](const SkV3& p0, const SkV3& p1, SkColor color = SK_ColorRED) {
            if (SkScalarNearlyEqual(p0.x, p1.x) && SkScalarNearlyEqual(p0.y, p1.y)) {
                return;
            }
            canvas->drawLine({p0.x/p0.z, p0.y/p0.z}, {p1.x/p1.z, p1.y/p1.z}, paint(color, 0.f));
        };


        SkRRect rrect = this->primitiveShape();
        SkRect outer = rrect.getBounds();
        SkV2 center = {outer.centerX(), outer.centerY()};
        SkV2 localCorners[4] = {{outer.fLeft, outer.fTop}, {outer.fRight, outer.fTop},
                                {outer.fRight, outer.fBottom}, {outer.fLeft, outer.fBottom}};

        CornerVerts corners[4]; // TL, TR, BR, BL
        if (fMode == PrimitiveMode::kFillRect) {
            corners[0] = compute_corner(ctm, localCorners[0], localCorners[3], localCorners[1], center, 0.f, 0.f, -1.f, -1.f);
            corners[1] = compute_corner(ctm, localCorners[1], localCorners[0], localCorners[2], center, 0.f, 0.f, -1.f, -1.f);
            corners[2] = compute_corner(ctm, localCorners[2], localCorners[1], localCorners[3], center, 0.f, 0.f, -1.f, -1.f);
            corners[3] = compute_corner(ctm, localCorners[3], localCorners[2], localCorners[0], center, 0.f, 0.f, -1.f, -1.f);
        } else if (fMode == PrimitiveMode::kFillRRect) {
            corners[0] = compute_corner(ctm, localCorners[0], localCorners[3], localCorners[1], center,
                    rrect.radii(SkRRect::kUpperLeft_Corner).fX, rrect.radii(SkRRect::kUpperLeft_Corner).fY, -1.f, -1.f);
            corners[1] = compute_corner(ctm, localCorners[1], localCorners[0], localCorners[2], center,
                    rrect.radii(SkRRect::kUpperRight_Corner).fY, rrect.radii(SkRRect::kUpperRight_Corner).fX, -1.f, -1.f);
            corners[2] = compute_corner(ctm, localCorners[2], localCorners[1], localCorners[3], center,
                    rrect.radii(SkRRect::kLowerRight_Corner).fX, rrect.radii(SkRRect::kLowerRight_Corner).fY, -1.f, -1.f);
            corners[3] = compute_corner(ctm, localCorners[3], localCorners[2], localCorners[0], center,
                    rrect.radii(SkRRect::kLowerLeft_Corner).fY, rrect.radii(SkRRect::kLowerLeft_Corner).fX, -1.f, -1.f);
        }

        // Corner blocks
        for (auto c : corners) {
            // Outer structure
            drawLine(c.fOuterFrame0, c.fOuterFrame0_outset);
            drawLine(c.fOuterFrame0, c.fOuterFrame0_inset);

            drawLine(c.fOuterFrame0, c.fOuterTangent0);
            drawLine(c.fOuterFrame0_outset, c.fOuterTangent0_outsetA);
            drawLine(c.fOuterFrame0_inset, c.fOuterTangent_inset);

            drawLine(c.fOuterTangent0_outsetA, c.fOuterTangent0);
            drawLine(c.fOuterTangent0_outsetA, c.fOuterTangent0_outsetB);
            drawLine(c.fOuterTangent0_outsetB, c.fOuterTangent0);

            drawLine(c.fOuterTangent0_outsetB, c.fOuterTangent1_outsetB);
            drawLine(c.fOuterTangent0, c.fOuterTangent1);

            drawLine(c.fOuterTangent1_outsetA, c.fOuterTangent1);
            drawLine(c.fOuterTangent1_outsetA, c.fOuterTangent1_outsetB);
            drawLine(c.fOuterTangent1_outsetB, c.fOuterTangent1);

            drawLine(c.fOuterFrame1, c.fOuterTangent1);
            drawLine(c.fOuterFrame1_outset, c.fOuterTangent1_outsetA);
            drawLine(c.fOuterFrame1_inset, c.fOuterTangent_inset);

            drawLine(c.fOuterFrame1, c.fOuterFrame1_outset);
            drawLine(c.fOuterFrame1, c.fOuterFrame1_inset);

            // Inner structure
            drawLine(c.fInnerFrame0_outset, c.fInnerFrame0_inset);
            drawLine(c.fInnerFrame0_outset, c.fInnerTangent0_outset);
            drawLine(c.fInnerFrame0_inset, c.fInnerTangent_inset);

            drawLine(c.fInnerFrame1_inset, c.fInnerFrame1_inset);
            drawLine(c.fInnerFrame1_outset, c.fInnerTangent1_outset);
            drawLine(c.fInnerFrame1_inset, c.fInnerTangent_inset);

            // Inner to outer connections
            drawLine(c.fOuterFrame0_inset, c.fInnerFrame0_outset);
            drawLine(c.fOuterFrame1_inset, c.fInnerFrame1_outset);
        }

        // Corner connections
        for (int i = 0; i < 4; ++i) {
            int n = i == 3 ? 0 : i + 1;

            drawLine(corners[i].fOuterFrame1, corners[n].fOuterFrame0);
            drawLine(corners[i].fOuterFrame1_outset, corners[n].fOuterFrame0_outset);
            drawLine(corners[i].fOuterFrame1_inset, corners[n].fOuterFrame0_inset);

            drawLine(corners[i].fInnerFrame1_inset, corners[n].fInnerFrame0_inset);
            drawLine(corners[i].fInnerFrame1_outset, corners[n].fInnerFrame0_outset);
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
