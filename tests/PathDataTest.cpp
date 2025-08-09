/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPathData.h"

#include "tests/Test.h"

namespace {
template <typename T> bool spaneq(SkSpan<T> a, SkSpan<T> b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}
}

#if 0
static sk_sp<SkPathData> Oval(const SkRect&,
                              SkPathDirection = SkPathDirection::kCW,
                              unsigned startIndex = 0);
static sk_sp<SkPathData> RRect(const SkRRect&,
                               SkPathDirection dir = SkPathDirection::kCW,
                               unsigned startIndex = 0);
static sk_sp<SkPathData> Arc(const SkArc&);
#endif


DEF_TEST(pathdata_empty, reporter) {
    auto pdata = SkPathData::Empty();

    REPORTER_ASSERT(reporter, pdata->empty());
    REPORTER_ASSERT(reporter, pdata->points().empty());
    REPORTER_ASSERT(reporter, pdata->conics().empty());
    REPORTER_ASSERT(reporter, pdata->verbs().empty());

    REPORTER_ASSERT(reporter, pdata->bounds() == SkRect::MakeEmpty());
    REPORTER_ASSERT(reporter, pdata->getSegmentMask() == 0);

    REPORTER_ASSERT(reporter, !pdata->asLine());
    REPORTER_ASSERT(reporter, !pdata->asOval());
    REPORTER_ASSERT(reporter, !pdata->asRect());
    REPORTER_ASSERT(reporter, !pdata->asRRect());

    auto xformed = pdata->transform(SkMatrix::Scale(2, 3));
    REPORTER_ASSERT(reporter, *pdata == *xformed);
}

const SkPathVerb g3LineRectVerbs[] = {
    SkPathVerb::kMove,
    SkPathVerb::kLine, SkPathVerb::kLine, SkPathVerb::kLine,
    SkPathVerb::kClose,
};
const SkPathVerb g4LineRectVerbs[] = {
    SkPathVerb::kMove,
    SkPathVerb::kLine, SkPathVerb::kLine, SkPathVerb::kLine, SkPathVerb::kLine,
    SkPathVerb::kClose,
};

/*
 *  Differe ways to "make" a rectangular PathData
 */
static sk_sp<SkPathData> factory_maker(const SkRect& r, SkPathDirection d) {
    return SkPathData::Rect(r, d);
}
static sk_sp<SkPathData> manual3_maker(const SkRect& r, SkPathDirection d) {
    std::array<SkPoint, 4> pts = r.toQuad(d);
    return SkPathData::Make(pts, g3LineRectVerbs, {});
}
static sk_sp<SkPathData> manual4_maker(const SkRect& r, SkPathDirection d) {
    std::array<SkPoint, 5> pts;
    r.copyToQuad(pts, d);
    pts[4] = pts[0];    // explicly add the closing line
    return SkPathData::Make(pts, g4LineRectVerbs, {});
}

DEF_TEST(pathdata_rect, reporter) {
    const SkRect r = {1, 2, 3, 4};

    auto sign = [](float x) -> float {
        if (x == 0) {
            return 0;
        } else {
            return x > 0 ? 1 : -1;
        }
    };

    for (auto maker : {factory_maker, manual3_maker, manual4_maker}) {
        for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
            auto pdata = maker(r, dir);
            REPORTER_ASSERT(reporter, r == pdata->bounds());

            // 1. manually determine if *we* think it is a rect

            const SkSpan<const SkPoint> pts = pdata->points();
            REPORTER_ASSERT(reporter, r == SkRect::Bounds(pts).value());

            const float crossSign = (dir == SkPathDirection::kCW) ? 1 : -1;
            for (int i = 1; i < 3; ++i) {
                SkVector u = pts[i]   - pts[i-1],
                         v = pts[i+1] - pts[i];
                const float cross = u.cross(v),
                            dot   = u.dot(v);
                REPORTER_ASSERT(reporter, dot == 0);
                REPORTER_ASSERT(reporter, crossSign == sign(cross));
            }

            // 2. now ask the pathdata

            auto isa = pdata->asRect();
            REPORTER_ASSERT(reporter, isa.has_value());
            REPORTER_ASSERT(reporter, isa->fRect == r);
            REPORTER_ASSERT(reporter, isa->fDirection == dir);
            REPORTER_ASSERT(reporter, isa->fStartIndex == 0);
        }
    }
}

DEF_TEST(pathdata_polygon, reporter) {
    const SkPoint points[] = {
        {0, 1}, {2, 3}, {4, 5}, {6, 7},
    };

    for (auto isClosed : {false, true}) {
        for (size_t n = 0; n <= std::size(points); ++n) {
            const SkSpan<const SkPoint> pts = {points, n};
            auto pdata = SkPathData::Polygon(pts, isClosed);

            REPORTER_ASSERT(reporter, spaneq(pdata->points(), pts));
            REPORTER_ASSERT(reporter, pdata->conics().empty());

            if (n == 0) {
                REPORTER_ASSERT(reporter, pdata->empty());
                continue;
            }

            auto line = pdata->asLine();
            if (n == 2 && !isClosed) {
                REPORTER_ASSERT(reporter, line.has_value());
                REPORTER_ASSERT(reporter, line.value()[0] == points[0]);
                REPORTER_ASSERT(reporter, line.value()[1] == points[1]);

                auto pline = SkPathData::Line(points[0], points[1]);
                REPORTER_ASSERT(reporter, *pline == *pdata);
            } else {
                REPORTER_ASSERT(reporter, !line.has_value());
            }

            const size_t expectedVerbs = pts.size() + isClosed;
            auto vbs = pdata->verbs();
            REPORTER_ASSERT(reporter, vbs.size() == expectedVerbs);

            REPORTER_ASSERT(reporter, vbs[0] == SkPathVerb::kMove);
            for (size_t i = 1; i < pts.size(); ++i) {
                REPORTER_ASSERT(reporter, vbs[i] == SkPathVerb::kLine);
            }
            if (isClosed) {
                REPORTER_ASSERT(reporter, vbs.back() == SkPathVerb::kClose);
            }
        }
    }
}
