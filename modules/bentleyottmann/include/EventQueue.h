// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkSpan.h"
#include "include/private/base/SkAssert.h"
#include "modules/bentleyottmann/include/Point.h"
#include "modules/bentleyottmann/include/Segment.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>

#ifndef EventQueue_DEFINED
#define EventQueue_DEFINED

namespace bentleyottmann {

struct BoundingBox {
    Point topLeft = Point::Largest();
    Point bottomRight = Point::Smallest();

    void addPoint(Point p) {
        topLeft.x = std::min(p.x, topLeft.x);
        topLeft.y = std::min(p.y, topLeft.y);
        bottomRight.x = std::max(p.x, bottomRight.x);
        bottomRight.y = std::max(p.y, bottomRight.y);
    }
};

class EventQueue;
class SweepStatus;

class Event {
public:
    virtual ~Event() = default;
    virtual void handle(Point p, EventQueue* queue, SweepStatus* status) const = 0;
};

using EventList = std::vector<std::unique_ptr<Event>>;

class EventQueue {
public:
    // Queue ordered by Point.
    using Queue = std::multimap<Point, std::unique_ptr<Event>>;

    static std::optional<EventQueue> Make(SkSpan<const Segment> segments);

    EventQueue(Queue&& queue);

    // Returns the empty vector when the processing should stop.
    std::tuple<Point, EventList*> nextEvent();

private:
    Queue fQueue;
    EventList fCurrentEvents;
};

std::optional<Point> intersect(const Segment& s0, const Segment& s1);

class SweepStatus {
public:


private:
    std::vector<Segment> fStatus;
};

}  // namespace bentleyottmann
#endif  // EventQueue_DEFINED
