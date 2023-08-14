// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkSpan.h"
#include "include/private/base/SkAssert.h"

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

struct Point {
    int32_t x;
    int32_t y;

    // Relation for ordering events.
    friend bool operator<(const Point& p0, const Point& p1);
    friend bool operator>(const Point& p0, const Point& p1);
    friend bool operator>=(const Point& p0, const Point& p1);
    friend bool operator<=(const Point& p0, const Point& p1);

    // Equality
    friend bool operator==(const Point& p0, const Point& p1);
    friend bool operator!=(const Point& p0, const Point& p1);

    // Extremes
    static Point Smallest();
    static Point Largest();
    static bool DifferenceTooBig(Point p0, Point p1);
};

struct Segment {
    Point p0;
    Point p1;

    // Because Y is larger going down the y-axis.
    // Get the higher point. It will be left most for horizontal segment.
    Point upper();

    // Get the lower point. It will be the right most for horizontal segment.
    Point lower();
};

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

class SweepStatus {
public:


private:
    std::vector<Segment> fStatus;
};
}  // namespace bentleyottmann
#endif  // EventQueue_DEFINED
