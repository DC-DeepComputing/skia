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

