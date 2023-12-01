// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Myers.h"

#include "include/core/SkSpan.h"
#include "include/private/base/SkTo.h"
#include "modules/bentleyottmann/include/Int96.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

namespace myers {

Point operator-(const Point& p0, const Point& p1) {
    return {p0.x - p1.x, p0.y - p1.y};
}

std::tuple<int64_t, int64_t> point_to_s64(Point p) {
    return std::make_tuple(SkToS64(p.x), SkToS64(p.y));
}

Point Segment::upper() const {
    return fUpper;
}

Point Segment::lower() const {
    return fLower;
}

std::tuple<int32_t, int32_t, int32_t, int32_t> Segment::bounds() const {
    auto [left, right] = std::minmax(fUpper.x, fLower.x);
    return std::make_tuple(left, fUpper.y, right, fLower.y);
}

bool Segment::isHorizontal() const {
    return fUpper.y == fLower.y;
}

bool Segment::isVertical() const {
    return fUpper.x == fLower.x;
}

int64_t compare_slopes(const Segment& s0, const Segment& s1) {
    const auto [u0, l0] = s0;
    const auto [u1, l1] = s1;

    const auto [d0x, d0y] = point_to_s64(l0 - u0);
    const auto [d1x, d1y] = point_to_s64(l1 - u1);

    // Since horizontal lines are handled separately and the ordering of points for the segment,
    // then there should always be positive dny.
    SkASSERT(d0y >= 0 && d1y >= 0);

    //     * slope(s0) = d0x / d0y
    //     * slope(s1) = d1x / d1y
    // If we want to find d0x / d0y < d1x / d1y, then
    //    d0x * d1y < d1x * d0y
    //    d0x * d1y - d1x * d0y < 0.
    // We know that d0y and d1y are both positive, so we can do a cross multiply without worrying
    // about changing the relation.
    // Here are the combinations:
    //    * <  - cross_of_slopes(s0, s1) < 0
    //    * == - cross_of_slopes(s0, s1) == 0
    //    * >  - cross_of_slopes(s0, s1) > 0
    int64_t cross = d0x * d1y - d1x * d0y;
    return cross;
}

bool slope_s0_less_than_slope_s1(const Segment& s0, const Segment& s1) {
    // Horizontal have infinite slopes.
    if (s0.isHorizontal()) {
        // Either s1 is not horizontal, then s0's slope is bigger, or s1 is horizontal and
        // therefore equal to s0's slope.
        return false;
    } else if (s1.isHorizontal()) {
        // s0 is not horizontal, so s1's slope is always larger.
        return true;
    }

    return compare_slopes(s0, s1) < 0;
}

// element < value
bool element_less_than_upper_value(const Segment& element, const Segment& value) {
    const auto [x, y] = value.upper();

    {
        const auto [l, r] = std::minmax(element.upper().x, element.lower().x);
        if (x < l) {
            return false;
        }

        if (r < x) {
            return true;
        }
    }

    const auto [u, l] = element;
    const auto [dx, dy] = point_to_s64(l - u);

    //    u.x + dx * (y - u.y) / dy < x
    //    dx * (y - u.y) / dy < x - u.x
    //    dx * (y - u.y) < dy * (x - u.x)
    int64_t lhs = dx * (y - u.y),
            rhs = dy * (x - u.x);

    return (lhs < rhs) || ((lhs == rhs) && slope_s0_less_than_slope_s1(element, value));
}

int64_t compare_point_to_segment(Point p, const Segment& s) {
    const auto [x, y] = p;
    const auto [u, l] = s;

    // The upper point is on s when
    // x = u.x + (l.x - u.x)(y - u.y) / (l.y - u.y),
    // x - u.x = (l.x - u.x)(y - u.y) / (l.y - u.y),
    // (x - u.x)(l.y - u.y) = (l.x - u.x)(y - u.y),
    // where x and y are upper() of s0 and u and l are upper and lower of s1.
    const int64_t sDx = l.x - u.x;
    const int64_t sDy = l.y - u.y;
    const int64_t pDx = x - u.x;
    const int64_t pDy = y - u.y;

    // * p < s is negative
    // * p == s is 0
    // * p > s is positive
    int64_t cross = pDx * sDy - sDx * pDy;
    return cross;
}

std::optional<bool> s0_intersects_s1_easy_cases(const Segment& s0, const Segment& s1) {
    const auto [left0, right0] = std::minmax(s0.upper().x, s0.lower().x);
    const auto [left1, right1] = std::minmax(s1.upper().x, s1.lower().x);

    if (right0 < left1 || right1 < left0) {
        return false;
    }

    if (s0.upper() == s1.upper() || s0.upper() == s1.lower() ||
        s0.lower() == s1.upper() || s0.lower() == s1.lower()) {
        return true;
    }

    return std::nullopt;
}

// Return true if upper(s0) is on interior of s1.
bool upper_horizontal_s0_intersects_s1(const Segment& s0, const Segment& s1) {
    SkASSERT(s0.isHorizontal());

    if (auto easyCases = s0_intersects_s1_easy_cases(s0, s1)) {
        return easyCases.value();
    }

    if (s1.isHorizontal()) {
        return true;
    }

    return compare_point_to_segment(s0.upper(), s1) <= 0 &&
           compare_point_to_segment(s0.lower(), s1) >= 0;
}

// Return true if upper(s0) is on interior of s1.
bool upper_s0_intersects_s1(const Segment& s0, const Segment& s1) {
    SkASSERT(!s0.isHorizontal());

    if (auto easyCases = s0_intersects_s1_easy_cases(s0, s1)) {
        return easyCases.value();
    }

    return compare_point_to_segment(s0.upper(), s1) == 0;
}

// Return true if s0(y) < s1(y) else if s0(y) == s1(y) then slope(s0) < slope(s1)
bool s0_less_than_s1_at_y(const Segment& s0, const Segment& s1, int32_t y) {
    // Neither s0 nor s1 are horizontal because this is used during the sorting phase
    SkASSERT(!s0.isHorizontal() && !s1.isHorizontal());

    const auto [u0, l0] = s0;
    const auto [u1, l1] = s1;

    const auto [left0, right0] = std::minmax(s0.upper().x, s0.lower().x);
    const auto [left1, right1] = std::minmax(s1.upper().x, s1.lower().x);

    if (right0 < left1) {
        return true;
    } else if (right1 < left0) {
        return false;
    }

    const Point d0 = l0 - u0;
    const Point d1 = l1 - u1;

    // Since horizontal lines are handled separately and the ordering of points for the segment,
    // then there should always be positive Dy.
    SkASSERT(d0.y > 0 && d1.y > 0);

    namespace bo = bentleyottmann;
    using Int96 = bo::Int96;

    // Defining s0(y) and s1(y),
    //    s0(y) = u0.x + (y - u0.y) * d0.x / d0.y
    //    s1(y) = u1.x + (y - y1.y) * d1.x / d1.y
    // Find the following
    //    s0(y) < s1(y)
    // Substituting s0(y) and s1(y)
    //    u0.x + (y - u0.y) * d0.x / d0.y < u1.x + (y - y1.y) * d1.x / d1.y
    // Factoring out the denominator.
    //    (u0.x * d0.y + (y - u0.y) * d0.x) / d0.y < (u1.x * d1.y + (y - y1.y) * d1.x) / d1.y
    // Cross-multiplying the denominators. The sign will not switch because d0.y and d1.y are
    // always positive.
    //    d1.y * (u0.x * d0.y + (y - u0.y) * d0.x) < d0.y * (u1.x * d1.y + (y - y1.y) * d1.x)
    // If these are equal, then we use the slope to break the tie.
    //    d0.x / d0.y < d1.x / d1.y
    // Cross multiplying leaves.
    //    d0.x * d1.y < d1.x < d0.y
    const Int96 lhs = bo::multiply(d1.y, u0.x * SkToS64(d0.y) + (y - u0.y) * SkToS64(d0.x));
    const Int96 rhs = bo::multiply(d0.y, u1.x * SkToS64(d1.y) + (y - u1.y) * SkToS64(d1.x));

    return lhs < rhs || ((lhs == rhs) && slope_s0_less_than_slope_s1(s0, s1));
}

struct Event {
    const int32_t y;
    SkSpan<Segment> begin;
    SkSpan<Segment> horizontal;
    SkSpan<Segment> end;
};

class EventQueue {
    class CompactEvent {
    public:
        const int32_t y;
        const int32_t start;
        const int32_t endOfBegin;
        const int32_t endOfHorizontal;
        const int32_t endOfEnd;
    };

public:
    static EventQueue Make(SkSpan<const Segment> segments) {
        SkASSERT(!segments.empty());
        SkASSERT(segments.size() < INT32_MAX);

        using SetupTuple = std::tuple<Segment, int32_t, int, int32_t>;
        enum EventType {
            kBegin = 0,
            kHorizontal = 1,
            kEnd = 2
        };

        std::vector<SetupTuple> eventOrdering;
        for (const auto& s : segments) {

            // Exclude zero length segments.
            if (s.upper() == s.lower()) {
                continue;
            }

            if (s.isHorizontal()) {
                eventOrdering.emplace_back(s, s.upper().y, kHorizontal, -s.upper().x);
            } else {
                eventOrdering.emplace_back(s, s.upper().y, kBegin, -s.upper().x);
                eventOrdering.emplace_back(s, s.lower().y, kEnd, -s.upper().x);
            }
        }

        auto eventLess = [](const SetupTuple& l, const SetupTuple& r) {
            return std::tie(std::get<1>(l), std::get<2>(l), std::get<3>(l)) <
                      std::tie(std::get<1>(r), std::get<2>(r), std::get<3>(r));
        };

        // Remove duplicate segments.
        std::sort(eventOrdering.begin(), eventOrdering.end(), eventLess);
        auto eraseFrom = std::unique(eventOrdering.begin(), eventOrdering.end());
        eventOrdering.erase(eraseFrom, eventOrdering.end());

        std::vector<CompactEvent> events;
        std::vector<Segment> segmentStorage;
        int32_t currentEvent = std::get<1>(eventOrdering.front());
        int32_t start = 0,
                endOfBegin = 0,
                endOfHorizontal = 0,
                endOfEnd = 0;
        for (const auto& [s, event, type, _] : eventOrdering) {
            if (currentEvent != event) {
                events.push_back(CompactEvent{currentEvent,
                                              start,
                                              endOfBegin,
                                              endOfHorizontal,
                                              endOfEnd});
                start = endOfBegin = endOfHorizontal = endOfEnd = segmentStorage.size();
                currentEvent = event;
            }

            segmentStorage.push_back(s);

            const size_t segmentCount = segmentStorage.size();
            switch (type) {
                case kBegin: endOfBegin = segmentCount;
                    [[fallthrough]];
                case kHorizontal: endOfHorizontal = segmentCount;
                    [[fallthrough]];
                case kEnd: endOfEnd = segmentCount;
            }
        }

        // Store the last event.
        events.push_back(CompactEvent{currentEvent,
                                      start,
                                      endOfBegin,
                                      endOfHorizontal,
                                      endOfEnd});

        return EventQueue{std::move(events), std::move(segmentStorage)};
    }

