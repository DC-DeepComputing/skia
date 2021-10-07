/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_geom_Rect_DEFINED
#define skgpu_geom_Rect_DEFINED

#include <limits>
#include "experimental/graphite/include/GraphiteTypes.h"
#include "include/core/SkRect.h"

namespace skgpu::geom {

/**
 * SIMD rect implementation. Vales are stored internally in the form: [left, top, -right, -bot].
 *
 * Some operations (e.g., intersect, inset) may return a negative or empty rect
 * (negative meaning, left >= right or top >= bot).
 *
 * Operations on a rect that either is negative or empty, while well-defined, might not give the
 * intended result. It is the caller's responsibility to check isEmptyOrNegative() if needed.
 */
class Rect {
public:
    Rect() = default;
    Rect(float l, float t, float r, float b) : fVals(l, t, -r, -b) {}
    Rect(float2 topLeft, float2 botRight) : fVals(topLeft, -botRight) {}

    static Rect MakeLTRB(float l, float t, float r, float b) {
        return Rect(l, t, r, b);
    }
    static Rect MakeTopLeftBotRight(float2 topLeft, float2 botRight) {
        return Rect(topLeft, botRight);
    }
    static Rect MakeXYWH(float x, float y, float w, float h) {
        return MakeTopLeftSize(float2(x,y), float2(w,h));
    }
    static Rect MakeTopLeftSize(float2 topLeft, float2 size) {
        return Rect(topLeft, topLeft + size);
    }
    static Rect MakeWH(float w, float h) {
        return Rect(0, 0, w, h);
    }
    static Rect MakeSize(float2 size) {
        return Rect(float2(0), size);
    }
    static Rect MakeSkRect(const SkRect& skRect) {
        return Rect(skRect.left(), skRect.top(), skRect.right(), skRect.bottom());
    }

    float x() const { return fVals.x(); }
    float y() const { return fVals.y(); }
    float left() const { return fVals.x(); }
    float top() const { return fVals.y(); }
    float right() const { return -fVals.z(); }
    float bot() const { return -fVals.w(); }
    float2 topLeft() const { return fVals.xy(); }
    float2 botRight() const { return -fVals.zw(); }
    float4 ltrb() const { return float4(fVals.xy(), -fVals.zw()); }

    // Is left >= right, or top >= bot?
    bool isEmptyOrNegative() const { return skvx::any(fVals.xy() + fVals.zw() >= 0); }

    float2 size() const { return -(fVals.xy() + fVals.zw()); }

    float2 center() const {
        float4 p = fVals * float4(.5f, .5f, -.5f, -.5f);
        return p.xy() + p.zw();
    }

    float area() const {
        float2 negativeSize = fVals.xy() + fVals.zw();
        return negativeSize.x() * negativeSize.y();
    }

    // A rect stored in a complementary form of: [right, bottom, -left, -top]. Store a local
    // ComplementRect object if intersects() will be called many times.
    struct ComplementRect {
        ComplementRect(Rect rect) : fVals(-rect.fVals.zwxy()) {}
        ComplementRect(float2 pt) : fVals(pt, -pt) {}
        float4 fVals;  // [right, bottom, -left, -top]
    };

    bool intersects(ComplementRect complement) const { return skvx::all(fVals < complement.fVals); }
    bool contains(Rect rect) const { return skvx::all(fVals < rect.fVals); }
    bool contains(float2 pt) const { return skvx::all(fVals < float4(pt,-pt)); }

    // Some operations may return a negative or empty rect. Operations on a rect that either is
    // negative or empty, while well-defined, might not give the intended result. It is the caller's
    // responsibility to check isEmptyOrNegative() if needed.
    Rect makeRoundIn() const { return skvx::ceil(fVals); }
    Rect makeRoundOut() const { return skvx::floor(fVals); }
    Rect makeInset(float inset) const { return fVals - inset; }
    Rect makeInset(float2 inset) const { return fVals - inset.xyxy(); }
    Rect makeOutset(float outset) const { return fVals - outset; }
    Rect makeOutset(float2 outset) const { return fVals - outset.xyxy(); }
    Rect makeOffset(float offset) const { return fVals + float4(offset, offset, -offset, -offset); }
    Rect makeOffset(float2 offset) const { return fVals + float4(offset, -offset); }
    Rect makeJoin(Rect rect) const { return skvx::min(fVals, rect.fVals); }
    Rect makeIntersect(const Rect& rect) const { return skvx::max(fVals, rect.fVals); }

    void roundIn() { *this = this->makeRoundIn(); }
    void roundOut() { *this = this->makeRoundOut(); }
    void inset(float inset) { *this = this->makeInset(inset); }
    void inset(float2 inset) { *this = this->makeInset(inset); }
    void outset(float outset) { *this = this->makeOutset(outset); }
    void outset(float2 outset) { *this = this->makeOutset(outset); }
    void offset(float offset) { *this = this->makeOffset(offset); }
    void offset(float2 offset) { *this = this->makeOffset(offset); }
    void join(Rect rect) { *this = this->makeJoin(rect); }
    void intersect(Rect& rect) { *this = this->makeIntersect(rect); }

private:
    Rect(float4 vals) : fVals(vals) {}  // vals.zw must already be negated.

    float4 fVals;  // [left, top, -right, -bottom]
};

} // namespace skgpu::geom

#endif // skgpu_geom_Rect_DEFINED
