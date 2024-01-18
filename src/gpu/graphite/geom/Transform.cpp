/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/geom/Transform_graphite.h"

#include "src/base/SkVx.h"
#include "src/core/SkMatrixPriv.h"
#include "src/gpu/graphite/geom/Rect.h"

namespace skgpu::graphite {

namespace {

Rect map_rect(const SkM44& m, const Rect& r) {
    // TODO: Can Rect's (l,t,-r,-b) structure be used to optimize mapRect?
    // TODO: Can take this opportunity to implement 100% accurate perspective plane clipping since
    //       it doesn't have to match raster/ganesh rendering behavior.
    return SkMatrixPriv::MapRect(m, r.asSkRect());
}

void map_points(const SkM44& m, const SkV4* in, SkV4* out, int count) {
    // TODO: These maybe should go into SkM44, since bulk point mapping seems generally useful
    auto c0 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(m) + 0);
    auto c1 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(m) + 4);
    auto c2 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(m) + 8);
    auto c3 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(m) + 12);

    for (int i = 0; i < count; ++i) {
        auto p = (c0 * in[i].x) + (c1 * in[i].y) + (c2 * in[i].z) + (c3 * in[i].w);
        p.store(out + i);
    }
}

// Returns singular value decomposition of the 2x2 matrix [m00 m01] as {min, max}
//                                                        [m10 m11]
std::pair<float, float> compute_svd(float m00, float m01, float m10, float m11) {
    // no-persp, these are the singular values of [m00,m01][m10,m11], which is just the upper 2x2
    // and equivalent to SkMatrix::getMinmaxScales().
    float s1 = m00*m00 + m01*m01 + m10*m10 + m11*m11;

    float e = m00*m00 + m01*m01 - m10*m10 - m11*m11;
    float f = m00*m10 + m01*m11;
    float s2 = SkScalarSqrt(e*e + 4*f*f);

    // s2 >= 0, so (s1 - s2) <= (s1 + s2)
    return {SkScalarSqrt(0.5f * (s1 - s2)),
            SkScalarSqrt(0.5f * (s1 + s2))};
}

std::pair<float, float> sort_scale(float sx, float sy) {
    float min = std::abs(sx);
    float max = std::abs(sy);
    if (min > max) {
        return {max, min};
    } else {
        return {min, max};
    }
}

std::tuple<Transform::Type, float, float> get_matrix_info(const SkM44& m, SkM44* inverse) {
    // First compute the inverse.
    // TODO: Alternatively we could compute type first and have type-specific inverses, but it seems
    // useful to ensure any non-invalid matrix returns true from SkM44::invert.
    if (!m.invert(inverse)) {
        return {Transform::Type::kInvalid, 1.f, 1.f};
    }

    static constexpr SkV4 kNoPerspective = {0.f, 0.f, 0.f, 1.f};
    static constexpr SkV4 kNoZ = {0.f, 0.f, 1.f, 0.f};
    if (m.row(3) != kNoPerspective || m.col(2) != kNoZ || m.row(2) != kNoZ) {
        // Projection matrices will have per-location scale factors calculated, so cached SVD will
        // not be read.
        return {Transform::Type::kProjection, 1.f, 1.f};
    }

    //                                              [sx kx 0 tx]
    // At this point, we know that m is of the form [ky sy 0 ty]
    //                                              [0  0  1 0 ]
    //                                              [0  0  0 1 ]
    // Other than kIdentity, none of the types depend on (tx, ty). The remaining types are
    // identified by considering the upper 2x2.
    float sx = m.rc(0, 0);
    float sy = m.rc(1, 1);
    float kx = m.rc(0, 1);
    float ky = m.rc(1, 0);
    if (kx == 0.f && ky == 0.f) {
        // 2x2 is a diagonal matrix
        auto [min, max] = sort_scale(sx, sy);
        if (sx == 1.f && sy == 1.f && m.rc(0, 3) == 0.f && m.rc(1, 3) == 0.f) {
            return {Transform::Type::kIdentity, min, max};
        } else if (sx > 0.f && sy > 0.f) {
            return {Transform::Type::kSimpleRectStaysRect, min, max};
        } else {
            // We don't need to worry about sx or sy being 0 here because that would imply the
            // matrix wasn't invertible and that was already tested.
            SkASSERT(sx != 0.f && sy != 0.f);
            return {Transform::Type::kRectStaysRect, min, max};
        }
    } else if (sx == 0.f && sy == 0.f) {
        // 2x2 is an anti-diagonal matrix and represents a 90 or 270 degree rotation plus optional
        // scale and translate. Similar to before, kx and ky can't be 0 or m wouldn't be invertible.
        SkASSERT(kx != 0.f && ky != 0.f);
        auto [min, max] = sort_scale(kx, ky);\
        return {Transform::Type::kRectStaysRect, min, max};
    } else {
        auto [min, max] = compute_svd(sx, kx, ky, sy);
        return {Transform::Type::kAffine, min, max};
    }
}

} // anonymous namespace

