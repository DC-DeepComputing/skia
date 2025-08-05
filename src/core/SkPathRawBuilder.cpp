/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRawBuilder.h"

void SkPathRawBuilder::moveTo(SkPoint p) {
    check_extend_pts(1);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p;
    fVbStorage[fVbs++] = SkPathVerb::kMove;
}

void SkPathRawBuilder::lineTo(SkPoint p) {
    check_extend_pts(1);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p;
    fVbStorage[fVbs++] = SkPathVerb::kLine;
}

void SkPathRawBuilder::quadTo(SkPoint p1, SkPoint p2) {
    check_extend_pts(2);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p1;
    fPtStorage[fPts++] = p2;
    fVbStorage[fVbs++] = SkPathVerb::kQuad;
}

void SkPathRawBuilder::conicTo(SkPoint p1, SkPoint p2, SkScalar w) {
    check_extend_pts(2);
    check_extend_cns(1);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p1;
    fPtStorage[fPts++] = p2;
    fCnStorage[fCns++] = w;
    fVbStorage[fVbs++] = SkPathVerb::kConic;
}

void SkPathRawBuilder::cubicTo(SkPoint p1, SkPoint p2, SkPoint p3) {
    check_extend_pts(3);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p1;
    fPtStorage[fPts++] = p2;
    fPtStorage[fPts++] = p3;
    fVbStorage[fVbs++] = SkPathVerb::kCubic;
}

void SkPathRawBuilder::close() {
    check_extend_vbs(1);
    fVbStorage[fVbs++] = SkPathVerb::kClose;
}

SkPathRaw SkPathRawBuilder::raw(SkPathFillType ft, bool isConvex) const {
    const auto ptSpan = fPtStorage.first(fPts);
    const auto vbSpan = fVbStorage.first(fVbs);
    return {
        ptSpan,
        vbSpan,
        fCnStorage.first(fCns),
        SkRect::BoundsOrEmpty(ptSpan),
        ft,
        isConvex,
        SkPathPriv::ComputeSegmentMask(vbSpan),
    };
}
