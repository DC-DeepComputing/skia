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

static constexpr float kMiterScale = 1.f;
static constexpr float kBevelScale = 0.0f;
static constexpr float kRoundScale = SK_FloatSqrt2 - 1.f;

struct LocalCornerVert {
    SkV2 fPosition;     // In unit square that each corner is normalized to
    SkV2 fNormal;       // Direction that AA outset is applied in
    float fStrokeScale; // Signed scale factor applied to external stroke radius, should be (-1,1)
    float fMirrorScale; // Scale fPosition.yx, along with external join-scale, should be (0,1).

    bool isOuterCurve() const { return fStrokeScale > 0.f; }
    bool deviceSpaceNormal() const {
        return this->isOuterCurve() && (fNormal.x > 0.f || fNormal.y > 0.f);
    }
    bool bisectMiterNormal() const {
        return this->isOuterCurve() && (fNormal.x > 0.f && fNormal.y > 0.f);
    }

    // If 'center' is non-null, then interior vertices are set to the center instead of their
    // regular calculation, since it's a fill. 'cornerMapping' is a row-major 2x2 matrix
    //    [[x y], [z w]] to flip and swizzle the normalized positions into the local coord space.
    SkV3 transform(const SkM44& m, const SkV4& cornerMapping, const SkV2& cornerPt,
                   const SkV2& cornerRadii, const SkV2* center,
                   float strokeRadius, float joinScale, float localAARadius) const {
        if (this->deviceSpaceNormal()) {
            // Don't locally move along normals when it's a device-space outset.
            localAARadius = 0.f;
        }

        // scale+translate define the mapping from normalized unit corner to actual corner,
        // before any AA offsets are added.
        SkV2 scale = cornerRadii + SkV2{fStrokeScale*strokeRadius, fStrokeScale*strokeRadius};
        SkV2 translate;
        if (scale.x < 0.f || scale.y < 0.f) {
            // We have a miter corner (either stroked rect corner with any join style, or a stroked
            // round corner where the stroke radius exceeds the corner radius). To avoid inverting
            // the interior vertices, we adjust the normalized space to be a localAARadius-sized
            // mitered square that's translated to align with the original edge.
            translate = scale - cornerRadii - SkV2{localAARadius, localAARadius};
            scale = {localAARadius, localAARadius};
            joinScale = kMiterScale;
        } else {
            // The corner doesn't collapse, or it's the outer curve of the corner, so leave scale
            // as-is. We translate such that (0,0) of the normalized space maps to -cornerRadii
            // (i.e. the interior vertex of the base curve before any stroke inset/outset).
            translate = -cornerRadii;
        }

        SkV2 localPos;
        if (SkToBool(center) && !this->isOuterCurve()) {
            localPos = *center;
        } else {
            SkV2 normalizedPos =
                    scale*(fPosition + joinScale*fMirrorScale*SkV2{fPosition.y, fPosition.x}) +
                    localAARadius * fNormal;

            // Handle insets crossing over each other; this coerces them into a roughly mitered
            // diagonal even if the outsets preserve the base style's shape.
            float min = std::min(normalizedPos.x, normalizedPos.y);
            if (min < 0) {
                // FIXME this might not be exactly right...
                SkV2 max = {std::min(scale.x - localAARadius, 0.f),
                            std::min(scale.y - localAARadius, 0.f)};

                normalizedPos = {std::max(max.x, min),
                                 std::max(max.y, min)};
            }

            normalizedPos += translate;
            // FIXME handle outer/inner overlap here
            // FIXME handle outer insets crossing over center
            localPos.x = cornerMapping.x*normalizedPos.x + cornerMapping.y*normalizedPos.y;
            localPos.y = cornerMapping.z*normalizedPos.x + cornerMapping.w*normalizedPos.y;
            localPos += cornerPt;
        }

        SkV4 devPos = m.map(localPos.x, localPos.y, 0.f, 1.f);
        if (this->deviceSpaceNormal()) {
            // Assuming quadrilaterals have their non-orthogonal local basis set in
            // cornerMapping, this will give us the right bisector and normals.
            SkV2 devNorm;
            if (this->bisectMiterNormal() && joinScale == kMiterScale) {
                SkV4 e0 = m.map(cornerMapping.x, cornerMapping.z, 0.f, 0.f);
                SkV4 e1 = m.map(cornerMapping.y, cornerMapping.w, 0.f, 0.f);

                SkV2 e0Norm = SkV2{e0.x, e0.y}.normalize();
                SkV2 e1Norm = SkV2{e1.x, e1.y}.normalize();
                if (e0Norm.dot(e1Norm) < -0.8) {
                    // Nearly opposite directions, so the sum could have cancellation, so instead
                    // bisect orthogonal vectors and flip to keep a consistent direction.
                    e0Norm = SkV2{-e0Norm.y, e0Norm.x};
                    e1Norm = SkV2{e1Norm.y, -e1Norm.x};
                    // FIXME and flip based on sign of e0Norm X e1Norm.z since unlike when mapping
                    // by normal matrix, this math does appear to be sensitive to winding inversion.
                }
                devNorm = (e0Norm + e1Norm).normalize();
            } else {
                SkM44 normalMatrix = m * SkM44(cornerMapping.x, cornerMapping.y, 0.f, cornerPt.x,
                                               cornerMapping.z, cornerMapping.w, 0.f, cornerPt.y,
                                               0.f, 0.f, 1.f, 0.f,
                                               0.f, 0.f, 0.f, 1.f);

                // Account for non-uniform stretching of the corner space, which we skip for
                // miter since it's corner space maps to an oriented point, so scale is invalid
                if (joinScale != kMiterScale) {
                    normalMatrix.preScale(scale.x, scale.y);
                }
                bool valid = normalMatrix.invert(&normalMatrix);
                SkASSERT(valid);
                normalMatrix = normalMatrix.transpose();
                SkV4 devNorm4 = normalMatrix.map(fNormal.x, fNormal.y, 0.f, 0.f);
                devNorm = SkV2{devNorm4.x, devNorm4.y}.normalize();
            }

            devPos.x += devPos.w * kAARadius * devNorm.x;
            devPos.y += devPos.w * kAARadius * devNorm.y;
        }

        return SkV3{devPos.x, devPos.y, devPos.w};
    }
};

