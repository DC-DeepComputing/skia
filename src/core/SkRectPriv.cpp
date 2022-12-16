#include "src/core/SkRectPriv.h"

template<typename R>
static bool subtract(const R& a, const R& b, R* out) {
    if (a.isEmpty() || b.isEmpty() || !R::Intersects(a, b)) {
        // Either already empty, or subtracting the empty rect, or there's no intersection, so
        // in all cases the answer is A.
        *out = a;
        return true;
    }

    // 4 rectangles to consider. If the edge in A is contained in B, the resulting difference can
    // be represented exactly as a rectangle. Otherwise the difference is the largest subrectangle
    // that is disjoint from B:
    // 1. Left part of A:   (A.left,  A.top,    B.left,  A.bottom)
    // 2. Right part of A:  (B.right, A.top,    A.right, A.bottom)
    // 3. Top part of A:    (A.left,  A.top,    A.right, B.top)
    // 4. Bottom part of A: (A.left,  B.bottom, A.right, A.bottom)
    //
    // Depending on how B intersects A, there will be 1 to 4 positive areas:
    //  - 4 occur when A contains B
    //  - 3 occur when B intersects a single edge
    //  - 2 occur when B intersects at a corner, or spans two opposing edges
    //  - 1 occurs when B spans two opposing edges and contains a 3rd, resulting in an exact rect
    //  - 0 occurs when B contains A, resulting in the empty rect
    //
    // Compute the relative areas of the 4 rects described above. Since each subrectangle shares
    // either the width or height of A, we only have to divide by the other dimension, which avoids
    // overflow on int32 types, and even if the float relative areas overflow to infinity, the
    // comparisons work out correctly and (one of) the infinitely large subrects will be chosen.
    float aHeight = (float) a.height();
    float aWidth = (float) a.width();
    float leftArea = 0.f, rightArea = 0.f, topArea = 0.f, bottomArea = 0.f;
    int positiveCount = 0;
    if (b.fLeft > a.fLeft) {
        leftArea = (b.fLeft - a.fLeft) / aWidth;
        positiveCount++;
    }
    if (a.fRight > b.fRight) {
        rightArea = (a.fRight - b.fRight) / aWidth;
        positiveCount++;
    }
    if (b.fTop > a.fTop) {
        topArea = (b.fTop - a.fTop) / aHeight;
        positiveCount++;
    }
    if (a.fBottom > b.fBottom) {
        bottomArea = (a.fBottom - b.fBottom) / aHeight;
        positiveCount++;
    }

    if (positiveCount == 0) {
        SkASSERT(b.contains(a));
        *out = R::MakeEmpty();
        return true;
    }

    *out = a;
    if (leftArea > rightArea && leftArea > topArea && leftArea > bottomArea) {
        // Left chunk of A, so the new right edge is B's left edge
        out->fRight = b.fLeft;
    } else if (rightArea > topArea && rightArea > bottomArea) {
        // Right chunk of A, so the new left edge is B's right edge
        out->fLeft = b.fRight;
    } else if (topArea > bottomArea) {
        // Top chunk of A, so the new bottom edge is B's top edge
        out->fBottom = b.fTop;
    } else {
        // Bottom chunk of A, so the new top edge is B's bottom edge
        SkASSERT(bottomArea > 0.f);
        out->fTop = b.fBottom;
    }

    // If we have 1 valid area, the disjoint shape is representable as a rectangle.
    SkASSERT(!R::Intersects(*out, b));
    return positiveCount == 1;
}

bool SkRectPriv::Subtract(const SkRect& a, const SkRect& b, SkRect* out) {
    return subtract<SkRect>(a, b, out);
}

bool SkRectPriv::Subtract(const SkIRect& a, const SkIRect& b, SkIRect* out) {
    return subtract<SkIRect>(a, b, out);
}
