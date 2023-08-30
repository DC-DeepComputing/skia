// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/SweepLine.h"

#include "modules/bentleyottmann/include/Point.h"

#include "tests/Test.h"

using namespace bentleyottmann;

namespace bentleyottmann {
struct SweepLineTestingPeer {
    SweepLineTestingPeer(SweepLine* sl) : fSL{sl} {}
    void verifySweepLine(int32_t y) const {
        fSL->verify(y);
    }
    void insertSegment(int i, const Segment& s) {
        auto& v = fSL->fSweepLine;
        v.insert(v.begin() + i, s);
    }
    size_t size() const {
        return fSL->fSweepLine.size();
    }

    SweepLine* const fSL;
};
}  // namespace bentleyottmann

using TP = SweepLineTestingPeer;

DEF_TEST(BO_SweepLineBasic, reporter) {
    {
        SweepLine sweepLine;
        TP tp{&sweepLine};
        tp.verifySweepLine(0);
    }
    { // No exposed intersection on delete.
        SweepLine sweepLine;
        TP tp{&sweepLine};
        EventQueue eventQueue;
        Segment s = {{0, -100}, {0, 100}};
        tp.insertSegment(1, s);
        tp.verifySweepLine(0);
        REPORTER_ASSERT(reporter, tp.size() == 3);
        Event e = {{0, 100}, Lower()};
        sweepLine.handleEvent(e, &eventQueue);
        tp.verifySweepLine(0);
        REPORTER_ASSERT(reporter, !eventQueue.hasMoreEvents());
        REPORTER_ASSERT(reporter, tp.size() == 2);
    }
    { // Expose intersection
        SweepLine sweepLine;
        TP tp{&sweepLine};
        EventQueue eventQueue;
        Segment s = {{0, -100}, {0, 100}};
        // Crossing exposed by s ending.
        Segment l = {{-3, 98}, {3, 104}};
        Segment r = {{3, 98}, {-3, 104}};
        tp.insertSegment(1, l);
        tp.insertSegment(2, s);
        tp.insertSegment(3, r);
        tp.verifySweepLine(99);
        REPORTER_ASSERT(reporter, tp.size() == 5);

        Event e = {{0, 100}, Lower()};
        sweepLine.handleEvent(e, &eventQueue);

        tp.verifySweepLine(100);
        REPORTER_ASSERT(reporter, eventQueue.hasMoreEvents());
        REPORTER_ASSERT(reporter, tp.size() == 4);
    }
}