static constexpr LocalCornerVert kCornerTemplate[20] = {
    // Outer anchors (no normal movement)
    { {0.0f, 1.0f}, { 0.0f,  0.0f},  1.0f, 0.0f },
    { {0.0f, 1.0f}, { 0.0f,  0.0f},  1.0f, 1.0f },
    { {1.0f, 0.0f}, { 0.0f,  0.0f},  1.0f, 1.0f },
    { {1.0f, 0.0f}, { 0.0f,  0.0f},  1.0f, 0.0f },

    // Device-space AA outsets
    { {0.0f, 1.0f}, { 0.0f,  1.0f},  1.0f, 0.0f },
    { {0.0f, 1.0f}, { 0.0f,  1.0f},  1.0f, 1.0f },
    { {0.0f, 1.0f}, { 1.0f,  1.0f},  1.0f, 1.0f }, // We normalize after transforming so (1,1) is ok
    { {1.0f, 0.0f}, { 1.0f,  1.0f},  1.0f, 1.0f }, // and helps parallel inner curve's attr data.
    { {1.0f, 0.0f}, { 1.0f,  0.0f},  1.0f, 1.0f },
    { {1.0f, 0.0f}, { 1.0f,  0.0f},  1.0f, 0.0f },

    // Outer curve local AA insets
    { {0.0f, 1.0f}, { 0.0f, -1.0f},  1.0f, 0.0f },
    { {0.5f, 0.5f}, {-1.0f, -1.0f},  1.0f, 1.0f }, // Use (-1,-1) insets so miter edges are parallel
    { {1.0f, 0.0f}, {-1.0f,  0.0f},  1.0f, 0.0f }, // ""

    // Inner curve local AA outsets
    { {0.0f, 1.0f}, { 0.0f,  1.0f}, -1.0f, 0.0f },
    { {0.0f, 1.0f}, { 1.0f,  1.0f}, -1.0f, 1.0f }, // Places vertex at corner when mitered
    { {1.0f, 0.0f}, { 1.0f,  1.0f}, -1.0f, 1.0f },
    { {1.0f, 0.0f}, { 1.0f,  0.0f}, -1.0f, 0.0f },

    // Inner curve local AA insets
    { {0.0f, 1.0f}, { 0.0f, -1.0f}, -1.0f, 0.0f },
    { {0.5f, 0.5f}, {-1.0f, -1.0f}, -1.0f, 1.0f },
    { {1.0f, 0.0f}, {-1.0f,  0.0f}, -1.0f, 0.0f }
};

