// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/EventQueue.h"
#include "tests/Test.h"

using namespace bentleyottmann;

DEF_TEST(BO_SegmentBasic, reporter) {
    {
        Segment s = {{0, 0}, {1, 1}};
        REPORTER_ASSERT(reporter, s.upper() == s.p0);
        REPORTER_ASSERT(reporter, s.lower() == s.p1);
    }

    {
        Segment s = {{1, 0}, {0, 1}};
        REPORTER_ASSERT(reporter, s.upper() == s.p0);
        REPORTER_ASSERT(reporter, s.lower() == s.p1);
    }

    {
        Segment s = {{1, 1}, {0, 0}};
        REPORTER_ASSERT(reporter, s.upper() == s.p1);
        REPORTER_ASSERT(reporter, s.lower() == s.p0);
    }

    {
        Segment s = {{0, 1}, {1, 0}};
        REPORTER_ASSERT(reporter, s.upper() == s.p1);
        REPORTER_ASSERT(reporter, s.lower() == s.p0);
    }
}

static Segment swap_ends(const Segment& s) {
    return {s.p1, s.p0};
}

DEF_TEST(BO_no_intersection_bounding_box, reporter) {
    Segment interesting[] = {{Point::Smallest(),  Point::Smallest()+ Point{10, 5}},
                             {Point::Largest(), Point::Largest() - Point{10, 5}},
                             {{-10, -5}, {10, 5}}};

    // Intersection
    for (auto& s0 : interesting) {
        auto [l, t, r, b] = s0.bounds();

        // Points in the interior of interesting rectangles
        for(Point p : {Point {l + 1, t + 1},
                       Point {r - 1, t + 1},
                       Point {r - 1, b - 1},
                       Point {l + 1, b - 1}}) {
            Segment s1 = {p, {0, 0}};
            REPORTER_ASSERT(reporter, !no_intersection_by_bounding_box(s0, s1));
            REPORTER_ASSERT(reporter, !no_intersection_by_bounding_box(s1, s0));
            REPORTER_ASSERT(reporter,
                            !no_intersection_by_bounding_box(swap_ends(s0), swap_ends(s1)));
            REPORTER_ASSERT(reporter,
                            !no_intersection_by_bounding_box(swap_ends(s0), swap_ends(s1)));
        }
    }

    int32_t small = Point::Smallest().x,
            big = Point::Largest().x;

    // No Intersection
    for (auto& s0 : interesting) {
        auto [l, t, r, b] = s0.bounds();
        Segment outside[] = {{{r, t}, {big, b}},
                             {{r, b}, {big, big}},
                             {{l, b}, {r, big}},
                             {{l, b}, {small, big}},
                             {{l, t}, {small, b}},
                             {{l, t}, {small, small}},
                             {{l, t}, {r, small}},
                             {{r, t}, {small, small}}};
        for (auto& s1 : outside) {
            REPORTER_ASSERT(reporter, no_intersection_by_bounding_box(s0, s1));
            REPORTER_ASSERT(reporter, no_intersection_by_bounding_box(s1, s0));
            REPORTER_ASSERT(reporter,
                            no_intersection_by_bounding_box(swap_ends(s0), swap_ends(s1)));
            REPORTER_ASSERT(reporter,
                            no_intersection_by_bounding_box(swap_ends(s0), swap_ends(s1)));
        }
    }
}

DEF_TEST(BO_no_intersection_by_sign_approximation, reporter) {

}

DEF_TEST(BO_intersectBasic, reporter) {
    {
        Segment s0 = {{-1, 0}, {1,  0}},
                s1 = {{ 0, 1}, {0, -1}};

        {
            auto answer = intersect(s0, s1);
            REPORTER_ASSERT(reporter, answer.has_value());
            Point p = {0, 0};
            REPORTER_ASSERT(reporter, answer.value() == p);
        }
        {
            auto answer = intersect(s1, s0);
            REPORTER_ASSERT(reporter, answer.has_value());
            Point p = {0, 0};
            REPORTER_ASSERT(reporter, answer.value() == p);
        }
    }
    {
        Segment s0 = {{-1, 0}, {5,  0}},
                s1 = {{ 0, 1}, {0, -1}};

        {
            auto answer = intersect(s0, s1);
            REPORTER_ASSERT(reporter, answer.has_value());
            Point p = {0, 0};
            REPORTER_ASSERT(reporter, answer.value() == p);
        }
        {
            auto answer = intersect(s1, s0);
            REPORTER_ASSERT(reporter, answer.has_value());
            Point p = {0, 0};
            REPORTER_ASSERT(reporter, answer.value() == p);
        }
    }

    {
        Segment s0 = {{5, 0}, {-1,  0}},
                s1 = {{ 0, -1}, {0, 1}};

        {
            auto answer = intersect(s0, s1);
            REPORTER_ASSERT(reporter, answer.has_value());
            Point p = {0, 0};
            REPORTER_ASSERT(reporter, answer.value() == p);
        }
        {
            auto answer = intersect(s1, s0);
            REPORTER_ASSERT(reporter, answer.has_value());
            Point p = {0, 0};
            REPORTER_ASSERT(reporter, answer.value() == p);
        }
    }
}

DEF_TEST(BO_EventQueueBasic, reporter) {
    {
        EventQueue::Queue q;
        EventQueue eq{std::move(q)};
        auto [p, v] = eq.nextEvent();
        REPORTER_ASSERT(reporter, v->empty());
    }
    {
        EventQueue::Queue q;
        Point eventPoint = {100, 100};
        q.insert({eventPoint, nullptr});
        EventQueue eq{std::move(q)};
        {
            auto [p, v] = eq.nextEvent();
            REPORTER_ASSERT(reporter, v->size() == 1);
            REPORTER_ASSERT(reporter, p == eventPoint);
        }
        {
            auto [p, v] = eq.nextEvent();
            REPORTER_ASSERT(reporter, v->empty());
        }
    }
    {
        EventQueue::Queue q;
        Point eventPoint = {100, 100};
        q.insert({eventPoint, nullptr});
        q.insert({eventPoint, nullptr});
        EventQueue eq{std::move(q)};
        {
            auto [p, el] = eq.nextEvent();
            REPORTER_ASSERT(reporter, el->size() == 2);
            REPORTER_ASSERT(reporter, p == eventPoint);
        }
        {
            auto [p, el] = eq.nextEvent();
            REPORTER_ASSERT(reporter, el->empty());
        }
    }
    {
        EventQueue::Queue q;
        q.insert({Point{200, 200}, nullptr});
        q.insert({Point{200, 100}, nullptr});
        q.insert({Point{100, 100}, nullptr});
        q.insert({Point{100, 200}, nullptr});
        EventQueue eq{std::move(q)};
        {
            const Point answers[] = {
                    Point{100, 100}, Point{200, 100}, Point{100, 200}, Point{200, 200}};
            for (auto answer : answers) {
                auto [p, el] = eq.nextEvent();
                REPORTER_ASSERT(reporter, el->size() == 1);
                REPORTER_ASSERT(reporter, p == answer);
            }
        }
        {
            auto [p, el] = eq.nextEvent();
            REPORTER_ASSERT(reporter, el->empty());
        }
    }
}

