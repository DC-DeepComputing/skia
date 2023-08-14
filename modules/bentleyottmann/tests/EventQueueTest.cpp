// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/EventQueue.h"
#include "tests/Test.h"

using namespace bentleyottmann;

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