    EventQueue(std::vector<CompactEvent>&& events, std::vector<Segment>&& segmentStorage)
        : fEvents{std::move(events)}
        , fSegmentStorage{std::move(segmentStorage)} {}

    bool hasMoreEvents() const {
        return fCurrentEvent < SkTo<int32_t>(fEvents.size());
    }

    Event nextEvent() {
        auto& [y, start, endOfBegin, endOfHorizontal, endOfEnd] = fEvents[fCurrentEvent];
        fCurrentEvent += 1;
        SkSpan<Segment> begin{&fSegmentStorage[start], endOfBegin - start};
        SkSpan<Segment> horizontal{&fSegmentStorage[endOfBegin], endOfHorizontal - endOfBegin};
        SkSpan<Segment> end{&fSegmentStorage[endOfHorizontal], endOfEnd - endOfHorizontal};

        return {y, begin, horizontal, end};
    }

private:
    const std::vector<CompactEvent> fEvents;
    std::vector<Segment> fSegmentStorage;
    int32_t fCurrentEvent = 0;
};

class CrossingAccumulator {
public:
    void recordCrossing(const Segment& s0, const Segment& s1) {
        if (s0.upper() == s1.lower() || s0.lower() == s1.upper()) {
            return;
        }
        if ((s0.upper() == s1.upper() || s0.lower() == s1.lower()) && compare_slopes(s0, s1) != 0) {
            return;
        }

        fCrossings.emplace_back(s0, s1);
    }

