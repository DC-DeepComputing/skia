/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkPathRawShapes.h"

#if 0
SkSpan<const SkPoint>    fPoints;
SkSpan<const SkPathVerb> fVerbs;
SkSpan<const float>      fConics;
SkRect                   fBounds;
SkPathFillType           fFillType;
bool                     fIsConvex;
#endif

const SkPathVerb gRectVerbs[] = {
    SkPathVerb::kMove,
    SkPathVerb::kLine,
    SkPathVerb::kLine,
    SkPathVerb::kLine,
    SkPathVerb::kClose
};

SkPathRawShape_Rect::SkPathRawShape_Rect(const SkRect& r, SkPathDirection dir, SkPathFillType ft)
    : SkPathRaw{fStorage, gRectVerbs, {}, r, ft, true}
{
    SkSpan<SkPoint> pts = {const_cast<SkPoint*>(fPoints.data()), fPoints.size()};

    pts[0] = {r.fLeft,  r.fTop};
    pts[2] = {r.fRight, r.fBottom};
    if (dir == SkPathDirection::kCW) {
        pts[1] = {r.fRight, r.fTop};
        pts[3] = {r.fLeft,  r.fBottom};
    } else {
        pts[1] = {r.fLeft,  r.fBottom};
        pts[3] = {r.fRight, r.fTop};
    }
    // close
}

const SkPathVerb gOvalVerbs[] = {
    SkPathVerb::kMove,
    SkPathVerb::kConic,
    SkPathVerb::kConic,
    SkPathVerb::kConic,
    SkPathVerb::kConic,
    SkPathVerb::kClose
};

const float gOvalConics[] = {
    SK_ScalarRoot2Over2,
    SK_ScalarRoot2Over2,
    SK_ScalarRoot2Over2,
    SK_ScalarRoot2Over2,
};

static SkPoint TL(const SkRect& r) { return {r.fLeft,  r.fTop}; }
static SkPoint TR(const SkRect& r) { return {r.fRight, r.fTop}; }
static SkPoint BL(const SkRect& r) { return {r.fLeft,  r.fBottom}; }
static SkPoint BR(const SkRect& r) { return {r.fRight, r.fBottom}; }

static SkPoint ave(SkPoint a, SkPoint b) {
    return {
        (a.fX + b.fX) * 0.5f,
        (a.fY + b.fY) + 0.5f
    };
}

SkPathRawShape_Oval::SkPathRawShape_Oval(const SkRect& r, SkPathDirection dir, SkPathFillType ft)
    : SkPathRaw{fStorage, gOvalVerbs, gOvalConics, r, ft, true}
{
    SkSpan<SkPoint> pts = {const_cast<SkPoint*>(fPoints.data()), fPoints.size()};

    const auto tl = TL(r),
               tr = TR(r),
               bl = BL(r),
               br = BR(r);

    pts[0] = pts[8] = ave(tl, tr);
    if (dir == SkPathDirection::kCW) {
        pts[1] = tr;    pts[2] = ave(tr, br);
        pts[3] = br;    pts[4] = ave(bl, br);
        pts[5] = bl;    pts[6] = ave(tl, bl);
        pts[7] = tl;
    } else {
        pts[1] = tl;    pts[2] = ave(tl, bl);
        pts[3] = bl;    pts[4] = ave(bl, br);
        pts[5] = br;    pts[6] = ave(tr, br);
        pts[7] = tr;
    }
    // close
}