static void compute_corner(SkV3 devPts[20], const SkM44& m, const SkV4& cornerMapping,
                           const SkV2& cornerPt, const SkV2& cornerRadii, const SkV2* center,
                           float strokeRadius, SkPaint::Join join) {
    float joinScale;
    float localAARadius = local_aa_radius(m, cornerPt);

    // FIXME: checking against localAARadius can snap to rect corner unexpectedly under high skew
    // because localAARadius gets so big.
    if (cornerRadii.x <= 0.f || cornerRadii.y <= 0.f) {
        // Effectively a rectangular corner
        joinScale = kMiterScale; // default for rect corners
        if (strokeRadius > 0.f) {
            // Non-hairline strokes need to adjust the join scale factor to match style.
            if (join == SkPaint::kBevel_Join) {
                joinScale = kBevelScale;
            } else if (join == SkPaint::kRound_Join) {
                joinScale = kRoundScale;
            }
        }
    } else {
        // Rounded filled corner vertices are always positioned for a round join since the
        // underlying geometry has no real tangent discontinuity.
        joinScale = kRoundScale;
    }

    for (int i = 0; i < 20; ++i) {
        devPts[i] = kCornerTemplate[i].transform(m, cornerMapping, cornerPt, cornerRadii,
                                                 center, strokeRadius, joinScale, localAARadius);
    }
}

static const uint16_t kBR = 0;
static const uint16_t kTR = 20;
static const uint16_t kTL = 40;
static const uint16_t kBL = 60;
static void compute_vertices(SkV3 devPts[80], const SkM44& m, const SkRRect& rrect,
                             float strokeRadius, SkPaint::Join join) {
    SkV2 center = {rrect.getBounds().centerX(), rrect.getBounds().centerY()};
    const SkV2* snapToCenter;
    if (strokeRadius < 0.f) {
        // A fill, so inner vertices need to snap to the center and then adjust the stroke radius
        // to 0 for later math to work out nicely.
        snapToCenter = &center;
        strokeRadius = 0.f;
    } else {
        // A stroke or hairline, so inner vertices won't snap to the center.
        // TODO: Additionally, inner vertex insets could still cross over each other, which isn't
        // the same as snapping to the center since we need a non-empty coverage ramp, but may mean
        // the center coords must be available to check against.
        if (2.f * strokeRadius >= rrect.width() || 2.f * strokeRadius >= rrect.height()) {
            // Except when the stroke is so large that the interior has filled in
            snapToCenter = &center;
        } else {
            snapToCenter = nullptr;
        }
    }

    // The normalized corner template is defined relative to the quarter circle with positive X
    // and positive Y, with a counter clockwise winding (if +Y points down). This corresponds to
    // the bottom-right corner.
    static constexpr SkV4 kBRBasis = { 1.f,  0.f,  0.f,  1.f};
    static constexpr SkV4 kTRBasis = { 0.f,  1.f, -1.f,  0.f};
    static constexpr SkV4 kTLBasis = {-1.f,  0.f,  0.f, -1.f};
    static constexpr SkV4 kBLBasis = { 0.f, -1.f,  1.f,  0.f};

    compute_corner(devPts + kBR, m, kBRBasis,
                   {rrect.getBounds().fRight,
                    rrect.getBounds().fBottom},
                   {rrect.radii(SkRRect::kLowerRight_Corner).fX,
                    rrect.radii(SkRRect::kLowerRight_Corner).fY},
                   snapToCenter, strokeRadius, join);
    compute_corner(devPts + kTR, m, kTRBasis,
                   {rrect.getBounds().fRight,
                    rrect.getBounds().fTop},
                   {rrect.radii(SkRRect::kUpperRight_Corner).fY,
                    rrect.radii(SkRRect::kUpperRight_Corner).fX},
                   snapToCenter, strokeRadius, join);
    compute_corner(devPts + kTL, m, kTLBasis,
                   {rrect.getBounds().fLeft,
                    rrect.getBounds().fTop},
                   {rrect.radii(SkRRect::kUpperLeft_Corner).fX,
                    rrect.radii(SkRRect::kUpperLeft_Corner).fY},
                   snapToCenter, strokeRadius, join);
    compute_corner(devPts + kBL, m, kBLBasis,
                   {rrect.getBounds().fLeft,
                    rrect.getBounds().fBottom},
                   {rrect.radii(SkRRect::kLowerLeft_Corner).fY,
                    rrect.radii(SkRRect::kLowerLeft_Corner).fX},
                   snapToCenter, strokeRadius, join);
}

