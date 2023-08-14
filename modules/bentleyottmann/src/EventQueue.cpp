// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.


#include "modules/bentleyottmann/include/EventQueue.h"

namespace bentleyottmann {
// -- Point ----------------------------------------------------------------------------------------
bool operator<(const Point& p0, const Point& p1) {
    return std::tie(p0.y, p0.x) < std::tie(p1.y, p1.x);
}

bool operator>(const Point& p0, const Point& p1) {
    return p1 < p0;
}

bool operator>=(const Point& p0, const Point& p1) {
    return !(p0 < p1);
}

bool operator<=(const Point& p0, const Point& p1) {
    return !(p0 > p1);
}

bool operator==(const Point& p0, const Point& p1) {
    return std::tie(p0.y, p0.x) == std::tie(p1.y, p0.x);
}

bool operator!=(const Point& p0, const Point& p1) {
    return !(p0 == p1);
}

Point Point::Smallest() {
    const int32_t kMinCoordinate = std::numeric_limits<int32_t>::min();
    return {kMinCoordinate, kMinCoordinate};
}

Point Point::Largest() {
    const int32_t kMaxCoordinate = std::numeric_limits<int32_t>::max();
    return {kMaxCoordinate, kMaxCoordinate};
}

bool Point::DifferenceTooBig(Point p0, Point p1) {
    auto tooBig = [](int32_t a, int32_t b) {
        return std::numeric_limits<int32_t>::max() - a <= b;
    };

    return tooBig(p0.x, p1.x) || tooBig(p0.y, p1.y);
}

// -- Segment --------------------------------------------------------------------------------------
Point Segment::upper() {
    return std::min(p0, p1);
}

Point Segment::lower() {
    return std::max(p0, p1);
}

//-- Upper -----------------------------------------------------------------------------------------
class Upper : public Event {
public:
    Upper(Point lower) : fLower{lower} {}

private:
    const Point fLower;
};

// -- Lower ----------------------------------------------------------------------------------------
class Lower : public Event {
public:
    Lower() = default;
};

// -- EventQueue -----------------------------------------------------------------------------------
std::optional<EventQueue> EventQueue::Make(SkSpan<const Segment> segments) {
    Queue queue;

    Point minPoint = Point::Largest(),
          maxPoint = Point::Smallest();

    for(const Segment& s : segments) {
        auto [upper, lower] = std::minmax(s.p0, s.p1);

        // DO NOT SUBMIT This is not right.
        minPoint = std::min(upper, minPoint);
        maxPoint = std::max(lower, maxPoint);
        queue.insert({upper, nullptr});
        queue.insert({lower, nullptr});
    }

    // If min max difference is too large, fail.
    if (Point::DifferenceTooBig(minPoint, maxPoint)) {
        return std::nullopt;
    }

    return EventQueue{std::move(queue)};
}

EventQueue::EventQueue(EventQueue::Queue&& queue) : fQueue{std::move(queue)} { }

std::tuple<Point, std::vector<Event*>*> EventQueue::nextEvent() {
    fCurrentEvents.clear();
    Point eventPoint = Point::Largest();
    if (!fQueue.empty()) {
        // The start of the range that all share the same event point.
        auto beginEvents = fQueue.lower_bound(Point::Smallest());

        // One after the range
        auto endEvents = fQueue.upper_bound(beginEvents->first);

        eventPoint = beginEvents->first;

        auto cursor = beginEvents;
        while (cursor != endEvents) {
            // Make sure all the events share the event point.
            SkASSERT(eventPoint == cursor->first);
            fCurrentEvents.push_back(cursor->second);
            cursor++;
        }

        // Remove the events that have been handled.
        fQueue.erase(beginEvents, endEvents);
    }

    return {eventPoint, &fCurrentEvents};
}
}  // namespace bentleyottmann
