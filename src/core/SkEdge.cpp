/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkEdge.h"

#include "include/core/SkRect.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkSafe32.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkFDot6.h"

#include <algorithm>
#include <utility>

#ifdef SK_DEBUG
void SkEdge::dump() const {
    SkASSERT(fSegmentCount == 0);
    SkDebugf("line edge: firstY:%d lastY:%d x:%g dx/dy:%g\n"
             "\twinding:%d curveShift:%u\n",
             fFirstY,
             fLastY,
             SkFixedToFloat(fX),
             SkFixedToFloat(fDX),
             static_cast<int8_t>(fWinding),
             fCurveShift);
}

void SkQuadraticEdge::dump() const {
    SkDebugf("quad edge; %u curve(s) left: firstY:%d lastY:%d x:%g dx/dy:%g\n"
             "\tqx:%g qy:%g dqx:%g dqy:%g ddqx:%g ddqy:%g qLastX:%g qLastY:%g\n"
             "\twinding:%d curveShift:%u\n",
             fSegmentCount,
             fFirstY,
             fLastY,
             SkFixedToFloat(fX),
             SkFixedToFloat(fDX),
             SkFixedToFloat(fQx),
             SkFixedToFloat(fQy),
             SkFixedToFloat(fQDx),
             SkFixedToFloat(fQDy),
             SkFixedToFloat(fQDDx),
             SkFixedToFloat(fQDDy),
             SkFixedToFloat(fQLastX),
             SkFixedToFloat(fQLastY),
             static_cast<int8_t>(fWinding),
             fCurveShift);
}

void SkCubicEdge::dump() const {
    SkDebugf("cube edge; %u curve(s) left: firstY:%d lastY:%d x:%g dx/dy:%g\n"
             "qx:%g qy:%g dcx:%g dcy:%g ddcx:%g ddcy:%g dddcx:%g dddcy:%g cLastX:%g cLastY:%g\n"
             "\twinding:%d curveShift:%u dShift:%u\n",
             fSegmentCount,
             fFirstY,
             fLastY,
             SkFixedToFloat(fX),
             SkFixedToFloat(fDX),
             SkFixedToFloat(fCx),
             SkFixedToFloat(fCy),
             SkFixedToFloat(fCDx),
             SkFixedToFloat(fCDy),
             SkFixedToFloat(fCDDx),
             SkFixedToFloat(fCDDy),
             SkFixedToFloat(fCDDDx),
             SkFixedToFloat(fCDDDy),
             SkFixedToFloat(fCLastX),
             SkFixedToFloat(fCLastY),
             static_cast<int8_t>(fWinding),
             fCurveShift,
             fCubicDShift);
}

#endif

bool SkEdge::setLine(const SkPoint& p0, const SkPoint& p1, const SkIRect* clip) {
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
    // are we completely above or below the clip?
    if (clip && (top >= clip->fBottom || bot <= clip->fTop)) {
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

    if (clip) {
        this->chopLineWithClip(*clip);
    }
    return true;
}

bool SkEdge::nextSegment() {
    SkDEBUGFAILF("Shouldn't be asking a linear edge to go to the next curve.");
    return false;
}

bool SkEdge::updateLine(SkFixed x0, SkFixed y0, SkFixed x1, SkFixed y1) {
    SkASSERT(fWinding == Winding::kCW || fWinding == Winding::kCCW);
    SkASSERT(fSegmentCount != 0);

    y0 >>= 10;
    y1 >>= 10;

    SkASSERT(y0 <= y1);

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y1);

//  SkASSERT(top >= fFirstY);

    // are we a zero-height line?
    if (top == bot) {
        return false;
    }

    x0 >>= 10;
    x1 >>= 10;

    SkFixed slope = SkFDot6Div(x1 - x0, y1 - y0);
    const SkFDot6 dy  = SkEdge_Compute_DY(top, y0);

    fX          = SkFDot6ToFixed(x0 + SkFixedMul(slope, dy));
    fDX         = slope;
    fFirstY     = top;
    fLastY      = bot - 1;

    return true;
}

void SkEdge::chopLineWithClip(const SkIRect& clip)
{
    int top = fFirstY;

    SkASSERT(top < clip.fBottom);

    // clip the line to the top
    if (top < clip.fTop)
    {
        SkASSERT(fLastY >= clip.fTop);
        fX += fDX * (clip.fTop - top);
        fFirstY = clip.fTop;
    }
}