    std::vector<Crossing> finishAndReleaseCrossings() {
        return std::move(fCrossings);
    }

private:
    std::vector<Crossing> fCrossings;
};

class SweepLine {
    static constexpr Segment kLeftStatusSentinel{{INT32_MIN, INT32_MIN}, {INT32_MIN, INT32_MAX}};
    static constexpr Segment kRightStatusSentinel{{INT32_MAX, INT32_MIN}, {INT32_MAX, INT32_MAX}};

public:
    SweepLine() {
        fStatus.push_back(kLeftStatusSentinel);
        fStatus.push_back(kRightStatusSentinel);
    }

    void handleEvent(Event e) {
        auto& [y, beginnings, horizontals, endings] = e;

        // Things could be out of order from last event.
        this->sortAndRecord(y);

        this->handleBeginnings(y, beginnings);
        this->handleHorizontal(y, horizontals);
        this->handleEndings(y, endings);
    }

    std::vector<Crossing> finishAndReleaseCrossings() {
        // Only the sentinels should be left.
        SkASSERT(this->statusEmpty());
        return fCrossings.finishAndReleaseCrossings();
    }

private:
    friend class SweepLineTestingPeer;
    using StatusLine = std::vector<Segment>;

    class SegmentLessAtY {
    public:
        SegmentLessAtY(int32_t y) : fY{y} {}
        bool operator()(const Segment& s0, const Segment& s1) {
            return s0_less_than_s1_at_y(s0, s1, fY);
        }

