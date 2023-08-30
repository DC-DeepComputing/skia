// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/SweepLine.h"

#include "modules/bentleyottmann/include/EventQueueInterface.h"
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

DEF_TEST(BO_SweepLineInsert, reporter) {
    {
        SweepLine sweepLine;
        TP tp{&sweepLine};
        tp.verifySweepLine(0);
    }
    { // Handle insert
        SweepLine sweepLine;
        TP tp{&sweepLine};

        class FailIfEventQueueAdd : public EventQueueInterface {
        public:
            void addCrossing(Point crossingPoint, const Segment& s0, const Segment& s1) override {
                SK_ABORT("There should be no crossings.");
            }
        } eventQueue;
        InsertionSegmentSet insertions;
        Segment s = {{0, -100}, {0, 100}};

        insertions.insert(s);

        tp.verifySweepLine(-99);

        sweepLine.handleInsertionsAndCheckForNewCrossings(
                Point{0, -100}, insertions, &eventQueue);
        REPORTER_ASSERT(reporter, tp.size() == 3);

        tp.verifySweepLine(-99);
    }
}