///////////////////////////////////////////////////////////////////////////////

/*  This limits the number of lines we use to approximate a curve.
    If we need to increase this, we need to store fSegmentCount in a larger data type.
    TODO(kjlubick): now that this is in an unsigned byte, we could go up to 7
*/
#define MAX_COEFF_SHIFT     6

static inline SkFDot6 cheap_distance(SkFDot6 dx, SkFDot6 dy)
{
    dx = SkAbs32(dx);
    dy = SkAbs32(dy);
    // return max + min/2
    if (dx > dy){
        dx += dy >> 1;
    } else {
        dx = dy + (dx >> 1);
    }
    return dx;
}

static inline int diff_to_shift(SkFDot6 dx, SkFDot6 dy, int accuracy)
{
    // cheap calc of distance from center of p0-p2 to the center of the curve
    SkFDot6 dist = cheap_distance(dx, dy);

    // shift down dist (it is currently in dot6)
    // down by 3 should give us 1/8 pixel accuracy (assuming our dist is accurate...)
    // this is chosen by heuristic: make it as big as possible (to minimize segments)
    // ... but small enough so that our curves still look smooth
    // For cubics still, we have shift > 0. TODO(kjlubick) can we align cubics and quads?
    dist = (dist + (1 << (2 + accuracy))) >> (3 + accuracy);

    // each subdivision (shift value) cuts this dist (error) by 1/4
    return (32 - SkCLZ(dist)) >> 1;
}

// Convert SkFDot6 to SkFixed and divide by 2 along the way. Rather than shift left 10 place
// and right one place, we just shift left 9 places to avoid throwing away data.
static inline SkFixed SkFDot6ToFixedDiv2(SkFDot6 value) {
    return SkLeftShift(value, 16 - 6 - 1);
}

bool SkQuadraticEdge::setQuadratic(const SkPoint pts[3]) {
    SkFDot6 x0, y0, x1, y1, x2, y2;

#ifdef SK_RASTERIZE_EVEN_ROUNDING
    x0 = SkScalarRoundToFDot6(pts[0].fX);
    y0 = SkScalarRoundToFDot6(pts[0].fY);
    x1 = SkScalarRoundToFDot6(pts[1].fX);
    y1 = SkScalarRoundToFDot6(pts[1].fY);
    x2 = SkScalarRoundToFDot6(pts[2].fX);
    y2 = SkScalarRoundToFDot6(pts[2].fY);
#else
    x0 = SkFloatToFDot6(pts[0].fX);
    y0 = SkFloatToFDot6(pts[0].fY);
    x1 = SkFloatToFDot6(pts[1].fX);
    y1 = SkFloatToFDot6(pts[1].fY);
    x2 = SkFloatToFDot6(pts[2].fX);
    y2 = SkFloatToFDot6(pts[2].fY);
#endif

    Winding winding = Winding::kCW;
    if (y0 > y2) {
        using std::swap;
        swap(x0, x2);
        swap(y0, y2);
        winding = Winding::kCCW;
    }
    SkASSERT(y0 <= y1 && y1 <= y2);

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y2);

    // are we a zero-height quad (line)?
    if (top == bot) {
        return false;
    }

    // compute number of steps needed (1 << shift)
    SkFDot6 dx = (SkLeftShift(x1, 1) - x0 - x2);
    SkFDot6 dy = (SkLeftShift(y1, 1) - y0 - y2);
    int shift = diff_to_shift(dx/4, dy/4, 0);
    SkASSERT(shift >= 0);

    // need at least 1 subdivision for our bias trick
    if (shift == 0) {
        shift = 1;
    } else if (shift > MAX_COEFF_SHIFT) {
        shift = MAX_COEFF_SHIFT;
    }

    fWinding = winding;
    fEdgeType = Type::kQuad;
    fSegmentCount = SkToS8(1 << shift);

    /*
     *  We want to reformulate into polynomial form, to make it clear how we
     *  should forward-difference.
     *
     *  p0 (1 - t)^2 + p1 t(1 - t) + p2 t^2 ==> At^2 + Bt + C
     *
     *  A = p0 - 2p1 + p2
     *  B = 2(p1 - p0)
     *  C = p0
     *
     *  Our caller must have constrained our inputs (p0..p2) to all fit into
     *  16.16. However, as seen above, we sometimes compute values that can be
     *  larger (e.g. B = 2*(p1 - p0)). To guard against overflow, we will store
     *  A and B at 1/2 of their actual value, and just apply a 2x scale during
     *  application in updateQuadratic(). Hence we store (shift - 1) in
     *  fCurveShift.
     */

    fCurveShift = SkToU8(shift - 1);

    SkFixed A = SkFDot6ToFixedDiv2(x0 - x1 - x1 + x2);  // 1/2 the real value
    SkFixed B = SkFDot6ToFixed(x1 - x0);                // 1/2 the real value

    fQx     = SkFDot6ToFixed(x0);
    fQDx    = B + (A >> shift);     // biased by shift
    fQDDx   = A >> (shift - 1);     // biased by shift

    A = SkFDot6ToFixedDiv2(y0 - y1 - y1 + y2);  // 1/2 the real value
    B = SkFDot6ToFixed(y1 - y0);                // 1/2 the real value

    fQy     = SkFDot6ToFixed(y0);
    fQDy    = B + (A >> shift);     // biased by shift
    fQDDy   = A >> (shift - 1);     // biased by shift

    fQLastX = SkFDot6ToFixed(x2);
    fQLastY = SkFDot6ToFixed(y2);

    return this->nextSegment();
}

