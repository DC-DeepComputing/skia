// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef EventQueueInterface_DEFINED
#define EventQueueInterface_DEFINED

#include "modules/bentleyottmann/include/Point.h"
#include "modules/bentleyottmann/include/Segment.h"

#include <set>

namespace bentleyottmann {
class EventQueueInterface {
public:
    EventQueueInterface() = default;
    EventQueueInterface(const EventQueueInterface&) = default;
    EventQueueInterface(EventQueueInterface&&) = default;
    EventQueueInterface& operator=(const EventQueueInterface&) = default;
    EventQueueInterface& operator=(EventQueueInterface&&) = default;
    virtual ~EventQueueInterface() = default;

    virtual void addCrossing(Point crossingPoint, const Segment& s0, const Segment& s1) = 0;
};

using DeletionSegmentSet = std::set<Segment>;
struct OrderBySlope {
    bool operator()(const Segment& s0, const Segment& s1) const;
};
using InsertionSegmentSet = std::set<Segment, OrderBySlope>;

class EventHandlerInterface {
public:
    virtual ~EventHandlerInterface() = default;

    virtual void handleDeletions(Point eventPoint, const DeletionSegmentSet& removing) = 0;

    virtual void handleInsertionsAndCheckForNewCrossings(
            Point eventPoint, const InsertionSegmentSet& inserting, EventQueueInterface* queue) = 0;
};
}  // namespace bentleyottmann
#endif  // EventQueueInterface_DEFINED
