/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEdge_DEFINED
#define SkEdge_DEFINED

#include "include/core/SkPoint.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFixed.h"
#include "src/core/SkFDot6.h"

#include <cstdint>
#include <utility>

struct SkIRect;

class SkEdge {
public:
    enum class Type : int8_t {
        kLine,
        kQuad,
        kCubic,
    };
    enum class Winding : int8_t {
        kCW = 1,    // clockwise
        kCCW = -1,  // counter clockwise
    };

    SkEdge* fNext;
    SkEdge* fPrev;

    SkFixed fX;             // where the line crosses this scanline
    SkFixed fDX;
    int32_t fFirstY;        // start of this scanline
    int32_t fLastY;
    Type    fEdgeType;      // Remembers the *initial* edge type
    Winding fWinding;

    bool setLine(const SkPoint& p0, const SkPoint& p1, const SkIRect* clip);
    // call this version if you know you don't have a clip
    inline bool setLine(const SkPoint& p0, const SkPoint& p1);

    bool hasNextSegment() {
        return fSegmentCount != 0;
    }
    virtual bool nextSegment();
protected:
    bool updateLine(SkFixed ax, SkFixed ay, SkFixed bx, SkFixed by);

    uint8_t fSegmentCount;  // only non-zero for Quad and Cubics
    uint8_t fCurveShift;    // appled to all Dx/DDx/DDDx except for fCubicDShift exception

private:
    void chopLineWithClip(const SkIRect& clip);

#ifdef SK_DEBUG
public:
    virtual ~SkEdge() = default;
    virtual void dump() const;
    void validate() const {
        SkASSERT(fPrev && fNext);
        SkASSERT(fPrev->fNext == this);
        SkASSERT(fNext->fPrev == this);

        SkASSERT(fFirstY <= fLastY);
        SkASSERT(fWinding == Winding::kCW || fWinding == Winding::kCCW);
    }
#endif
};

class SkQuadraticEdge final : public SkEdge {
public:
    bool setQuadratic(const SkPoint pts[3]);
    bool nextSegment() override;

private:
    SkFixed fQx, fQy;
    SkFixed fQDx, fQDy;
    SkFixed fQDDx, fQDDy;
    SkFixed fQLastX, fQLastY;

#ifdef SK_DEBUG
public:
    void dump() const override;
#endif
};

class SkCubicEdge final : public SkEdge {
public:
    bool setCubic(const SkPoint pts[4]);
    bool nextSegment() override;

private:
    SkFixed fCx, fCy;
    SkFixed fCDx, fCDy;
    SkFixed fCDDx, fCDDy;
    SkFixed fCDDDx, fCDDDy;
    SkFixed fCLastX, fCLastY;

    uint8_t fCubicDShift;   // applied to fCDx and fCDy only

#ifdef SK_DEBUG
public:
    void dump() const override;
#endif
};

// This correctly favors the lower-pixel when y0 is on a 1/2 pixel boundary
constexpr SkFDot6 SkEdge_Compute_DY(int top, SkFDot6 y0) {
    return SkIntToFDot6(top) + SK_FDot6Half - y0;
}

bool SkEdge::setLine(const SkPoint& p0, const SkPoint& p1) {
    SkFDot6 x0, y0, x1, y1;

#ifdef SK_RASTERIZE_EVEN_ROUNDING
    x0 = SkScalarRoundToFDot6(p0.fX);
    y0 = SkScalarRoundToFDot6(p0.fY);
    x1 = SkScalarRoundToFDot6(p1.fX);
    y1 = SkScalarRoundToFDot6(p1.fY);
#else
    x0 = SkFloatToFDot6(p0.fX);
    y0 = SkFloatToFDot6(p0.fY);
    x1 = SkFloatToFDot6(p1.fX);
    y1 = SkFloatToFDot6(p1.fY);
#endif

    Winding winding = Winding::kCW;

    if (y0 > y1) {
        using std::swap;
        swap(x0, x1);
        swap(y0, y1);
        winding = Winding::kCCW;
    }

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y1);

    // are we a zero-height line?
    if (top == bot) {
        return false;
    }

    SkFixed slope = SkFDot6Div(x1 - x0, y1 - y0);
    const SkFDot6 dy  = SkEdge_Compute_DY(top, y0);

    fX          = SkFDot6ToFixed(x0 + SkFixedMul(slope, dy));
    fDX         = slope;
    fFirstY     = top;
    fLastY      = bot - 1;
    fEdgeType   = Type::kLine;
    fSegmentCount = 0;
    fWinding    = winding;
    fCurveShift = 0;
    return true;
}

#endif
