// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.


#include "modules/bentleyottmann/include/Myers.h"
#include "tests/Test.h"

namespace myers {
extern bool upper_s0_on_s1(const Segment& s0, const Segment& s1);
extern bool slope_s0_less_than_slope_s1(const Segment& s0, const Segment& s1);
extern bool less_than_at_y(const Segment& s0, const Segment& s1, int32_t y);
}  // namespace myers

using namespace myers;

bool operator==(std::pair<const Point &, const Point &> l, std::tuple<Point, Point> r) {
    return std::get<0>(l) == std::get<0>(r) && std::get<1>(l) == std::get<1>(r);
}

DEF_TEST(MFC_order_segment_points, r) {
    {
        Point p0 = {0, 0},
              p1 = {1, 1};
        REPORTER_ASSERT(r, std::minmax(p0, p1) == std::make_tuple(p0, p1));
        REPORTER_ASSERT(r, std::minmax(p1, p0) == std::make_tuple(p0, p1));
    }
    {
        Point p0 = {0, 0},
              p1 = {-1, 1};
        REPORTER_ASSERT(r, std::minmax(p0, p1) == std::make_tuple(p0, p1));
        REPORTER_ASSERT(r, std::minmax(p1, p0) == std::make_tuple(p0, p1));
    }
    {
        Point p0 = {0, 0},
              p1 = {0, 1};
        REPORTER_ASSERT(r, std::minmax(p0, p1) == std::make_tuple(p0, p1));
        REPORTER_ASSERT(r, std::minmax(p1, p0) == std::make_tuple(p0, p1));
    }
}

DEF_TEST(MFC_s0_begins_on_s1, r) {
    {  // On s1.
        Segment s0 = {{0, 0}, {1, 1}},
                s1 = {{1, -1}, {-1, 1}};
        REPORTER_ASSERT(r, upper_s0_on_s1(s0, s1));
    }
    {  // Away from s1
        Segment s0 = {{1, 1}, {2, 2}},
                s1 = {{1, -1}, {-1, 1}};
        REPORTER_ASSERT(r, !upper_s0_on_s1(s0, s1));
    }
    {  // Crosses s1
        Segment s0 = {{-1, -1}, {1, 1}},
                s1 = {{1, -1}, {-1, 1}};
        REPORTER_ASSERT(r, !upper_s0_on_s1(s0, s1));
    }
}

DEF_TEST(MFC_slope_less_than, r) {
    {
        Segment s0 = {{0, 0}, {1, 1}},
                s1 = {{0, 0}, {-1, 1}};
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s0, s1));
        REPORTER_ASSERT(r, slope_s0_less_than_slope_s1(s1, s0));
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s0, s0));
    }
    {
        Segment s = {{0, 0}, {0,1}};
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s, s));
    }
}

DEF_TEST(MFC_less_than_at_y, r) {
    {
        Segment s0 = {{0, 0}, {2, 2}},
                s1 = {{0, 0}, {-2, 2}};
        REPORTER_ASSERT(r, !less_than_at_y(s0, s1, 1));
        REPORTER_ASSERT(r, less_than_at_y(s1, s0, 1));
    }
    {  // cross at 0 use slope to break tie.
        Segment s0 = {{-2, -2}, {2, 2}},
                s1 = {{2, -2}, {-2, 2}};
        REPORTER_ASSERT(r, less_than_at_y(s0, s1, -1));
        REPORTER_ASSERT(r, !less_than_at_y(s1, s0, -1));
        REPORTER_ASSERT(r, !less_than_at_y(s0, s1, 0));
        REPORTER_ASSERT(r, less_than_at_y(s1, s0, 0));
        REPORTER_ASSERT(r, !less_than_at_y(s0, s1, 1));
        REPORTER_ASSERT(r, less_than_at_y(s1, s0, 1));
    }
}

void test_both(skiatest::Reporter* r, SkSpan<Segment> segments) {
    auto myersResponse = myers_find_crossings(segments);
    auto bruteResponse = brute_force_crossings(segments);

    std::sort(myersResponse.begin(), myersResponse.end());
    std::sort(bruteResponse.begin(), bruteResponse.end());

    REPORTER_ASSERT(r, myersResponse.size() == bruteResponse.size());

    // There should be no duplicate crossings.
    REPORTER_ASSERT(r,
                    std::unique(myersResponse.begin(), myersResponse.end()) == myersResponse.end());
    REPORTER_ASSERT(r,
                    std::unique(bruteResponse.begin(), bruteResponse.end()) == bruteResponse.end());
    REPORTER_ASSERT(r, std::equal(myersResponse.begin(), myersResponse.end(),
                                  bruteResponse.begin(), bruteResponse.end()));
}

DEF_TEST(MFC_myers_brute_force_comparison, r) {
 /*   {  // Crossing
        Segment s0 = {{-1, -1}, {1, 1}},
                s1 = {{1, -1}, {-1, 1}};

        Segment segments[] = {s0, s1};

        test_both(r, segments);
    }

    {  // Two vertical lines
        Segment s0 = {{-1, -1}, {-1, 1}},
                s1 = {{1, -1}, {1, 1}};

        Segment segments[] = {s0, s1};

        test_both(r, segments);
    }

    {  // Two horizontal lines.
        Segment s0 = {{-1, -1}, {1, -1}},
                s1 = {{-1, 1}, {1, 1}};

        Segment segments[] = {s0, s1};

        test_both(r, segments);
    }
*/
    {
        Segment s0 = {{-2, 1}, {1, 1}},
                s1 = {{-1, 1}, {2, 1}};

        Segment segments[] = {s0, s1};

        test_both(r, segments);
    }
}
