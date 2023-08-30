// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/SweepLine.h"

#include "modules/bentleyottmann/include/EventQueue.h"
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
    { // Handle insert
        SweepLine sweepLine;
        TP tp{&sweepLine};
        EventQueue eventQueue;
        EventQueue::SegmentSet deletions;
        EventQueue::SlopeSegmentSet insertions;
        Segment s = {{0, -100}, {0, 100}};

        insertions.insert(s);

        tp.verifySweepLine(-99);

        sweepLine.handle(Point{0, -100}, deletions, insertions, &eventQueue);
        REPORTER_ASSERT(reporter, tp.size() == 3);

        tp.verifySweepLine(-99);

        // Should have a delete event inserted.
        REPORTER_ASSERT(reporter, !eventQueue.hasMoreEvents());
    }
}
