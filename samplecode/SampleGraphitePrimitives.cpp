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
// Singular values of [df/du dg/du] define perspective correct minimum and maximum scale factors
//                    [df/dv dg/dv]
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

    // no-persp, this is the singular values of [m00,m10][m01,m11], which is transpose of upper 2x2,
    // and singular values are invariant to transpose.
    auto [min, max] = minmax_scales(dfdu, dgdu, dfdv, dgdv);
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

// Vertex structure for a filled rectangle:
//   4 vertices at the original rectangle corners
//   8 vertices outset by normals on each edge
//   8 (dual-located) vertices at bisector of corner normals
//   4 co-located vertices at rectangle center

// Returns TL, TR, BR, BL
static std::array<SkV3, 4> map_rectangle(const SkM44& matrix, const SkRect& rect) {


    return {map(matrix, {rect.fLeft,  rect.fTop}),
            map(matrix, {rect.fRight, rect.fTop}),
            map(matrix, {rect.fRight, rect.fBottom}),
            map(matrix, {rect.fLeft,  rect.fBottom})};
}

// Returns (TL->TR), (TR->BR), (BR->BL), (BL->TL)
static std::array<SkV3, 4> compute_edges(const std::array<SkV3, 4>& corners) {
    return {line_between_homogenous_points(corners[0], corners[1]),
            line_between_homogenous_points(corners[1], corners[2]),
            line_between_homogenous_points(corners[2], corners[3]),
            line_between_homogenous_points(corners[3], corners[0])};
}

static SkV3 compute_center(const SkM44& matrix, const SkRect& rect) {
    // Calculate local interior (since th at gives us the local coords too), but should be roughly
    // equal (sans float error) to just averaging the device-mapped corners as well.
    float x = rect.centerX();
    float y = rect.centerY();
    SkV4 d = matrix.map(x, y, 0.f, 1.f);
    return {d.x, d.y, d.w};
}

// Returns [outset vertex for e0, bisecting vertex, outset vertex for e1] for the original corner
// with connecting edges e0, and e1, assuming CW winding.
static std::array<SkV3, 3> compute_rect_corner(const SkV3& corner, const SkV3& e0, const SkV3& e1) {
    // Compute the normals for each edge through the specific corner.
    SkV3 n0 = perpendicular_through_point(e0, corner);
    SkV3 n1 = perpendicular_through_point(e1, corner);

    // Shift both edges out by 10 pixels (for increased visibility)
    SkV3 outsetE0 = shift_line(e0, 10.f);
    SkV3 outsetE1 = shift_line(e1, 10.f);

    // Outset vertices are the intersection between the normal line and the outset edge
    SkV3 outsetV0 = line_intersection(outsetE0, n0);
    SkV3 outsetV1 = line_intersection(outsetE1, n1);

    return {outsetV0, bisection(e0, e1, corner, 10.f), outsetV1};
}