bool SkQuadraticEdge::nextSegment() {
    bool    success;
    int     count = fSegmentCount;
    SkFixed oldx = fQx;
    SkFixed oldy = fQy;
    SkFixed dx = fQDx;
    SkFixed dy = fQDy;
    SkFixed newx, newy;
    int     shift = fCurveShift;

    SkASSERT(count > 0);

    do {
        if (--count > 0)
        {
            newx    = oldx + (dx >> shift);
            dx    += fQDDx;
            newy    = oldy + (dy >> shift);
            dy    += fQDDy;
        }
        else    // last segment
        {
            newx    = fQLastX;
            newy    = fQLastY;
        }
        success = this->updateLine(oldx, oldy, newx, newy);
        oldx = newx;
        oldy = newy;
    } while (count > 0 && !success);

    fQx         = newx;
    fQy         = newy;
    fQDx        = dx;
    fQDy        = dy;
    fSegmentCount = SkToS8(count);
    return success;
}

/////////////////////////////////////////////////////////////////////////

static inline int SkFDot6UpShift(SkFDot6 x, int upShift) {
    SkASSERT((SkLeftShift(x, upShift) >> upShift) == x);
    return SkLeftShift(x, upShift);
}

/*  f(1/3) = (8a + 12b + 6c + d) / 27
    f(2/3) = (a + 6b + 12c + 8d) / 27

    f(1/3)-b = (8a - 15b + 6c + d) / 27
    f(2/3)-c = (a + 6b - 15c + 8d) / 27

    use 16/512 to approximate 1/27
*/
static SkFDot6 cubic_delta_from_line(SkFDot6 a, SkFDot6 b, SkFDot6 c, SkFDot6 d)
{
    // since our parameters may be negative, we don't use << to avoid ASAN warnings
    SkFDot6 oneThird = (a*8 - b*15 + 6*c + d) * 19 >> 9;
    SkFDot6 twoThird = (a + 6*b - c*15 + d*8) * 19 >> 9;

    return std::max(SkAbs32(oneThird), SkAbs32(twoThird));
}