// All indices
static const uint16_t kIndices[] = {
    // Exterior AA ramp outset
    kBR+0,kBR+4,kBR+1,kBR+5,kBR+1,kBR+6,kBR+2,kBR+7,kBR+2,kBR+8,kBR+3,kBR+9,
    kTR+0,kTR+4,kTR+1,kTR+5,kTR+1,kTR+6,kTR+2,kTR+7,kTR+2,kTR+8,kTR+3,kTR+9,
    kTL+0,kTL+4,kTL+1,kTL+5,kTL+1,kTL+6,kTL+2,kTL+7,kTL+2,kTL+8,kTL+3,kTL+9,
    kBL+0,kBL+4,kBL+1,kBL+5,kBL+1,kBL+6,kBL+2,kBL+7,kBL+2,kBL+8,kBL+3,kBL+9,
    kBR+0,kBR+4,kBR+0, // close loop, add extra vertex to bridge to next strip
    // Exterior AA ramp inset
    kBR+0,kBR+10,kBR+1,kBR+11,kBR+2,kBR+12,kBR+3,kBR+12,
    kTR+0,kTR+10,kTR+1,kTR+11,kTR+2,kTR+12,kTR+3,kTR+12,
    kTL+0,kTL+10,kTL+1,kTL+11,kTL+2,kTL+12,kTL+3,kTL+12,
    kBL+0,kBL+10,kBL+1,kBL+11,kBL+2,kBL+12,kBL+3,kBL+12,
    kBR+0,kBR+10,kBR+10, // close loop, add extra vertex to bridge to next strip
    // Strip between AA ramps
    kBR+13,kBR+10,kBR+14,kBR+11,kBR+15,kBR+12,kBR+16,kBR+12,
    kTR+13,kTR+10,kTR+14,kTR+11,kTR+15,kTR+12,kTR+16,kTR+12,
    kTL+13,kTL+10,kTL+14,kTL+11,kTL+15,kTL+12,kTL+16,kTL+12,
    kBL+13,kBL+10,kBL+14,kBL+11,kBL+15,kBL+12,kBL+16,kBL+12,
    kBR+13,kBR+10,kBR+13, // close loop, add extra vertex to next strip
    // Interior AA ramp
    kBR+13,kBR+17,kBR+14,kBR+18,kBR+15,kBR+19,kBR+16,kBR+19,
    kTR+13,kTR+17,kTR+14,kTR+18,kTR+15,kTR+19,kTR+16,kTR+19,
    kTL+13,kTL+17,kTL+14,kTL+18,kTL+15,kTL+19,kTL+16,kTL+19,
    kBL+13,kBL+17,kBL+14,kBL+18,kBL+15,kBL+19,kBL+16,kBL+19,
    kBR+13,kBR+17, // close the loop, mesh is complete
};

