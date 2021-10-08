/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_geom_Rect_DEFINED
#define skgpu_geom_Rect_DEFINED

#include "experimental/graphite/include/GraphiteTypes.h"
#include "include/core/SkRect.h"

namespace skgpu::geom {

#define AI SK_ALWAYS_INLINE

/**
 * SIMD rect implementation. Vales are stored internally in the form: [left, top, -right, -bot].
 *
 * Some operations (e.g., intersect, inset) may return a negative or empty rect
 * (negative meaning, left >= right or top >= bot).
 *
 * Operations on a rect that is either negative or empty, while well-defined, might not give the
 * intended result. It is the caller's responsibility to check isEmptyOrNegative() if needed.
 */
class Rect {
public:
    AI Rect() = default;
    AI Rect(float l, float t, float r, float b) : fVals(l, t, -r, -b) {}
    AI Rect(float2 topLeft, float2 botRight) : fVals(topLeft, -botRight) {}
    AI Rect(const SkRect& r) : Rect(Rect(r.left(), r.top(), r.right(), r.bottom())) {}

    AI static Rect XYWH(float x, float y, float w, float h) {
        return Rect(x, y, x + w, y + h);
    }
    AI static Rect XYWH(float2 topLeft, float2 size) {
        return Rect(topLeft, topLeft + size);
    }
    AI static Rect WH(float w, float h) {
        return Rect(0, 0, w, h);
    }
    AI static Rect WH(float2 size) {
        return Rect(float2(0), size);
    }

    AI float x() const { return fVals.x(); }
    AI float y() const { return fVals.y(); }
    AI float left() const { return fVals.x(); }
    AI float top() const { return fVals.y(); }
    AI float right() const { return -fVals.z(); }
    AI float bot() const { return -fVals.w(); }
    AI float2 topLeft() const { return fVals.xy(); }
    AI float2 botRight() const { return -fVals.zw(); }
    AI float4 ltrb() const { return float4(fVals.xy(), -fVals.zw()); }

    AI bool isEmptyOrNegative() const {
        return skvx::any(fVals.xy() + fVals.zw() >= 0);  // == ([l-r, r-b] >= 0) == ([w, h] <= 0)
    }

    AI float2 size() const { return -(fVals.xy() + fVals.zw()); }  // == [-(l-r), -(t-b)] == [w, h]

    AI float2 center() const {
        float4 p = fVals * float4(.5f, .5f, -.5f, -.5f);  // == [l, t, r, b] * .5
        return p.xy() + p.zw();  // == [(l + r)/2, (t + b)/2]
    }

    AI float area() const {
        float2 negativeSize = fVals.xy() + fVals.zw();  // == [l-r, t-b] == [-w, -h]
        return negativeSize.x() * negativeSize.y();
    }

    // A rect stored in a complementary form of: [right, bottom, -left, -top]. Store a local
    // ComplementRect object if intersects() will be called many times.
    struct ComplementRect {
        AI ComplementRect(Rect rect) : fVals(-rect.fVals.zwxy()) {}
        float4 fVals;  // [right, bottom, -left, -top]
    };

    AI bool intersects(ComplementRect comp) const { return skvx::all(fVals < comp.fVals); }
    AI bool contains(Rect rect) const { return skvx::all(fVals < rect.fVals); }
    AI bool contains(float2 pt) const { return skvx::all(fVals < float4(pt,-pt)); }

    // Some operations may return a negative or empty rect. Operations on a rect that either is
    // negative or empty, while well-defined, might not give the intended result. It is the caller's
    // responsibility to check isEmptyOrNegative() if needed.
    AI Rect makeRoundIn() const { return skvx::ceil(fVals); }
    AI Rect makeRoundOut() const { return skvx::floor(fVals); }
    AI Rect makeInset(float inset) const { return fVals + inset; }
    AI Rect makeInset(float2 inset) const { return fVals + inset.xyxy(); }
    AI Rect makeOutset(float outset) const { return fVals - outset; }
    AI Rect makeOutset(float2 outset) const { return fVals - outset.xyxy(); }
    AI Rect makeOffset(float2 offset) const { return fVals + float4(offset, -offset); }
    AI Rect makeJoin(Rect rect) const { return skvx::min(fVals, rect.fVals); }
    AI Rect makeIntersect(const Rect& rect) const { return skvx::max(fVals, rect.fVals); }

    AI void roundIn() { *this = this->makeRoundIn(); }
    AI void roundOut() { *this = this->makeRoundOut(); }
    AI void inset(float inset) { *this = this->makeInset(inset); }
    AI void inset(float2 inset) { *this = this->makeInset(inset); }
    AI void outset(float outset) { *this = this->makeOutset(outset); }
    AI void outset(float2 outset) { *this = this->makeOutset(outset); }
    AI void offset(float2 offset) { *this = this->makeOffset(offset); }
    AI void join(Rect rect) { *this = this->makeJoin(rect); }
    AI void intersect(Rect& rect) { *this = this->makeIntersect(rect); }

private:
    AI Rect(float4 vals) : fVals(vals) {}  // vals.zw must already be negated.

    float4 fVals;  // [left, top, -right, -bottom]
};

#undef AI

} // namespace skgpu::geom

#endif // skgpu_geom_Rect_DEFINED
