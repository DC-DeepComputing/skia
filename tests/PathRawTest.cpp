/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <array>
#include <cstdint>
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"

#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/base/SkRandom.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRaw.h"

#include "tests/Test.h"

class SkSPathRawBuilder {
public:
    SkSPathRawBuilder(SkSpan<SkPoint> ptStore, SkSpan<SkPathVerb> vbStore, SkSpan<float> cnStore)
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
    SkSpan<SkPoint>    fPtStorage;
    SkSpan<SkPathVerb> fVbStorage;
    SkSpan<SkScalar>   fCnStorage;
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
    fVbStorage[fVbs++] = SkPathVerb::kMove;
}

void SkSPathRawBuilder::lineTo(SkPoint p) {
    check_extend_pts(1);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p;
    fVbStorage[fVbs++] = SkPathVerb::kLine;
}

void SkSPathRawBuilder::quadTo(SkPoint p1, SkPoint p2) {
    check_extend_pts(2);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p1;
    fPtStorage[fPts++] = p2;
    fVbStorage[fVbs++] = SkPathVerb::kQuad;
}

void SkSPathRawBuilder::conicTo(SkPoint p1, SkPoint p2, SkScalar w) {
    check_extend_pts(2);
    check_extend_cns(1);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p1;
    fPtStorage[fPts++] = p2;
    fCnStorage[fCns++] = w;
    fVbStorage[fVbs++] = SkPathVerb::kConic;
}

void SkSPathRawBuilder::cubicTo(SkPoint p1, SkPoint p2, SkPoint p3) {
    check_extend_pts(3);
    check_extend_vbs(1);
    fPtStorage[fPts++] = p1;
    fPtStorage[fPts++] = p2;
    fPtStorage[fPts++] = p3;
    fVbStorage[fVbs++] = SkPathVerb::kCubic;
}

void SkSPathRawBuilder::close() {
    check_extend_vbs(1);
    fVbStorage[fVbs++] = SkPathVerb::kClose;
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

static void check_iter(skiatest::Reporter* reporter, SkPathRaw::Iter iter,
                       SkSpan<SkPoint> pts, SkSpan<const SkPathVerb> vbs, SkSpan<const float> cns) {
    size_t pIndex = 0, vIndex = 0, cIndex = 0;
    size_t moveToIndex = 0; // track the start of each contour

    while (auto r = iter.next()) {
        REPORTER_ASSERT(reporter, vIndex < vbs.size());
        REPORTER_ASSERT(reporter, vbs[vIndex++] == r->vrb);
        switch (r->vrb) {
            case SkPathVerb::kMove:
                moveToIndex = pIndex;
                REPORTER_ASSERT(reporter, pIndex < pts.size());
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->pts[0]);
                break;
            case SkPathVerb::kLine:
                REPORTER_ASSERT(reporter, pIndex < pts.size());
                REPORTER_ASSERT(reporter, pts[pIndex-1] == r->pts[0]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->pts[1]);
                break;
            case SkPathVerb::kQuad:
                REPORTER_ASSERT(reporter, pIndex+1 < pts.size());
                REPORTER_ASSERT(reporter, pts[pIndex-1] == r->pts[0]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->pts[1]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->pts[2]);
                break;
            case SkPathVerb::kConic:
                REPORTER_ASSERT(reporter, pIndex+1 < pts.size());
                REPORTER_ASSERT(reporter, pts[pIndex-1] == r->pts[0]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->pts[1]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->pts[2]);
                REPORTER_ASSERT(reporter, cIndex < cns.size());
                REPORTER_ASSERT(reporter, cns[cIndex++] == r->w);
                break;
            case SkPathVerb::kCubic:
                REPORTER_ASSERT(reporter, pIndex+2 < pts.size());
                REPORTER_ASSERT(reporter, pts[pIndex-1] == r->pts[0]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->pts[1]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->pts[2]);
                REPORTER_ASSERT(reporter, pts[pIndex++] == r->pts[3]);
                break;
            case SkPathVerb::kClose:
                REPORTER_ASSERT(reporter, pts[pIndex-1]    == r->pts[0]);   // last  pt in contour
                REPORTER_ASSERT(reporter, pts[moveToIndex] == r->pts[1]);   // first pt in contour
                break;
        }
    }

    // make sure the iter is really done
    REPORTER_ASSERT(reporter, !iter.next());
}