    private:
        const int32_t fY;
    };

    bool statusEmpty() const {
        return fStatus.size() == 2;
    }



    void sortAndRecord(int32_t y) {
        // If there are only the sentinels or just 1 segment, then nothing to sort.
        if (fStatus.size() <= 3) {
            return;
        }

        // Skip the first and last sentinels.
        for (size_t i = 2; i < fStatus.size() - 1; ++i) {
            const Segment t = fStatus[i];
            size_t j = i;
            for (; j > 1 && s0_less_than_s1_at_y(t, fStatus[j - 1], y); --j) {
                // While t < the thing before it move it down.
                fCrossings.recordCrossing(t, fStatus[j-1]);
                fStatus[j] = fStatus[j-1];
            }
            fStatus[j] = t;
        }
    }

    void checkCrossingsLeftAndRight(const Segment& segment, StatusLine::iterator insertionPoint) {
        SkASSERT(!segment.isHorizontal());

        // Match to the left using the left sentinel to break the loop.
        for (auto cursor = std::make_reverse_iterator(insertionPoint);
             upper_s0_intersects_s1(segment, *cursor);
             cursor++) {
            fCrossings.recordCrossing(segment, *cursor);
        }

        // Match to the right using the right sentinel to break the loop.
        for (auto cursor = insertionPoint; upper_s0_intersects_s1(segment, *cursor); cursor++) {
            fCrossings.recordCrossing(segment, *cursor);
        }
    }

