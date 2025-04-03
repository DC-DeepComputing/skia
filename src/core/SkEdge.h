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

/**
    SkEdge approximates curves with one or more line segments in a way that makes computing
    scan lines (rows of horizontal pixels between multiple edges) efficient. The general flow
    is to create an SkEdge (or one of its subclasses), use the fields to represent a
    line segment, and then use nextSegment() (until hasNextSegment() is false) to update the
    fields with the next line segment begins.
*/
class SkEdge {
public:
    virtual ~SkEdge() = default;

    enum class Type : int8_t {
        kLine,
        kQuad,
        kCubic,
    };
    enum class Winding : int8_t {
        kCW = 1,    // clockwise
        kCCW = -1,  // counter clockwise
    };

    // Can be used to join edges together
    SkEdge* fNext;
    SkEdge* fPrev;

    // The current line segment starts at (fX, fFirstY + 0.5). It has slope
    // fDXDY (yes, run over rise, because this is geared toward horizontal scanlines)
    // and stops once Y gets to fLastY + 0.5.
    SkFixed fX;

    SkFixed fDxDy;
    // These are integers because they represent a discrete pixel. Mathematically, these are
    // treated as half way inside the pixel, so 6 -> 6.5.
    int32_t fFirstY;
    int32_t fLastY;
    Type    fEdgeType;
    Winding fWinding;

    bool setLine(const SkPoint& p0, const SkPoint& p1, const SkIRect* clip);
    // call this version if you know you don't have a clip
    inline bool setLine(const SkPoint& p0, const SkPoint& p1);

    bool hasNextSegment() const {
        return fSegmentCount != 0;
    }
    virtual bool nextSegment();

    uint8_t segmentsLeft() const {
        return fSegmentCount;
    }
protected:
    bool updateLine(SkFixed ax, SkFixed ay, SkFixed bx, SkFixed by);

    uint8_t fSegmentCount;  // only non-zero for Quad and Cubics
    uint8_t fCurveShift;    // appled to all Dx/DDx/DDDx except for fCubicDShift exception

private:
    void chopLineWithClip(const SkIRect& clip);

#ifdef SK_DEBUG
public:
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
    SkFixed fQDxDt, fQDyDt;
    SkFixed fQD2xDt2, fQD2yDt2;
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
    SkFixed fCDxDt, fCDyDt;
    SkFixed fCD2xDt2, fCD2yDt2;
    SkFixed fCD3xDt3, fCD3yDt3;
    SkFixed fCLastX, fCLastY;

    uint8_t fCubicDShift;   // applied to fCDxDt and fCDyDt only

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

    // Note that SkFixedMul(SkFixed, SkFDot6) produces results in SkFDot6
    fX          = SkFDot6ToFixed(x0 + SkFixedMul(slope, dy));
    fDxDy       = slope;
    fFirstY     = top;
    fLastY      = bot - 1;
    fEdgeType   = Type::kLine;
    fSegmentCount = 0;
    fWinding    = winding;
    fCurveShift = 0;
    return true;
}

#endif