Transform::Transform(const SkM44& m) : fM(m) {
    std::tie(fType, fMinScaleFactor, fMaxScaleFactor) = get_matrix_info(m, &fInvM);
    SkDEBUGCODE(this->validate();)
}

bool Transform::operator==(const Transform& t) const {
    // Checking fM should be sufficient as all other values are computed from it.
    SkASSERT(fM != t.fM || (fInvM == t.fInvM &&
                            fType == t.fType &&
                            SkScalarNearlyEqual(fMinScaleFactor, t.fMinScaleFactor) &&
                            SkScalarNearlyEqual(fMaxScaleFactor, t.fMaxScaleFactor)));
    return fM == t.fM;
}

#ifdef SK_DEBUG
void Transform::validate() const {
    // Not all constructors go through get_matrix_info, so make sure the type/inverse is consistent.
    SkM44 inverse;
    auto [expected, min, max] = get_matrix_info(fM, &inverse);
    SkASSERT(expected == fType);
    SkASSERT(expected == Type::kProjection ||
             (fM.rc(3,0) == 0.f && fM.rc(3,1) == 0.f && fM.rc(3,2) == 0.f && fM.rc(3,3) == 1.f));

    SkASSERT(fMinScaleFactor <= fMaxScaleFactor);
    SkASSERT(SkScalarNearlyEqual(fMinScaleFactor, min));
    SkASSERT(SkScalarNearlyEqual(fMaxScaleFactor, max));

    if (expected != Type::kInvalid && expected != Type::kProjection) {
        // Not all types compute the actual SVD, so make sure their scale factors are consistent.
        auto [svdMin, svdMax] = compute_svd(fM.rc(0,0), fM.rc(0,1), fM.rc(1,0), fM.rc(1,1));
        SkASSERT(SkScalarNearlyEqual(fMinScaleFactor, svdMin));
        SkASSERT(SkScalarNearlyEqual(fMaxScaleFactor, svdMax));
    } else {
        // These types don't have constant scale factors
        SkASSERT(fMinScaleFactor == 1.f && fMaxScaleFactor == 1.f);
    }
}
#endif