// Given a local-space corner of the rrect bounding box in 'outerCorner', and its signed corner
// radii, the inner corner is calculated as (outerCorner + (radiusX, radiusY)), so a positive value
// moves the corner to the right or down, and a negative radius moves it to the left or up.
// Top-left corner should have signs (+,+), top-right = (-,+), bot-right = (-,-), bot-left = (+,-).
//
//
// [ (outerCorner.x, innerCorner.y), v0 outset for AA, v0 octagonal vertex, octagonal offset for v0,
//   (innerCorner.x, outerCorner.y), v3 outset for AA, v1 octagonal vertex, octagonal offset for v3,
//   innerCorner], all mapped by the matrix to device space.
static std::array<SkV3, 12> compute_rrect_corner(const SkM44& ctm,
                                                 const SkV2& outerCorner,
                                                 float signedRadiusX, float signedRadiusY,
                                                 const SkV3& e0, const SkV3& e1) {
    static constexpr float kOctoWeight = SK_FloatSqrt2 - 1.f;

    // FIXME if radiusX and Y are 0, the math here works out okay, except we don't end up with the
    // bisector that we manually add for the regular rect corner, and that seems useful when
    // we don't have the rest of the curve bulking things up for us.
    //
    // - Do we always want a central bisector? E.g. a point outset along the normal of octoEdge?
    //   I think that could turn into the bisector when radii are 0,0 (maybe not automatically, but
    //   boy that would be sick).
    //  - If I did have this point, would I need the others?


    // These 5 coordinates are in local space and must be mapped to device space
    SkV2 innerCorner = outerCorner + SkV2{signedRadiusX, signedRadiusY};
    SkV2 edgeRadX = {innerCorner.x, outerCorner.y};
    SkV2 edgeRadY = {outerCorner.x, innerCorner.y};

    SkV2 octogonRadX = edgeRadX - SkV2{kOctoWeight * signedRadiusX, 0.f};
    SkV2 octogonRadY = edgeRadY - SkV2{0.f, kOctoWeight * signedRadiusY};

    SkV3 edgeRadX3, edgeRadY3;
    float xMinScale, yMinScale, _;

    std::tie(edgeRadX3, xMinScale, _) = minmax_scales(ctm, edgeRadX);
    std::tie(edgeRadY3, yMinScale, _) = minmax_scales(ctm, edgeRadY);
    // Inset edgeRadX3 along Y and edgeRadY3 along X axis locally, pinning to innerCorner.
    auto sign = [](float v) { return v > 0.f ? 1.f : (v < 0.f ? -1.f : 0.f); };
    SkV2 inEdgeRadX = edgeRadX + SkV2{0.f, sign(signedRadiusY) * 10.f / xMinScale};
    SkV2 inEdgeRadY = edgeRadY + SkV2{sign(signedRadiusX) * 10.f / yMinScale, 0.f};
    if (signedRadiusY > 0.f) {
        inEdgeRadX.y = std::min(edgeRadY.y, inEdgeRadX.y);
    } else {
        inEdgeRadX.y = std::max(edgeRadY.y, inEdgeRadX.y);
    }
    if (signedRadiusX > 0.f) {
        inEdgeRadY.x = std::min(edgeRadX.x, inEdgeRadY.x);
    } else {
        inEdgeRadY.x = std::max(edgeRadX.x, inEdgeRadY.x);
    }

    const bool transposeEdgeCoords = (signedRadiusX * signedRadiusY) < 0.f;
    SkV3 e0Coord, e1Coord, octo0Coord, octo1Coord;
    SkV3 inE0Coord, inE1Coord;
    if (transposeEdgeCoords) {
        e0Coord = edgeRadX3;
        e1Coord = edgeRadY3;

        octo0Coord = map(ctm, octogonRadX);
        octo1Coord = map(ctm, octogonRadY);

        inE0Coord = map(ctm, inEdgeRadX);
        inE1Coord = map(ctm, inEdgeRadY);
    } else {
        e0Coord = edgeRadY3;
        e1Coord = edgeRadX3;

        octo0Coord = map(ctm, octogonRadY);
        octo1Coord = map(ctm, octogonRadX);

        inE0Coord = map(ctm, inEdgeRadY);
        inE1Coord = map(ctm, inEdgeRadX);
    }

    auto outset = [](const SkV3& edge, const SkV3& point) {
        SkV3 n = perpendicular_through_point(edge, point);
        SkV3 o = shift_line(edge, 10.f);
        return line_intersection(o, n);
    };

    SkV3 octoEdge = line_between_homogenous_points(octo0Coord, octo1Coord);
    auto octoOutset0 = compute_rect_corner(octo0Coord, e0, octoEdge);
    auto octoOutset1 = compute_rect_corner(octo1Coord, octoEdge, e1);

    // include the bisector at each octogon corner?
    return { e0Coord, outset(e0, e0Coord), octo0Coord, octoOutset0[0], octoOutset0[2],
             e1Coord, outset(e1, e1Coord), octo1Coord, octoOutset1[0], octoOutset1[2],
             inE0Coord, inE1Coord};
            //  map(ctm, innerCorner)};
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
        // For strokes, we keep the outer bounds consistent with the filled style, so we inset by
        // half the stroke width. For simplicity, clamp the stroke width to kBaseScale so that we
        // can collapse the interior, but not invert.
        return SkTPin(fStrokeWidth, 0.f, kBaseScale);
    }

    SkRRect primitiveShape() const {
        static const SkRect kOuterBounds = SkRect::MakeLTRB(-kBaseScale, -kBaseScale,
                                                            kBaseScale, kBaseScale);
        static const SkVector kOuterRadii[4] = { { 0.25f * kBaseScale, 0.75f * kBaseScale },
                                                 {0.f, 0.f}, //{ 0.1f * kBaseScale, 0.1f * kBaseScale }, // FIXME (0,0)
                                                 { 0.5f * kBaseScale, 0.5f * kBaseScale },
                                                 { 0.75f * kBaseScale, 0.25f * kBaseScale } };

        float strokeRadius = 0.5f * this->strokeWidth();
        switch(fMode) {
            case PrimitiveMode::kFillRect:
                return SkRRect::MakeRect(kOuterBounds);
            case PrimitiveMode::kFillRRect: {
                SkRRect rrect;
                rrect.setRectRadii(kOuterBounds, kOuterRadii);
                return rrect; }
            case PrimitiveMode::kStrokeRect:
                return SkRRect::MakeRect(kOuterBounds.makeInset(strokeRadius, strokeRadius));
            case PrimitiveMode::kStrokeRRect: {
                SkVector insetRadii[4];
                for (int i = 0; i < 4; ++i) {
                    insetRadii[i].fX = std::max(0.f, kOuterRadii[i].fX - strokeRadius);
                    insetRadii[i].fY = std::max(0.f, kOuterRadii[i].fY - strokeRadius);
                }
                SkRRect rrect;
                rrect.setRectRadii(kOuterBounds.makeInset(strokeRadius, strokeRadius), insetRadii);
                return rrect;
            }
        }

        SkUNREACHABLE;
    }

    void drawVertices(SkCanvas* canvas, const SkM44& ctm) {
        auto drawLine = [canvas](const SkV3& p0, const SkV3& p1, SkColor color = SK_ColorRED) {
            canvas->drawLine({p0.x/p0.z, p0.y/p0.z}, {p1.x/p1.z, p1.y/p1.z}, paint(color, 0.f));
        };


        SkRRect rrect = this->primitiveShape();
        SkRect outer = rrect.getBounds();

        float strokeRadius = 0.5f * this->strokeWidth();
        if (strokeRadius > 0.f) {
            // FIXME handle hairlines?
            outer.outset(strokeRadius, strokeRadius);
        }

        // TODO visualize the shapes specifically
        auto corners = map_rectangle(ctm, outer);
        auto edges = compute_edges(corners);
        auto center =  compute_center(ctm, outer);

        if (fMode == PrimitiveMode::kFillRect) {
            auto tl = compute_rect_corner(corners[0], edges[3], edges[0]);
            auto tr = compute_rect_corner(corners[1], edges[0], edges[1]);
            auto br = compute_rect_corner(corners[2], edges[1], edges[2]);
            auto bl = compute_rect_corner(corners[3], edges[2], edges[3]);

            // Top (left to right)
            drawLine(corners[0], center);
            drawLine(corners[0], tl[1]);      // TL bisector
            drawLine(tl[1], tl[2]);           // TL top bisector to outer
            drawLine(corners[0], tl[2]);      // TL top normal
            drawLine(corners[0], corners[1]); // TL -> TR inner
            drawLine(tl[2], tr[0]);           // TL -> TR outer
            drawLine(corners[1], tr[0]);      // TR top normal
            drawLine(tr[0], tr[1]);           // TR top outer to bisector

            // Right (top to bottom)
            drawLine(corners[1], center);
            drawLine(corners[1], tr[1]);      // TR bisector
            drawLine(tr[1], tr[2]);           // TR right bisector to outer
            drawLine(corners[1], tr[2]);      // TR right normal
            drawLine(corners[1], corners[2]); // TR -> BR inner
            drawLine(tr[2], br[0]);           // TR -> BR outer
            drawLine(corners[2], br[0]);      // BR right normal
            drawLine(br[0], br[1]);           // BR right outer to bisector

            // Bottom (right to left)
            drawLine(corners[2], center);
            drawLine(corners[2], br[1]);      // BR bisector
            drawLine(br[1], br[2]);           // BR bot bisector to outer
            drawLine(corners[2], br[2]);      // BR bot normal
            drawLine(corners[2], corners[3]); // BR -> BL inner
            drawLine(br[2], bl[0]);           // BR -> BL outer
            drawLine(corners[3], bl[0]);      // BL bot normal
            drawLine(bl[0], bl[1]);           // BL bot outer to bisector

            // Left (bottom to top)
            drawLine(corners[3], center);
            drawLine(corners[3], bl[1]);      // BL bisector
            drawLine(bl[1], bl[2]);           // BL left bisector to outer
            drawLine(corners[3], bl[2]);      // BL left normal
            drawLine(corners[3], corners[0]); // BL -> TL inner
            drawLine(bl[2], tl[0]);           // BL -> TL outer
            drawLine(corners[0], tl[0]);      // TL left normal
            drawLine(tl[0], tl[1]);           // TL left outer to bisector
        } else if (fMode == PrimitiveMode::kFillRRect) {
            auto tl = compute_rrect_corner(ctm,
                    {outer.fLeft, outer.fTop},
                    rrect.radii(SkRRect::kUpperLeft_Corner).fX,
                    rrect.radii(SkRRect::kUpperLeft_Corner).fY,
                    edges[3], edges[0]);
            auto tr = compute_rrect_corner(ctm,
                    {outer.fRight, outer.fTop},
                    -rrect.radii(SkRRect::kUpperRight_Corner).fX,
                    rrect.radii(SkRRect::kUpperRight_Corner).fY,
                    edges[0], edges[1]);
            auto br = compute_rrect_corner(ctm,
                    {outer.fRight, outer.fBottom},
                    -rrect.radii(SkRRect::kLowerRight_Corner).fX,
                    -rrect.radii(SkRRect::kLowerRight_Corner).fY,
                    edges[1], edges[2]);
            auto bl = compute_rrect_corner(ctm,
                    {outer.fLeft, outer.fBottom},
                    rrect.radii(SkRRect::kLowerLeft_Corner).fX,
                    -rrect.radii(SkRRect::kLowerLeft_Corner).fY,
                    edges[2], edges[3]);

            // { 0: e0Coord,
            //   1: outset(e0, e0Coord),
            //   2: octo0Coord,
            //   3: octoOutset0[0],
            //   4: octoOutset0[2],
            //   5: e1Coord,
            //   6: outset(e1, e1Coord),
            //   7: octo1Coord,
            //   8: octoOutset1[0],
            //   9: octoOutset1[2],
            //  10: inE0
            //  11: inE1
            for (auto corner : {tl, tr, br, bl}) {
                drawLine(corner[0], corner[1]);
                drawLine(corner[0], corner[2]);
                drawLine(corner[1], corner[3]);
                drawLine(corner[2], corner[3]);
                drawLine(corner[2], corner[4]);
                drawLine(corner[3], corner[4]);

                drawLine(corner[5], corner[6]);
                drawLine(corner[5], corner[7]);
                drawLine(corner[6], corner[9]);
                drawLine(corner[7], corner[8]);
                drawLine(corner[7], corner[9]);
                drawLine(corner[8], corner[9]);

                drawLine(corner[4], corner[8]);
                drawLine(corner[2], corner[7]);
                drawLine(corner[0], corner[10]);
                drawLine(corner[5], corner[11]);
                drawLine(corner[10], corner[11]);
            }

            drawLine(tl[5], tr[0]);
            drawLine(tl[6], tr[1]);
            drawLine(tl[11], tr[10]);
            drawLine(tl[10], center);
            drawLine(tl[11], center);

            drawLine(tr[5], br[0]);
            drawLine(tr[6], br[1]);
            drawLine(tr[11], br[10]);
            drawLine(tr[10], center);
            drawLine(tr[11], center);

            drawLine(br[5], bl[0]);
            drawLine(br[6], bl[1]);
            drawLine(br[11], bl[10]);
            drawLine(br[10], center);
            drawLine(br[11], center);

            drawLine(bl[5], tl[0]);
            drawLine(bl[6], tl[1]);
            drawLine(bl[11], tl[10]);
            drawLine(bl[10], center);
            drawLine(bl[11], center);
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
                fStrokeWidth = std::max(0.f, fStrokeWidth - 0.2f);
                return true;
            case '=':
                fStrokeWidth = std::min(kBaseScale, fStrokeWidth + 0.2f);
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