bool SkCubicEdge::setCubic(const SkPoint pts[4]) {
    SkFDot6 x0, y0, x1, y1, x2, y2, x3, y3;

#ifdef SK_RASTERIZE_EVEN_ROUNDING
    x0 = SkScalarRoundToFDot6(pts[0].fX);
    y0 = SkScalarRoundToFDot6(pts[0].fY);
    x1 = SkScalarRoundToFDot6(pts[1].fX);
    y1 = SkScalarRoundToFDot6(pts[1].fY);
    x2 = SkScalarRoundToFDot6(pts[2].fX);
    y2 = SkScalarRoundToFDot6(pts[2].fY);
    x3 = SkScalarRoundToFDot6(pts[3].fX);
    y3 = SkScalarRoundToFDot6(pts[3].fY);
#else
    x0 = SkFloatToFDot6(pts[0].fX);
    y0 = SkFloatToFDot6(pts[0].fY);
    x1 = SkFloatToFDot6(pts[1].fX);
    y1 = SkFloatToFDot6(pts[1].fY);
    x2 = SkFloatToFDot6(pts[2].fX);
    y2 = SkFloatToFDot6(pts[2].fY);
    x3 = SkFloatToFDot6(pts[3].fX);
    y3 = SkFloatToFDot6(pts[3].fY);
#endif

    Winding winding = Winding::kCW;
    if (y0 > y3)
    {
        using std::swap;
        swap(x0, x3);
        swap(x1, x2);
        swap(y0, y3);
        swap(y1, y2);
        winding = Winding::kCCW;
    }

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y3);

    // are we a zero-height cubic (line)?
    if (top == bot)
        return 0;

    // compute number of steps needed (1 << shift)
    // Can't use (center of curve - center of baseline), since center-of-curve
    // need not be the max delta from the baseline (it could even be coincident)
    // so we try just looking at the two off-curve points
    SkFDot6 dx = cubic_delta_from_line(x0, x1, x2, x3);
    SkFDot6 dy = cubic_delta_from_line(y0, y1, y2, y3);
    // add 1 (by observation)
    int shift = diff_to_shift(dx, dy, 2) + 1;
    // need at least 1 subdivision for our bias trick
    SkASSERT(shift > 0);
    if (shift > MAX_COEFF_SHIFT) {
        shift = MAX_COEFF_SHIFT;
    }

    /*  Our incoming data used to be shifted by 8 (for AA) or 10. That means we used to only be
        able to shift up is 8. However, we compute coefficients with a 3*, so the safest upshift
        is really 6.
        TODO(kjlubick) can this be increased now that AA is handled on its own edge type?
    */
    int upShift = 6;    // largest safe value
    int downShift = shift + upShift - 10;
    if (downShift < 0) {
        downShift = 0;
        upShift = 10 - shift;
    }

    fWinding = winding;
    fEdgeType = Type::kCubic;
    fSegmentCount = SkToS8(SkLeftShift(1, shift));
    fCurveShift = SkToU8(shift);
    fCubicDShift = SkToU8(downShift);

    SkFixed B = SkFDot6UpShift(3 * (x1 - x0), upShift);
    SkFixed C = SkFDot6UpShift(3 * (x0 - x1 - x1 + x2), upShift);
    SkFixed D = SkFDot6UpShift(x3 + 3 * (x1 - x2) - x0, upShift);

    fCx     = SkFDot6ToFixed(x0);
    fCDx    = B + (C >> shift) + (D >> 2*shift);    // biased by shift
    fCDDx   = 2*C + (3*D >> (shift - 1));           // biased by 2*shift
    fCDDDx  = 3*D >> (shift - 1);                   // biased by 2*shift

    B = SkFDot6UpShift(3 * (y1 - y0), upShift);
    C = SkFDot6UpShift(3 * (y0 - y1 - y1 + y2), upShift);
    D = SkFDot6UpShift(y3 + 3 * (y1 - y2) - y0, upShift);

    fCy     = SkFDot6ToFixed(y0);
    fCDy    = B + (C >> shift) + (D >> 2*shift);    // biased by shift
    fCDDy   = 2*C + (3*D >> (shift - 1));           // biased by 2*shift
    fCDDDy  = 3*D >> (shift - 1);                   // biased by 2*shift

    fCLastX = SkFDot6ToFixed(x3);
    fCLastY = SkFDot6ToFixed(y3);

    return this->nextSegment();
}

bool SkCubicEdge::nextSegment() {
    bool success;
    int  count = fSegmentCount;
    SkFixed oldx = fCx;
    SkFixed oldy = fCy;
    SkFixed newx, newy;
    const int ddshift = fCurveShift;
    const int dshift = fCubicDShift;

    SkASSERT(count < 0);

    do {
        if (--count < 0)
        {
            newx    = oldx + (fCDx >> dshift);
            fCDx    += fCDDx >> ddshift;
            fCDDx   += fCDDDx;

            newy    = oldy + (fCDy >> dshift);
            fCDy    += fCDDy >> ddshift;
            fCDDy   += fCDDDy;
        }
        else    // last segment
        {
            newx    = fCLastX;
            newy    = fCLastY;
        }

        // we want to say SkASSERT(oldy <= newy), but our finite fixedpoint
        // doesn't always achieve that, so we have to explicitly pin it here.
        if (newy < oldy) {
            newy = oldy;
        }

        success = this->updateLine(oldx, oldy, newx, newy);
        oldx = newx;
        oldy = newy;
    } while (count < 0 && !success);

    fCx         = newx;
    fCy         = newy;
    fSegmentCount = SkToS8(count);
    return success;
}