std::pair<float, float> Transform::scaleFactors(const SkV2& p) const {
    SkASSERT(fType != Type::kInvalid);
    if (fType < Type::kProjection) {
        return {fMinScaleFactor, fMaxScaleFactor};
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
    // df/du = (dx/du*w - x*dw/du)/w^2 = (m00*w - m30*x)/w^2 = (m00 - m30*f)/w
    // df/dv = (dx/dv*w - x*dw/dv)/w^2 = (m01*w - m31*x)/w^2 = (m01 - m31*f)/w
    // dg/du = (dy/du*w - y*dw/du)/w^2 = (m10*w - m30*y)/w^2 = (m10 - m30*g)/w
    // dg/dv = (dy/dv*w - y*dw/du)/w^2 = (m11*w - m31*y)/w^2 = (m11 - m31*g)/w
    //
    // Singular values of [df/du df/dv] define perspective correct minimum and maximum scale factors
    //                    [dg/du dg/dv]
    // for M evaluated at  (u,v)
    SkV4 devP = fM.map(p.x, p.y, 0.f, 1.f);

    const float dxdu = fM.rc(0,0);
    const float dxdv = fM.rc(0,1);
    const float dydu = fM.rc(1,0);
    const float dydv = fM.rc(1,1);
    const float dwdu = fM.rc(3,0);
    const float dwdv = fM.rc(3,1);

    float invW2 = sk_ieee_float_divide(1.f, (devP.w * devP.w));
    // non-persp has invW2 = 1, devP.w = 1, dwdu = 0, dwdv = 0
    float dfdu = (devP.w*dxdu - devP.x*dwdu) * invW2; // non-persp -> dxdu -> m00
    float dfdv = (devP.w*dxdv - devP.x*dwdv) * invW2; // non-persp -> dxdv -> m01
    float dgdu = (devP.w*dydu - devP.y*dwdu) * invW2; // non-persp -> dydu -> m10
    float dgdv = (devP.w*dydv - devP.y*dwdv) * invW2; // non-persp -> dydv -> m11

    // no-persp, these are the singular values of [m00,m01][m10,m11], which was already calculated
    // in get_matrix_info.
    return compute_svd(dfdu, dfdv, dgdu, dgdv);
}


float Transform::localAARadius(const Rect& bounds) const {
    SkASSERT(fType != Type::kInvalid);
    float min;

    if (fType < Type::kProjection) {
        // The scale factor is constant
        min = fMinScaleFactor;
    } else {
        // Calculate the minimum scale factor over the 4 corners of the bounding box
        float tl = std::get<0>(this->scaleFactors(SkV2{bounds.left(), bounds.top()}));
        float tr = std::get<0>(this->scaleFactors(SkV2{bounds.right(), bounds.top()}));
        float br = std::get<0>(this->scaleFactors(SkV2{bounds.right(), bounds.bot()}));
        float bl = std::get<0>(this->scaleFactors(SkV2{bounds.left(), bounds.bot()}));
        min = std::min(std::min(tl, tr), std::min(br, bl));
    }

    // Moving 1 from 'p' before transforming will move at least 'min' and at most 'max' from
    // the transformed point. Thus moving between [1/max, 1/min] pre-transformation means post
    // transformation moves between [1,max/min] so using 1/min as the local AA radius ensures that
    // the post-transformed point is at least 1px away from the original.
    float aaRadius = sk_ieee_float_divide(1.f, min);
    if (sk_float_isfinite(aaRadius)) {
        return aaRadius;
    } else {
        return SK_FloatInfinity;
    }
}

Rect Transform::mapRect(const Rect& rect) const { return map_rect(fM, rect); }
Rect Transform::inverseMapRect(const Rect& rect) const { return map_rect(fInvM, rect); }

void Transform::mapPoints(const Rect& localRect, SkV4 deviceOut[4]) const {
    SkV2 localCorners[4] = {{localRect.left(),  localRect.top()},
                            {localRect.right(), localRect.top()},
                            {localRect.right(), localRect.bot()},
                            {localRect.left(),  localRect.bot()}};
    this->mapPoints(localCorners, deviceOut, 4);
}

void Transform::mapPoints(const SkV2* localIn, SkV4* deviceOut, int count) const {
    // TODO: These maybe should go into SkM44, since bulk point mapping seems generally useful
    auto c0 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(fM) + 0);
    auto c1 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(fM) + 4);
    // skip c2 since localIn's z is assumed to be 0
    auto c3 = skvx::float4::Load(SkMatrixPriv::M44ColMajor(fM) + 12);

    for (int i = 0; i < count; ++i) {
        auto p = c0 * localIn[i].x + c1 * localIn[i].y /* + c2*0.f */ + c3 /* *1.f */;
        p.store(deviceOut + i);
    }
}

void Transform::mapPoints(const SkV4* localIn, SkV4* deviceOut, int count) const {
    return map_points(fM, localIn, deviceOut, count);
}

void Transform::inverseMapPoints(const SkV4* deviceIn, SkV4* localOut, int count) const {
    return map_points(fInvM, deviceIn, localOut, count);
}