    void checkCrossingsLeftAndRightForHorizontal(
            const Segment& segment, StatusLine::iterator insertionPoint) {
        SkASSERT(segment.isHorizontal());

        // Match to the left using the left sentinel to break the loop.
        for (auto cursor = std::make_reverse_iterator(insertionPoint);
             upper_horizontal_s0_intersects_s1(segment, *cursor);
             cursor++) {
            fCrossings.recordCrossing(segment, *cursor);
        }

        // Match to the right using the right sentinel to break the loop.
        for (auto cursor = insertionPoint;
             upper_horizontal_s0_intersects_s1(segment, *cursor);
             cursor++) {
            fCrossings.recordCrossing(segment, *cursor);
        }
    }

    void handleBeginnings(int32_t y, SkSpan<Segment> inserting) {
        for (const Segment& s : inserting) {
            auto insertionPoint =
                std::lower_bound(fStatus.begin(), fStatus.end(), s, element_less_than_upper_value);
            this->checkCrossingsLeftAndRight(s, insertionPoint);
            fStatus.insert(insertionPoint, s);
        }
    }

    void handleHorizontal(int32_t y, SkSpan<Segment> inserting) {
        for (const Segment& s : inserting) {
            auto insertionPoint =
                std::lower_bound(fStatus.begin(), fStatus.end(), s, element_less_than_upper_value);
            this->checkCrossingsLeftAndRightForHorizontal(s, insertionPoint);
            fStatus.insert(insertionPoint, s);
        }

        for (const Segment& s : inserting) {
            auto removedPoint = std::remove(fStatus.begin(), fStatus.end(), s);
            SkASSERT(removedPoint != fStatus.end());
            fStatus.erase(removedPoint, fStatus.end());
        }
    }

    void handleEndings(int32_t y, SkSpan<Segment> removing) {
        for (const Segment& s : removing) {
            auto removedPoint = std::remove(fStatus.begin(), fStatus.end(), s);
            SkASSERT(removedPoint != fStatus.end());
            fStatus.erase(removedPoint, fStatus.end());
        }
    }

