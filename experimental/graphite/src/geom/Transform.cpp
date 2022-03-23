/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/geom/Transform_graphite.h"

#include "experimental/graphite/src/geom/Rect.h"
#include "experimental/graphite/src/geom/VectorTypes.h"
#include "src/core/SkMatrixPriv.h"

namespace skgpu {

namespace {

Rect map_rect(const SkM44& m, const Rect& r) {
    // TODO: Can Rect's (l,t,-r,-b) structure be used to optimize mapRect?
    // TODO: Can take this opportunity to implement 100% accurate perspective plane clipping since
    //       it doesn't have to match raster/ganesh rendering behavior.
    return SkMatrixPriv::MapRect(m, r.asSkRect());
}

Transform::Type detect_type(const SkM44& m) {
    static constexpr SkV4 kNoPerspective = {0.f, 0.f, 0.f, 1.f};
    static constexpr SkV4 kNoZ = {0.f, 0.f, 1.f, 0.f};

    if (m.row(3) != kNoPerspective || m.col(2) != kNoZ || m.row(2) != kNoZ) {
        // TODO: If there was no perspective, but the transform modifies Z, we could possibly argue
        // for discarding z (orthographic projection) and still let the type be a lower value, but
        // this seems safer for now.
        return Transform::Type::kPerspective;
    } else if (m.rc(1,0) == 0.f && m.rc(0,1) == 0.f) {
        float sx = m.rc(0, 0);
        float sy = m.rc(1, 1);
        if (sx == 1.f && sy == 1.f && m.rc(0, 3) == 0.f && m.rc(1, 3) == 0.f) {
            return Transform::Type::kIdentity;
        } else if (sx > 0.f && sy > 0.f) {
            return Transform::Type::kSimpleRectStaysRect;
        } else {
            return Transform::Type::kRectStaysRect;
        }
    } else {
        return Transform::Type::kAffine;
    }
}

} // anonymous namespace

Transform::Transform(const SkM44& m)
        : fM(m) {
    if (fM.invert(&fInvM)) {
        fType = detect_type(m);
        // TODO: Actually calculate scale factor (not needed until stroking is implemented though)
        fScale = {1.f, 1.f};
    } else {
        fType = Type::kInvalid;
        fInvM = SkM44();
        fScale = {1.f, 1.f};
    }
}

bool Transform::operator==(const Transform& t) const {
    // Checking fM should be sufficient as all other values are computed from it.
    SkASSERT(fM != t.fM || (fInvM == t.fInvM && fType == t.fType && fScale == t.fScale));
    return fM == t.fM;
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
    float4 c0 = float4::Load(SkMatrixPriv::M44ColMajor(fM) + 0);
    float4 c1 = float4::Load(SkMatrixPriv::M44ColMajor(fM) + 4);
    // skip c2 since localIn's z is assumed to be 0
    float4 c3 = float4::Load(SkMatrixPriv::M44ColMajor(fM) + 12);

    for (int i = 0; i < count; ++i) {
        float4 p = c0 * localIn[i].x + c1 * localIn[i].y /* + c2*0.f */ + c3 /* *1.f */;
        p.store(deviceOut + i);
    }
}

} // namespace skgpu
