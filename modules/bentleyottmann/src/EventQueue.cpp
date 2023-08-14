// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.


#include "modules/bentleyottmann/include/EventQueue.h"

#include <cmath>

namespace bentleyottmann {

// -- Segment --------------------------------------------------------------------------------------
Point Segment::upper() const {
    return std::min(p0, p1);
}

Point Segment::lower() const {
    return std::max(p0, p1);
}

// Use auto [l, t, r, b] = s.bounds();
std::tuple<int32_t, int32_t, int32_t, int32_t> Segment::bounds() const {
    auto [l, r] = std::minmax(p0.x, p1.x);
    auto [t, b] = std::minmax(p0.y, p1.y);
    return std::make_tuple(l, t, r, b);
}

bool no_intersection_by_bounding_box(const Segment& s0, const Segment& s1) {
    auto [left0, top0, right0, bottom0] = s0.bounds();
    auto [left1, top1, right1, bottom1] = s1.bounds();
    // If the sides of the box touch, then there is no new intersection.
    return right0 <= left1 || right1 <= left0 || bottom0 <= top1 || bottom1 <= top0;
}

bool no_intersection_by_sign_approximation(const Point& Q, const Point& R, const Point& S) {
    // Compare the signs of two cross products Q x R & Q x S.
    // Calculate the sign of the factor.
    //  * High-bit cleared means the same signs, and therefore positive.
    //  * High-bit set means different signs, and therefore the factor is negative.
    int32_t signNegativeQxRy = Q.x ^ R.y,
            signNegativeQyRx = Q.y ^ R.x,
            signNegativeQxSy = Q.x ^ S.y,
            signNegativeQySx = Q.y ^ S.x;

    // If Q x R & Q x S have the same sign, then there is no intersection. We can conservatively
    // approximate the cross product sign using the fact that a cross product
    // a * b - c * d == e - f.
    // e:+ and f:- then the result is positive.
    // e:- and f:+ then the result is negative.
    // For the signs of both cross products to be the same, then the e's and f's sign must be the
    // same for both cross products, and e and f must have different signs.
    return (signNegativeQxRy ^ signNegativeQxSy) >= 0 &&  // First terms have different signs.
           (signNegativeQyRx ^ signNegativeQySx) >= 0 &&  // Second terms have different signs.
           (signNegativeQxRy ^ signNegativeQyRx) <  0;    // not definitely positive or negative.
}

//-- Upper -----------------------------------------------------------------------------------------
class Upper : public Event {
public:
    Upper(Point lower) : fLower{lower} {}
    void handle(Point p, EventQueue* queue, SweepStatus* status) const override {}

    const Point fLower;
};

// -- Lower ----------------------------------------------------------------------------------------
class Lower : public Event {
public:
    Lower() = default;
    void handle(Point p, EventQueue* queue, SweepStatus* status) const override {}
};

// -- EventQueue -----------------------------------------------------------------------------------
std::optional<EventQueue> EventQueue::Make(SkSpan<const Segment> segments) {
    Queue queue;

    BoundingBox boundingBox;
    for(const Segment& s : segments) {
        auto [upper, lower] = std::minmax(s.p0, s.p1);

        boundingBox.addPoint(upper);
        boundingBox.addPoint(lower);
        queue.insert({upper, std::make_unique<Upper>(lower)});
        queue.insert({lower, std::make_unique<Lower>()});
    }

    // If min max difference is too large, fail.
    if (Point::DifferenceTooBig(boundingBox.topLeft, boundingBox.bottomRight)) {
        return std::nullopt;
    }

    return EventQueue{std::move(queue)};
}

EventQueue::EventQueue(EventQueue::Queue&& queue) : fQueue{std::move(queue)} { }

std::tuple<Point, EventList*> EventQueue::nextEvent() {
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
            fCurrentEvents.push_back(std::move(cursor->second));
            cursor++;
        }

        // Remove the events that have been handled.
        fQueue.erase(beginEvents, endEvents);
    }

    return {eventPoint, &fCurrentEvents};
}

std::optional<Point> intersect(const Segment& s0, const Segment& s1) {

    // Check if the bounds intersect.
    if (no_intersection_by_bounding_box(s0, s1)) {
        return std::nullopt;
    }

    Point O = s0.p0,
          Q = s0.p1 - O,
          R = s1.p0 - O,
          S = s1.p1 - O;

    if (no_intersection_by_sign_approximation(Q, R, S)) {
        return std::nullopt;
    }

    // 64-bit cross product.
    auto cross = [](const Point& v0, const Point& v1) {
        int64_t x0 = SkToS64(v0.x),
                y0 = SkToS64(v0.y),
                x1 = SkToS64(v1.x),
                y1 = SkToS64(v1.y);
        return x0 * y1 - y0 * x1;
    };

    // Actually calculate the cross products.
    int64_t QXR = cross(Q, R),
            QXS = cross(Q, S);

    // The cross products have the same sign, so no intersection.
    if ((QXR ^ QXS) >= 0) {
        return std::nullopt;
    }

    // TODO: this calculation probably needs to use 32-bit x 64-bit -> 96-bit multiply and
    // 96-bit / 64-bit -> 32-bit quotient and a 64-bit remainder.
    // N / D constitute a value on [0, 1], where the intersection I is
    //     I = s0.p0 + (s0.p1 - s0.p0) * N/D.
    double N = QXR,
           D = QXR - QXS,
           t = N / D;

    SkASSERT(0 <= t && t <= 1);

    int32_t x = std::round(t * (s1.p1.x - s1.p0.x) + s1.p0.x),
            y = std::round(t * (s1.p1.y - s1.p0.y) + s1.p0.y);

    return Point{x, y};
}

}  // namespace bentleyottmann