DEF_TEST(pathraw_iter, reporter) {
    SkPoint pts[11];
    SkPathVerb vbs[8];
    float cns[1];

    constexpr size_t N = 11;
    SkPoint p[N];
    for (size_t i = 0; i < N; ++i) {
        p[i] = {SkScalar(i), SkScalar(i)};
    }

    SkSPathRawBuilder bu(pts, vbs, cns);

    const SkPathVerb verbs[] = {
        SkPathVerb::kMove,
        SkPathVerb::kLine,
        SkPathVerb::kQuad,
        SkPathVerb::kCubic,
        SkPathVerb::kClose,
        SkPathVerb::kMove,
        SkPathVerb::kLine,
        SkPathVerb::kConic,
    };

    bu.moveTo(p[0]);
    bu.lineTo(p[1]);
    bu.quadTo(p[2], p[3]);
    bu.cubicTo(p[4], p[5], p[6]);
    bu.close();
    bu.moveTo(p[7]);
    bu.lineTo(p[8]);
    bu.conicTo(p[9], p[10], 2);

    auto raw = bu.raw(SkPathFillType::kWinding);

    REPORTER_ASSERT(reporter, raw.fPoints.size() == N);
    REPORTER_ASSERT(reporter, raw.fVerbs.size() == 8);
    REPORTER_ASSERT(reporter, raw.fConics.size() == 1);

    check_iter(reporter, raw, p, verbs, cns);

    // now make sure pathbuilder generates the same results

    SkPathBuilder pb;

    pb.moveTo(p[0]);
    pb.lineTo(p[1]);
    pb.quadTo(p[2], p[3]);
    pb.cubicTo(p[4], p[5], p[6]);
    pb.close();
    pb.moveTo(p[7]);
    pb.lineTo(p[8]);
    pb.conicTo(p[9], p[10], 2);

    auto path = pb.detach();
    raw = SkPathPriv::Raw(path);

    check_iter(reporter, raw, p, verbs, cns);
}

DEF_TEST(pathraw_stack, reporter) {
    auto check = [&reporter](const SkPathRaw& path,
                             SkPathFillType expected_filltype,
                             SkSpan<const SkPathVerb> expected_verbs,
                             SkSpan<const SkPoint> expected_points,
                             SkSpan<const float> expected_weights) {
        REPORTER_ASSERT(reporter, path.fFillType == expected_filltype);
        REPORTER_ASSERT(reporter, path.fVerbs.size() == expected_verbs.size());
        for (size_t i = 0; i < path.fVerbs.size(); ++i) {
            REPORTER_ASSERT(reporter, path.fVerbs[i] == expected_verbs[i]);
        }
        if (path.fPoints.size() != expected_points.size()) {
            fprintf(stderr, "*** pt size: %zu, expected: %zu\n",
                    path.fPoints.size(),
                    expected_points.size());
        }
        REPORTER_ASSERT(reporter, path.fPoints.size() == expected_points.size());
        for (size_t i = 0; i < path.fPoints.size(); ++i) {
            REPORTER_ASSERT(reporter, path.fPoints[i] == expected_points[i]);
        }
        REPORTER_ASSERT(reporter, path.fConics.size() == expected_weights.size());
        for (size_t i = 0; i < path.fConics.size(); ++i) {
            REPORTER_ASSERT(reporter, path.fConics[i] == expected_weights[i]);
        }

        SkRect expected_bounds;
        expected_bounds.setBoundsCheck(expected_points);
        REPORTER_ASSERT(reporter, path.fBounds == expected_bounds);
    };

    {
        static constexpr auto path = SkSTPathRaw(
            SkPathFillType::kEvenOdd,
            MoveTo({1, 2}),
            LineTo({3, 4}),
            QuadTo({5, 6}, {7, 8}),
            CubicTo({9, 10}, {11, 12}, {13, 14}),
            ConicTo({15, 16}, {17, 18}, 1.2f),
            Close());

        // static_assert(decltype(path)::VerbCount()  == 6);
        // static_assert(decltype(path)::PointCount() == 9);
        // static_assert(decltype(path)::ConicCount() == 1);

        check(path,
              SkPathFillType::kEvenOdd,
              {
                SkPathVerb::kMove,
                SkPathVerb::kLine,
                SkPathVerb::kQuad,
                SkPathVerb::kCubic,
                SkPathVerb::kConic,
                SkPathVerb::kClose
              },
              { {1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 10}, {11, 12}, {13, 14}, {15, 16}, {17, 18} },
              { 1.2f }
        );
    }

    {
        SkRandom rand;
        std::array<SkPoint, 9> pts;
        for (auto& pt : pts) { pt = {rand.nextF(), rand.nextF()}; }

        const SkSTPathRaw path(
            SkPathFillType::kEvenOdd,
            MoveTo(pts[0]),
            LineTo(pts[1]),
            QuadTo(pts[2], pts[3]),
            CubicTo(pts[4], pts[5], pts[6]),
            ConicTo(pts[7], pts[8], 1.4f),
            Close()
        );

        // static_assert(decltype(path)::VerbCount()  == 6);
        // static_assert(decltype(path)::PointCount() == 9);
        // static_assert(decltype(path)::ConicCount() == 1);

        check(path,
              SkPathFillType::kEvenOdd,
              {
                SkPathVerb::kMove,
                SkPathVerb::kLine,
                SkPathVerb::kQuad,
                SkPathVerb::kCubic,
                SkPathVerb::kConic,
                SkPathVerb::kClose
              },
              pts,
              { 1.4f }
        );
    }
}