Transform Transform::preTranslate(float x, float y) const {
    Transform t = *this;
    t.fM.preTranslate(x, y);
    t.fInvM.postTranslate(-x, -y);

    // Under normal conditions, type and scale won't change, but if we've overflown the translation
    // components, mark the matrix as invalid.
    if (!t.fM.isFinite() || !t.fInvM.isFinite()) {
        t.fType = Type::kInvalid;
    }
    SkDEBUGCODE(t.validate();)
    return t;
}

Transform Transform::postTranslate(float x, float y) const {
    Transform t = *this;
    t.fM.postTranslate(x, y);
    t.fInvM.preTranslate(-x, -y);
    if (!t.fM.isFinite() || !t.fInvM.isFinite()) {
        t.fType = Type::kInvalid;
    }
    SkDEBUGCODE(t.validate();)
    return t;
}

// FIXME might need to bring back transform ctor arg for SVD
// FIXME what are SVD rules for matrix products?
//   hmm, it's not a seeming like SVD(AB) = SVD(A) * SVD(B) for their singular values...
//   maybe there are some fast paths that would let me do this, but doesn't seem to be the case
//   for general solution.
//
// M = U*S*V^T
// U and V are unitary, e.g. U*U^T = I and U^T*U = I
// S is diagonal, non-negative reals and are the singular values of M
//
// M1*M2 = U1*S1*V1^T*U2*S2*V2^T
//       = U1*(V1*S1^T)^T*(S2^T*U2^T)^T*V2^T
//       = U1*(S2^T*U2^T*V1*S1^T)*V2^T


//       = U1*V1*(S1*S2)*(V2*U2)^T
//
// but is U1*V1 unitary? and is (V2*U2) unitary?
//
// (U1*V1)*(U1*V1)^T = U1*V1*V1^T*U1^T = U1*U1^T = I check
// (U1*V1)^T*(U1*V1) = V1^T*U1^T*U1*V1 = V1^T*V1 = I check
// (V2*U2)^T*(V2*U2) = U2^T*V2^T*V2*U2 = U2^T*U2 = I check ...
//
// What about the inverse case? are SV's just reciprocoals?
//
// M^-1 = (U*S*V^T)^-1 = V^T^-1*S^-1*U^-1 = V*S^-1*U^T and S^-1 is a diagonal with reciprocals
// so yeah, inverse svd is just 1/svd


Transform Transform::concat(const Transform& t) const {
    // Let's say that A = (sx=4,sy=1) and B = (sx=1,sy=4)
    // in both cases max scale factor is 4 and min scale factor is 1
    // but A*B = (scale 4) and has a max scale factor of 4 as well.
    // and not 16! so yeah, this is just bogus.
    float min = fMinScaleFactor * t.fMinScaleFactor;
    float max = fMaxScaleFactor * t.fMaxScaleFactor;
    Transform c = {fM * t.fM, t.fInvM * fInvM, std::max(fType, t.fType), min, max};
    if (!c.fM.isFinite() || !c.fInvM.isFinite()) {
        c.fType = Type::kInvalid;
    }
    SkDEBUGCODE(c.validate();)
    return c;
}

Transform Transform::concatInverse(const Transform& t) const {
    // This is invalid too
    float min = fMinScaleFactor / t.fMaxScaleFactor;
    float max = fMaxScaleFactor / t.fMinScaleFactor;
    Transform c = {fM * t.fInvM, t.fM * fInvM, std::max(fType, t.fType), min, max};
    if (!c.fM.isFinite() || !c.fInvM.isFinite()) {
        c.fType = Type::kInvalid;
    }
    SkDEBUGCODE(c.validate();)
    return c;
}

Transform Transform::concatInverse(const SkM44& t) const {
    // saves a multiply compared to inverting just t and then computing fM*t^-1 and t*fInvM, if we
    // instead start with (t*fInvM) and swap definition of computed fM and fInvM.
    Transform inverse{t * fInvM};
    // The singular values of the inverse are the reciprocals of the original values
    float min = 1.f / inverse.fMaxScaleFactor;
    float max = 1.f / inverse.fMinScaleFactor;
    Transform c = {inverse.fInvM, inverse.fM, inverse.fType, min, max};
    SkDEBUGCODE(c.validate();)
    return c;
}

} // namespace skgpu::graphite
