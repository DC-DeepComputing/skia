/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRect_DEFINED
#define GrRect_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"

struct GrIRect16 {
    int16_t fLeft, fTop, fRight, fBottom;

    static GrIRect16 SK_WARN_UNUSED_RESULT MakeEmpty() {
        GrIRect16 r;
        r.setEmpty();
        return r;
    }

    static GrIRect16 SK_WARN_UNUSED_RESULT MakeWH(int16_t w, int16_t h) {
        GrIRect16 r;
        r.set(0, 0, w, h);
        return r;
    }

    static GrIRect16 SK_WARN_UNUSED_RESULT MakeXYWH(int16_t x, int16_t y, int16_t w, int16_t h) {
        GrIRect16 r;
        r.set(x, y, x + w, y + h);
        return r;
    }

    static GrIRect16 SK_WARN_UNUSED_RESULT Make(const SkIRect& ir) {
        GrIRect16 r;
        r.set(ir);
        return r;
    }

    int width() const { return fRight - fLeft; }
    int height() const { return fBottom - fTop; }
    int area() const { return this->width() * this->height(); }
    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }

    void setEmpty() { memset(this, 0, sizeof(*this)); }

    void set(int16_t left, int16_t top, int16_t right, int16_t bottom) {
        fLeft = left;
        fTop = top;
        fRight = right;
        fBottom = bottom;
    }

    void set(const SkIRect& r) {
        fLeft   = SkToS16(r.fLeft);
        fTop    = SkToS16(r.fTop);
        fRight  = SkToS16(r.fRight);
        fBottom = SkToS16(r.fBottom);
    }

    void offset(int16_t dx, int16_t dy) {
        fLeft   += dx;
        fTop    += dy;
        fRight  += dx;
        fBottom += dy;
    }
};

/** Returns true if the rectangles have a nonzero area of overlap. It assumed that rects can be
    infinitely small but not "inverted". */
static inline bool GrRectsOverlap(const SkRect& a, const SkRect& b) {
    // See skbug.com/6607 about the isFinite() checks.
    SkASSERT(!a.isFinite() || (a.fLeft <= a.fRight && a.fTop <= a.fBottom));
    SkASSERT(!b.isFinite() || (b.fLeft <= b.fRight && b.fTop <= b.fBottom));
    return a.fRight > b.fLeft && a.fBottom > b.fTop && b.fRight > a.fLeft && b.fBottom > a.fTop;
}

/** Returns true if the rectangles overlap or share an edge or corner. It assumed that rects can be
    infinitely small but not "inverted". */
static inline bool GrRectsTouchOrOverlap(const SkRect& a, const SkRect& b) {
    // See skbug.com/6607 about the isFinite() checks.
    SkASSERT(!a.isFinite() || (a.fLeft <= a.fRight && a.fTop <= a.fBottom));
    SkASSERT(!b.isFinite() || (b.fLeft <= b.fRight && b.fTop <= b.fBottom));
    return a.fRight >= b.fLeft && a.fBottom >= b.fTop && b.fRight >= a.fLeft && b.fBottom >= a.fTop;
}

/**
 * Apply the transform from 'inRect' to 'outRect' to each point in 'inPts', storing the mapped point
 * into the parallel index of 'outPts'.
 */
static inline void GrMapRectPoints(const SkRect& inRect, const SkRect& outRect,
                                   const SkPoint inPts[], SkPoint outPts[], int ptCount) {
    SkMatrix::RectToRect(inRect, outRect).mapPoints(outPts, inPts, ptCount);
}

/**
 * Clips the srcRect and the dstRect to the bounds of the srcSize and dstSize respectively. Returns
 * true if the clipped src and dst rects remain non-empty. The clipped rects preserve the aspect
 * ratio of the original src and dst rects even if one or both extended outside the src and dst
 * dimensions. This means that src and dst rects that have the same dimensions will still have the
 * same (but possibly clipped) width and heights as each other.
 *
 * The clipped values are returned in clippedSrcRect and clippedDstPoint. Returns false if the src
 * rect or dst rect are entirely clipped out of their dimensions.
 */
static inline bool GrClipSrcRectAndDstRect(const SkISize& dstSize,
                                            const SkISize& srcSize,
                                            const SkIRect& srcRect,
                                            const SkIRect& dstRect,
                                            SkIRect* clippedSrcRect,
                                            SkIRect* clippedDstRect) {
    if (srcRect.isEmpty() || dstRect.isEmpty()) {
        return false;
    }

    SkRect srcF = SkRect::Make(srcRect);
    SkRect dstF = SkRect::Make(dstRect);

    float sx = dstF.width() / srcF.width();
    float sy = dstF.height() / srcF.height();

    // clip the src edges to src dimensions, adjusting dst accordingly
    if (srcF.fLeft < 0.f) {
        dstF.fLeft -= srcF.fLeft * sx;
        srcF.fLeft = 0.f;
    }
    if (srcF.fTop < 0.f) {
        dstF.fTop -= srcF.fTop * sy;
        srcF.fTop = 0.f;
    }
    if (srcF.fRight > srcSize.width()) {
        dstF.fRight -= (srcF.fRight - srcSize.width()) * sx;
        srcF.fRight = srcSize.width();
    }
    if (srcF.fBottom > srcSize.height()) {
        dstF.fBottom -= (srcF.fBottom - srcSize.height()) * sy;
        srcF.fBottom = srcSize.height();
    }

    // clip the adjusted dst edges to dst dimensions, adjusting src accordingly
    sx = 1.f / sx;
    sy = 1.f / sy;
    if (dstF.fLeft < 0.f) {
        srcF.fLeft -= dstF.fLeft * sx;
        dstF.fLeft = 0.f;
    }
    if (dstF.fTop < 0.f) {
        srcF.fTop -= dstF.fTop * sy;
        dstF.fTop = 0.f;
    }
    if (dstF.fRight > dstSize.width()) {
        srcF.fRight -= (dstF.fRight - dstSize.width()) * sx;
        dstF.fRight = dstSize.width();
    }
    if (dstF.fBottom > dstSize.height()) {
        srcF.fBottom -= (dstF.fBottom - dstSize.height()) * sy;
        dstF.fBottom = dstSize.height();
    }

    // The above clipping steps may have inverted the rect if it didn't intersect either the src or
    // dst bounds.
    if (srcF.isEmpty() || dstF.isEmpty()) {
        return false;
    }

    *clippedSrcRect = srcF.roundOut();
    *clippedDstRect = dstF.roundOut();

    // before rounding, the clipped src and dst should match the original aspect ratio
    SkASSERT(SkScalarNearlyEqual(srcF.width() / dstF.width(), sx));
    SkASSERT(SkScalarNearlyEqual(srcF.height() / dstF.height(), sy));
    // after rounding, they should still be within the target dimensions
    SkASSERT(SkIRect::MakeSize(srcSize).contains(*clippedSrcRect));
    SkASSERT(SkIRect::MakeSize(dstSize).contains(*clippedDstRect));
    return true;
}
#endif
