// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SweepLine_DEFINED
#define SweepLine_DEFINED

#include "modules/bentleyottmann/include/EventQueue.h"
#include "modules/bentleyottmann/include/Point.h"
#include "modules/bentleyottmann/include/Segment.h"

#include <limits>
#include <vector>

namespace bentleyottmann {

class SweepLine : public EventQueue::EventHandler {
public:
    SweepLine();
    void handleEvent(const Event& event, EventQueue* queue);

    void handle(Point eventPoint,
                const EventQueue::SegmentSet& removing,
                const EventQueue::SlopeSegmentSet& inserting,
                EventQueue* queue) override;

private:
    friend struct SweepLineTestingPeer;

    void verify(int32_t y) const;

    std::vector<Segment> fSweepLine;
};
}  // namespace bentleyottmann
#endif  // SweepLine_DEFINED
