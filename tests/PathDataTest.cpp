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
static sk_sp<SkPathData> Rect(const SkRect&,
                              SkPathDirection = SkPathDirection::kCW,
                              unsigned startIndex = 0);
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

    REPORTER_ASSERT(reporter, pdata->isEmpty());
    REPORTER_ASSERT(reporter, pdata->points().empty());
    REPORTER_ASSERT(reporter, pdata->conics().empty());
    REPORTER_ASSERT(reporter, pdata->verbs().empty());

    REPORTER_ASSERT(reporter, pdata->bounds() == SkRect::MakeEmpty());
    REPORTER_ASSERT(reporter, pdata->getSegmentMask() == 0);

    REPORTER_ASSERT(reporter, !pdata->asArc());
    REPORTER_ASSERT(reporter, !pdata->asLine());
    REPORTER_ASSERT(reporter, !pdata->asOval());
    REPORTER_ASSERT(reporter, !pdata->asRect());
    REPORTER_ASSERT(reporter, !pdata->asRRect());

    auto xformed = pdata->transform(SkMatrix::Scale(2, 3));
    REPORTER_ASSERT(reporter, *pdata == *xformed);
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
                REPORTER_ASSERT(reporter, pdata->isEmpty());
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
