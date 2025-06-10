/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathRaw.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"

#include "tests/Test.h"

class SkSPathRawBuilder {
public:
    SkSPathRawBuilder(SkSpan<SkPoint> ptStore, SkSpan<uint8_t> vbStore, SkSpan<SkScalar> cnStore)
        : fPtStorage(ptStore)
        , fVbStorage(vbStore)
        , fCnStorage(cnStore)
        , fPts(0), fCns(0), fVbs(0)
    {}

    void moveTo(SkPoint);
    void lineTo(SkPoint);
    void quadTo(SkPoint, SkPoint);
    void conicTo(SkPoint, SkPoint, SkScalar w);
    void cubicTo(SkPoint, SkPoint, SkPoint);
    void close();

    SkPathRaw raw(SkPathFillType, bool isConvex = false) const;

private:
    SkSpan<SkPoint>  fPtStorage;
    SkSpan<uint8_t>  fVbStorage;
    SkSpan<SkScalar> fCnStorage;
    size_t fPts, fCns, fVbs;

    void check_extend_pts(size_t n) const {
        SkASSERT(fPts + n <= fPtStorage.size());
    }
    void check_extend_vbs(size_t n) const {
        SkASSERT(fVbs + n <= fVbStorage.size());
    }
    void check_extend_cns(size_t n) const {
        SkASSERT(fCns + n <= fCnStorage.size());
    }
};

void SkSPathRawBuilder::moveTo(SkPoint p) {
    check_extend_pts(1);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p;
    fVbStorage[fVbs++] = (uint8_t)SkPathVerb::kMove;
}

void SkSPathRawBuilder::lineTo(SkPoint p) {
    check_extend_pts(1);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p;
    fVbStorage[fVbs++] = (uint8_t)SkPathVerb::kLine;
}

void SkSPathRawBuilder::quadTo(SkPoint p1, SkPoint p2) {
    check_extend_pts(2);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p1;
    fPtStorage[fPts++] = p2;
    fVbStorage[fVbs++] = (uint8_t)SkPathVerb::kQuad;
}

void SkSPathRawBuilder::conicTo(SkPoint p1, SkPoint p2, SkScalar w) {
    check_extend_pts(2);
    check_extend_cns(1);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p1;
    fPtStorage[fPts++] = p2;
    fCnStorage[fCns++] = w;
    fVbStorage[fVbs++] = (uint8_t)SkPathVerb::kConic;
}

void SkSPathRawBuilder::cubicTo(SkPoint p1, SkPoint p2, SkPoint p3) {
    check_extend_pts(3);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p1;
    fPtStorage[fPts++] = p2;
    fPtStorage[fPts++] = p3;
    fVbStorage[fVbs++] = (uint8_t)SkPathVerb::kCubic;
}

void SkSPathRawBuilder::close() {
    check_extend_vbs(1);
    fVbStorage[fVbs++] = (uint8_t)SkPathVerb::kClose;
}

SkPathRaw SkSPathRawBuilder::raw(SkPathFillType ft, bool isConvex) const {
    const auto ptSpan = fPtStorage.first(fPts);
    return {
        ptSpan,
        fVbStorage.first(fVbs),
        fCnStorage.first(fCns),
        SkRect::BoundsOrEmpty(ptSpan),
        ft,
        isConvex,
    };
}

DEF_TEST(pathraw_iter, reporter) {
    SkPoint pts[11];
    uint8_t vbs[8];
    SkScalar cns[1];

    constexpr size_t N = 11;
    SkPoint p[N];
    for (size_t i = 0; i < N; ++i) {
        p[i] = {SkScalar(i), SkScalar(i)};
    }

    SkSPathRawBuilder bu(pts, vbs, cns);

    bu.moveTo(p[0]);
    bu.lineTo(p[1]);
    bu.quadTo(p[2], p[3]);
    bu.cubicTo(p[4], p[5], p[6]);
    bu.close();

    auto raw = bu.raw(SkPathFillType::kWinding);

    REPORTER_ASSERT(reporter, raw.fPoints.size() == 7);
    REPORTER_ASSERT(reporter, raw.fVerbs.size() == 5);
    REPORTER_ASSERT(reporter, raw.fConics.size() == 0);

    bu.moveTo(p[7]);
    bu.lineTo(p[8]);
    bu.conicTo(p[9], p[10], 2);

    raw = bu.raw(SkPathFillType::kWinding);

    REPORTER_ASSERT(reporter, raw.fPoints.size() == N);
    REPORTER_ASSERT(reporter, raw.fVerbs.size() == 8);
    REPORTER_ASSERT(reporter, raw.fConics.size() == 1);

    // Now test the iterator

    auto iter = raw.iter();
    auto r = iter.next();
    REPORTER_ASSERT(reporter, r);
    REPORTER_ASSERT(reporter, r->vrb == SkPathVerb::kMove);
    REPORTER_ASSERT(reporter, r->pts.size() == 1);
    REPORTER_ASSERT(reporter, r->pts[0] == p[0]);

    r = iter.next();
    REPORTER_ASSERT(reporter, r);
    REPORTER_ASSERT(reporter, r->vrb == SkPathVerb::kLine);
    REPORTER_ASSERT(reporter, r->pts.size() == 2);
    for (int i = 0; i < 2; ++i) {
        REPORTER_ASSERT(reporter, r->pts[i] == p[i + 0]);
    }

    r = iter.next();
    REPORTER_ASSERT(reporter, r);
    REPORTER_ASSERT(reporter, r->vrb == SkPathVerb::kQuad);
    REPORTER_ASSERT(reporter, r->pts.size() == 3);
    for (int i = 0; i < 3; ++i) {
        REPORTER_ASSERT(reporter, r->pts[i] == p[i + 1]);
    }

    r = iter.next();
    REPORTER_ASSERT(reporter, r);
    REPORTER_ASSERT(reporter, r->vrb == SkPathVerb::kCubic);
    REPORTER_ASSERT(reporter, r->pts.size() == 4);
    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, r->pts[i] == p[i + 3]);
    }

    r = iter.next();
    REPORTER_ASSERT(reporter, r);
    REPORTER_ASSERT(reporter, r->vrb == SkPathVerb::kClose);
    REPORTER_ASSERT(reporter, r->pts.size() == 2);
    REPORTER_ASSERT(reporter, r->pts[0] == p[6]);
    REPORTER_ASSERT(reporter, r->pts[1] == p[0]);

    r = iter.next();
    REPORTER_ASSERT(reporter, r);
    REPORTER_ASSERT(reporter, r->vrb == SkPathVerb::kMove);
    REPORTER_ASSERT(reporter, r->pts[0] == p[7]);

    r = iter.next();
    REPORTER_ASSERT(reporter, r);
    REPORTER_ASSERT(reporter, r->vrb == SkPathVerb::kLine);
    for (int i = 0; i < 2; ++i) {
        REPORTER_ASSERT(reporter, r->pts[i] == p[i + 7]);
    }
    r = iter.next();
    REPORTER_ASSERT(reporter, r);
    REPORTER_ASSERT(reporter, r->vrb == SkPathVerb::kConic);
    REPORTER_ASSERT(reporter, r->w == 2);
    REPORTER_ASSERT(reporter, r->pts.size() == 3);
    for (int i = 0; i < 3; ++i) {
        REPORTER_ASSERT(reporter, r->pts[i] == p[i + 8]);
    }

    // we should be done now

    r = iter.next();
    REPORTER_ASSERT(reporter, !r);
    r = iter.next();
    REPORTER_ASSERT(reporter, !r);
}