// Separated to draw with different colors (vs. duplicating vertices to change colors).
static const uint16_t kOuterCornerIndices[] = {
    kBR+0,  kBR+0,kBR+4,kBR+1,kBR+5,kBR+1,kBR+6,kBR+2,kBR+7,kBR+2,kBR+8,kBR+3,kBR+9,  kBR+9,
    kTR+0,  kTR+0,kTR+4,kTR+1,kTR+5,kTR+1,kTR+6,kTR+2,kTR+7,kTR+2,kTR+8,kTR+3,kTR+9,  kTR+9,
    kTL+0,  kTL+0,kTL+4,kTL+1,kTL+5,kTL+1,kTL+6,kTL+2,kTL+7,kTL+2,kTL+8,kTL+3,kTL+9,  kTL+9,
    kBL+0,  kBL+0,kBL+4,kBL+1,kBL+5,kBL+1,kBL+6,kBL+2,kBL+7,kBL+2,kBL+8,kBL+3,kBL+9,  kBL+9,

    kBR+0,  kBR+0,kBR+10,kBR+1,kBR+11,kBR+2,kBR+12,kBR+3,kBR+12,  kBR+12,
    kTR+0,  kTR+0,kTR+10,kTR+1,kTR+11,kTR+2,kTR+12,kTR+3,kTR+12,  kTR+12,
    kTL+0,  kTL+0,kTL+10,kTL+1,kTL+11,kTL+2,kTL+12,kTL+3,kTL+12,  kTL+12,
    kBL+0,  kBL+0,kBL+10,kBL+1,kBL+11,kBL+2,kBL+12,kBL+3,kBL+12,  kBL+12,
};

static const uint16_t kInnerCornerIndices[] = {
    kBR+13,  kBR+13,kBR+17,kBR+14,kBR+18,kBR+15,kBR+19,kBR+16,kBR+19,  kBR+19,
    kTR+13,  kTR+13,kTR+17,kTR+14,kTR+18,kTR+15,kTR+19,kTR+16,kTR+19,  kTR+19,
    kTL+13,  kTL+13,kTL+17,kTL+14,kTL+18,kTL+15,kTL+19,kTL+16,kTL+19,  kTL+19,
    kBL+13,  kBL+13,kBL+17,kBL+14,kBL+18,kBL+15,kBL+19,kBL+16,kBL+19,  kBL+19,
};