    StatusLine fStatus;
    CrossingAccumulator fCrossings;
};

SkSpan<Segment> remove_zero_segments_and_duplicates(SkSpan<Segment> segments) {
    auto isZeroSegment = [](const Segment& segment) {
        return segment.upper() == segment.lower();
    };
    const auto zeroSegments = std::remove_if(segments.begin(), segments.end(), isZeroSegment);

    std::sort(segments.begin(), zeroSegments);

    const auto duplicateSegments = std::unique(segments.begin(), zeroSegments);

    return SkSpan{segments.data(), std::distance(segments.begin(), duplicateSegments)};
}

std::vector<Crossing> myers_find_crossings(SkSpan<Segment> segments) {

    SkSpan<const Segment> cleanSegments = remove_zero_segments_and_duplicates(segments);

    EventQueue eventQueue = EventQueue::Make(cleanSegments);
    SweepLine sweepLine;

    while (eventQueue.hasMoreEvents()) {
        sweepLine.handleEvent(eventQueue.nextEvent());
    }

    return sweepLine.finishAndReleaseCrossings();
}


// Returns if there is an intersection on one of the segments, but if there is end-point to
// end-point intersection then this is rejected. Those intersections are already in the data.
bool has_inner_intersection(const Segment& s0, const Segment& s1) {

    if (s1.upper().y < s0.upper().y
        || ((s1.upper().y == s0.upper().y) && (s1.lower().y > s0.lower().y))) {
            return has_inner_intersection(s1, s0);
    }

    SkASSERT((s0.upper().y <= s1.upper().y));

    {  // If extents don't overlap then there is no intersection.
        auto [l0, t0, r0, b0] = s0.bounds();
        auto [l1, t1, r1, b1] = s1.bounds();
        if (r1 < l0 || r0 < l1 || b1 < t0 || b0 < t1) {
            return false;
        }
    }

    if (s0.upper() == s1.lower() || s0.lower() == s1.upper()) {
        // Segments overlap at opposite endpoints, but no other portions overlap, there is no
        // intersection.
        return false;
    }

    if ((s0.isHorizontal() && s1.isHorizontal()) || (s0.isVertical() && s1.isVertical())) {
        return true;
    }

    // 64-bit cross product.
    auto cross = [](const Point& v0, const Point& v1) {
        const auto [x0, y0] = point_to_s64(v0);
        const auto [x1, y1] = point_to_s64(v1);
        return x0 * y1 - y0 * x1;
    };

    auto [u0, l0] = s0;
    auto [u1, l1] = s1;

    const Point D0 = l0 - u0,
                D1 = l1 - u1;

    // This is the cross product of the slopes. If it is 0, then the slopes are equal.
    const int64_t D1xD0 = cross(D1, D0);

    if (s0.upper() == s1.upper() || s0.lower() == s1.lower()) {
        // There is an intersection only if the segments are co-linear, otherwise the segments
        // don't intersect.
        return D1xD0 == 0;
    }

    // The vector from the start of one vector to the other.
    const Point R = u1 - u0;

    const int64_t D0xR = cross(D0, R);

    if (D0xR == 0) {
        return true;
    }

    if (D1xD0 == 0) {
        return false;
    }

    if (l1.y < l0.y) {
        const int64_t D0xS = D0xR - D1xD0;

        if (D0xS == 0) {
            // The endpoint is on the interior of S0.
            return true;
        }

        return (D0xR ^ D0xS) < 0;
    }

    const int64_t endCross = cross(D1, l0 - u1);
    if (endCross == 0) {
        return ((D1xD0 ^ D0xR) >= 0) && std::abs(D0xR) < std::abs(D1xD0);
    }
    return (D0xR ^ endCross) >= 0;
}


// Derivation of Intersection
// The intersection point I = (X, Y) of the two segments (x0, y0) -> (x1, y1)
// and (x2, y2) -> (x3, y3).
//    X = x0 + s(x1 - x0) = x2 + t(x3 - x2)
//    Y = y0 + s(y1 - y0) = y2 + t(y3 - y2)
//
// Solve for s in terms of t.
//    x0 + s(x1 - x0) = x2 + t(x3 - x2)
//    s(x1 - x0) = x2 - x0 + t(x3 - x2)
//    s = (x2 - x0 + t(x3 - x2)) / (x1 - x0)
//
// Back substitute s into the equation for Y.
//    y0 + ((x2 - x0 + t(x3 - x2)) / (x1 - x0))(y1 - y0) = y2 + t(y3 - y2)
//    (x2 - x0 + t(x3 - x2)) / (x1 - x0) = (y2 - y0 + t(y3 - y2)) / (y1 - y0)
//    (y1 - y0)(x2 - x0 + t(x3 - x2)) = (x1 - x0)(y2 - y0 + t(y3 - y2))
//    (y1 - y0)(x2 - x0) + t(y1 - y0)(x3 - x2) = (x1 - x0)(y2 - y0) + t(x1 - x0)(y3 - y2)
// Collecting t's on one side, and constants on the other.
//    t((y1 - y0)(x3 - x2) - (x1 - x0)(y3 - y2)) = (x1 - x0)(y2 - y0) - (y1 - y0)(x2 - x0)
//    t((d0y * d1x) - (d0x * d1y)) = (d0x * (y2 - y0)) - (d0y * (x2 - x0))
//    t(d1 x d0) = d0 x (p2 - p0)
//    t = (d0 x (p2 - p0)) / (d1 x d0)
//
// Solve for t in terms of x.
//    x0 + s(x1 - x0) = x2 + t(x3 - x2)
//    x0 - x2 + s(x1 - x0) = t(x3 - x2)
//    (x0 - x2 + s(x1 - x0)) / (x3 - x2) = t
// Back substitute t into the equation for Y.
//    y0 + s(y1 - y0) = y2 + ((x0 - x2 + s(x1 - x0)) / (x3 - x2))(y3 - y2)
//    (y0 - y2 + s(y1 - y0)) / (y3 - y2) = (x0 - x2 + s(x1 - x0)) / (x3 - x2)
//    (x3 - x2)(y0 - y2 + s(y1 - y0)) = (y3 - y2)(x0 - x2 + s(x1 - x0))
//    (x3 - x2)(y0 - y2) + s(x3 - x2)(y1 - y0) = (y3 - y2)(x0 - x2) + s(y3 - y2)(x1 - x0)
// Collecting s's on on side and constants on the other.
//    s((x3 - x2)(y1 - y0) - (y3 - y2)(x1 - x0)) = (y3 - y2)(x0 - x2) - (x3 - x2)(y0 - y2)
//    s((d1x * d0y) - (d1y * d0x)) = d1y * (x0 - x2) - d1x * (y0 - y2)
//    s(d1 x d0) = (p0 - p2) x d1
//    s = ((p0 - p2) x d1) / (d1 x d0)
//    s = (d1 x (p2 - p0)) / (d1 x d0)


bool has_inner_intersection2(const Segment& s0, const Segment& s1) {
    // Make sure that s0 upper is above s1 upper.
    if (s1.upper().y < s0.upper().y
        || ((s1.upper().y == s0.upper().y) && (s1.lower().y > s0.lower().y))) {
        return has_inner_intersection2(s1, s0);
    }

    SkASSERT((s0.upper().y <= s1.upper().y));

    {  // If extents don't overlap then there is no intersection.
        auto [l0, t0, r0, b0] = s0.bounds();
        auto [l1, t1, r1, b1] = s1.bounds();
        if (r1 < l0 || r0 < l1 || b1 < t0 || b0 < t1) {
            return false;
        }
    }

    // 64-bit cross product.
    auto cross = [](const Point& v0, const Point& v1) {
        const auto [x0, y0] = point_to_s64(v0);
        const auto [x1, y1] = point_to_s64(v1);
        return x0 * y1 - y0 * x1;
    };

    auto [u0, l0] = s0;
    auto [u1, l1] = s1;

    const Point D0 = l0 - u0,
                D1 = l1 - u1;

    // This is the cross product of the slopes. If it is 0, then the slopes are equal.
    const Point U0toU1 = (u1 - u0);
    const int64_t D0xU0toU1 = cross(D0, U0toU1);
    if (D0xU0toU1 == 0) {
        // u1 is on s0.
        return true;
    }

    if (l1.y <= l0.y) {
        const Point U0toL1 = (l1 - u0);
        const int64_t D0xU0toL1 = cross(D0, U0toL1);
        if (D0xU0toL1 == 0) {
            // l1 is on s0.
            return true;
        }

        return (D0xU0toU1 ^ D0xU0toL1) < 0;
    } else {
        const Point U1toL0 = (l0 - u1);
        const int64_t D1xU1toL0 = cross(D1, U1toL0);
        if (D1xU1toL0 == 0) {
            return true;
        }
        return (D0xU0toU1 ^ D1xU1toL0) >= 0;
    }
}

std::vector<Crossing> brute_force_crossings(SkSpan<Segment> segments) {

    SkSpan<const Segment> cleanSegments = remove_zero_segments_and_duplicates(segments);

    CrossingAccumulator crossings;
    if (cleanSegments.size() >= 2) {
        for (auto i = cleanSegments.begin(); i != std::prev(cleanSegments.end()); ++i) {
            for (auto j = std::next(i); j != cleanSegments.end(); ++j) {
                if (has_inner_intersection2(*i, *j)) {
                    crossings.recordCrossing(*i, *j);
                }
            }
        }
    }
    return crossings.finishAndReleaseCrossings();
}
}  // namespace myers