static const uint16_t kInteriorIndices[] = {
    kBR+13,kBR+10,kBR+14,kBR+11,kBR+15,kBR+12,kBR+16,kBR+12,
    kTR+13,kTR+10,kTR+14,kTR+11,kTR+15,kTR+12,kTR+16,kTR+12,
    kTL+13,kTL+10,kTL+14,kTL+11,kTL+15,kTL+12,kTL+16,kTL+12,
    kBL+13,kBL+10,kBL+14,kBL+11,kBL+15,kBL+12,kBL+16,kBL+12,
    kBR+13,kBR+10
};
// Implicit in the original mesh from the tri-strip connections between corners
static const uint16_t kEdgeIndices[] = {
    kBR+3,   kBR+3,kBR+9,kTR+0,kTR+4,      kTR+4,
    kBR+3,   kBR+3,kBR+12,kTR+0,kTR+10,    kTR+10,
    kBR+16,  kBR+16,kBR+19,kTR+13,kTR+17,  kTR+17,

    kTR+3,   kTR+3,kTR+9,kTL+0,kTL+4,      kTL+4,
    kTR+3,   kTR+3,kTR+12,kTL+0,kTL+10,    kTL+10,
    kTR+16,  kTR+16,kTR+19,kTL+13,kTL+17,  kTL+17,

    kTL+3,   kTL+3,kTL+9,kBL+0,kBL+4,      kBL+4,
    kTL+3,   kTL+3,kTL+12,kBL+0,kBL+10,    kBL+10,
    kTL+16,  kTL+16,kTL+19,kBL+13,kBL+17,  kBL+17,

    kBL+3,   kBL+3,kBL+9,kBR+0,kBR+4,      kBR+4,
    kBL+3,   kBL+3,kBL+12,kBR+0,kBR+10,    kBR+10,
    kBL+16,  kBL+16,kBL+19,kBR+13,kBR+17,  kBR+17
};

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
        SkM44 viewMatrix = canvas->getLocalToDevice();

        canvas->concat(this->basisMatrix());

        SkM44 totalMatrix = canvas->getLocalToDevice();

            // Base shape + style
            SkRRect rrect = this->primitiveShape();
            canvas->drawRRect(rrect, paint(SK_ColorBLUE, this->strokeWidth(), fJoinMode));
        canvas->restore();

        canvas->save();
        canvas->resetMatrix();
            // Draw the full mesh directly in device space
            this->drawVertices(canvas, totalMatrix);
            // Draw the controls in device space so we get consistent circles for the click points.
            SkV4 origin = viewMatrix.map(fOrigin.x, fOrigin.y, 0.f, 1.f);
            SkV4 xAxis = viewMatrix.map(fXAxisPoint.x, fXAxisPoint.y, 0.f, 1.f);
            SkV4 yAxis = viewMatrix.map(fYAxisPoint.x, fYAxisPoint.y, 0.f, 1.f);

            // Axes
            canvas->drawLine({origin.x/origin.w, origin.y/origin.w},
                             {xAxis.x/xAxis.w, xAxis.y/xAxis.w}, paint(SK_ColorRED, 0.f));
            canvas->drawLine({origin.x/origin.w, origin.y/origin.w},
                             {yAxis.x/yAxis.w, yAxis.y/yAxis.w}, paint(SK_ColorGREEN, 0.f));

            // Control points
            canvas->drawCircle({origin.x/origin.w, origin.y/origin.w},
                               kControlPointRadius, paint(SK_ColorBLACK));
            canvas->drawCircle({xAxis.x/xAxis.w, xAxis.y/xAxis.w},
                               kControlPointRadius, paint(SK_ColorRED));
            canvas->drawCircle({yAxis.x/yAxis.w, yAxis.y/yAxis.w},
                               kControlPointRadius, paint(SK_ColorGREEN));
        canvas->restore();
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
        SkRRect rrect = this->primitiveShape();
        float strokeRadius = 0.5f * this->strokeWidth();

        SkV3 points[80];
        SkPoint vertices[80];
        compute_vertices(points, ctm, rrect, strokeRadius, fJoinMode);
        // SkCanvas::drawVertices() wants SkPoint, not SkV3 but normally we'd let the GPU handle the
        // perspective division and clipping.
        for (int i = 0; i < 80; ++i) {
            vertices[i] = SkPoint{points[i].x/points[i].z, points[i].y/points[i].z};
        }

        auto drawMeshSubset = [vertices, canvas](SkColor color,
                                                 const uint16_t* indices,
                                                 size_t indexCount) {
            sk_sp<SkVertices> mesh = SkVertices::MakeCopy(
                    SkVertices::kTriangleStrip_VertexMode, 80, vertices,
                    nullptr, nullptr, (int) indexCount, indices);
            SkPaint meshPaint;
            meshPaint.setColor(color);
            meshPaint.setAlphaf(0.5f);
            canvas->drawVertices(mesh, SkBlendMode::kSrc, meshPaint);
        };
        drawMeshSubset(SK_ColorGRAY,    kEdgeIndices,        SK_ARRAY_COUNT(kEdgeIndices));
        drawMeshSubset(SK_ColorDKGRAY,  kInteriorIndices,    SK_ARRAY_COUNT(kInteriorIndices));
        drawMeshSubset(SK_ColorMAGENTA, kInnerCornerIndices, SK_ARRAY_COUNT(kInnerCornerIndices));
        drawMeshSubset(SK_ColorCYAN,    kOuterCornerIndices, SK_ARRAY_COUNT(kOuterCornerIndices));

        // Draw the edges over the triangle strip mesh, but keep track of edges already drawn so
        // that we don't oversaturate AA on edges shared by multiple triangles.
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
        for (size_t i = 2; i < SK_ARRAY_COUNT(kIndices); ++i) {
            drawEdge(kIndices[i-1], kIndices[i]);
            drawEdge(kIndices[i-2], kIndices[i]);
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
