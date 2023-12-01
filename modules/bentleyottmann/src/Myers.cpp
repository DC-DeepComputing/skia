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

bool Segment::isDiagonal() const {
    return fUpper.x != fLower.x && fUpper.y != fLower.y;
}

bool slope_s0_less_than_slope_s1(const Segment& s0, const Segment& s1) {
    const auto [u0, l0] = s0;
    const auto [u1, l1] = s1;

    const auto [d0x, d0y] = point_to_s64(l0 - u0);
    const auto [d1x, d1y] = point_to_s64(l1 - u1);

    // Since horizontal lines are handled separately and the ordering of points for the segment,
    // then there should always be positive dn.y.
    SkASSERT(d0y >= 0 && d1y >= 0);

    //     * slope(s0) = d0x / d0y
    //     * slope(s1) = d1x / d1y
    // We want to find d0x / d0y < d1x / d1y.
    //
    // We know that d0y and d1y are both positive, so we can do a cross multiply without worrying
    // about changing the relation.
    return d0x * d1y < d1x * d0y;
}

bool upper_s0_less_than_s1(const Segment& s0, const Segment& s1) {
    const auto [x, y] = s0.upper();

    {
        const auto [l, r] = std::minmax(s1.upper().x, s1.lower().x);
        if (x < l) {
            return true;
        }

        if (r < x) {
            return false;
        }
    }

    const auto [u, l] = s1;
    const auto [dx, dy] = point_to_s64(l - u);

    //    x < u.x + dx * (y - u.y) / dy
    //    x - u.x < dx * (y - u.y) / dy
    //    dy * (x - u.x) < dx * (y - u.y)
    int64_t lhs = dy * (x - u.x),
            rhs = dx * (y - u.y);

    return (lhs < rhs) || ((lhs == rhs) && slope_s0_less_than_slope_s1(s0, s1));
}

// Return true if upper(s0) is on interior of s1.
bool upper_s0_on_s1(const Segment& s0, const Segment& s1) {

    {
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
    }

    if (s0.isHorizontal() && s1.isHorizontal()) {
        return true;
    }

    const auto [x, y] = s0.upper();
    const auto [u, l] = s1;



    // Check extents
    const auto [left, right] = std::minmax(u.x, l.x);
    if (right < x || x < left) {
        return false;
    }

    // If it begins at one of the end points, then we already know that intersection.
    if (u == Point{x, y} || l == Point{x, y}) {
        return false;
    }

    // Special case horizontal lines.
    if (s1.isHorizontal()) {
        // x is between the ends of s.
        return u.x < x && x < l.x;
    }

    // The upper point is on s when
    // x = u.x + (l.x - u.x)(y - u.y) / (l.y - u.y),
    // x - u.x = (l.x - u.x)(y - u.y) / (l.y - u.y),
    // (x - u.x)(l.y - u.y) = (l.x - u.x)(y - u.y),
    // where x and y are upper() of s0 and u and l are upper and lower of s1.
    const int64_t sDx = l.x - u.x;
    const int64_t sDy = l.y - u.y;
    const int64_t pDx = x - u.x;
    const int64_t pDy = y - u.y;

    return pDx * sDy == sDx * pDy;
}

// Let s0 be this segment, and let s1 be s.
// Return true if s0(y) < s1(y) else if s0(y) == s1(y) then slope(s0) < slope(s1)
bool less_than_at_y(const Segment& s0, const Segment& s1, int32_t y) {
    const auto [u0, l0] = s0;
    const auto [u1, l1] = s1;

    // Handle all the horizontal cases.
    if (s0.isHorizontal() || s1.isHorizontal()) {
        if (s0.isHorizontal() && s1.isHorizontal()) {
            return s0.upper().x < s1.upper().x;
        }

        auto horizontalLess = [y](int32_t x, const Segment& s) {
            const Point origin = s.lower();
            const auto [ox, oy] = origin;
            const auto [dx, dy] = point_to_s64(s.upper() - origin);
            return dy * x < ox * dy + dx * (y - oy);
        };

        if (s0.isHorizontal()) {
            return horizontalLess(u0.x, s1);
        } else {
            return horizontalLess(u1.x, s0);
        }
    }

    // Check extents
    const auto [left0, right0] = std::minmax(u0.x, l0.x);
    const auto [left1, right1] = std::minmax(u1.x, l1.x);

    if (right0 <= left1) {
        // s0 is fully to the left of s1.
        return true;
    } else if (right1 <= left0) {
        // s0 is fully to the right of s1.
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

    if (lhs < rhs) {
        return true;
    } else if (lhs == rhs) {
        // Break tie using slope.
        return slope_s0_less_than_slope_s1(s0, s1);
    }

    return false;
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

    std::vector<Crossing> finishAndOwnCrossings() {
        // Only the sentinels should be left.
        SkASSERT(this->statusEmpty());
        return std::move(fCrossings);
    }

private:
    friend class SweepLineTestingPeer;
    using StatusLine = std::vector<Segment>;

    class SegmentLessAtY {
    public:
        SegmentLessAtY(int32_t y) : fY{y} {}
        bool operator()(const Segment& s0, const Segment& s1) {
            return less_than_at_y(s0, s1, fY);
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

        // Skip the first sentinel.
        for (size_t i = 2; i < fStatus.size(); ++i) {
            const Segment t = fStatus[i];
            size_t j = i;
            for (; j > 1 && less_than_at_y(t, fStatus[j-1], y); --j) {
                // While t < the thing before it move it down.
                fCrossings.emplace_back(t, fStatus[j-1]);
                fStatus[j] = fStatus[j-1];
            }
            fStatus[j] = t;
        }
    }

    void checkCrossingsLeftAndRight(const Segment& segment, StatusLine::iterator insertionPoint) {
        // Match to the left using the left sentinel to break the loop.
        for (auto cursor = std::make_reverse_iterator(insertionPoint);
             upper_s0_on_s1(segment, *cursor);
             cursor++) {
            fCrossings.emplace_back(segment, *cursor);
        }

        // Match to the right using the right sentinel to break the loop.
        for (auto cursor = insertionPoint; upper_s0_on_s1(segment, *cursor); cursor++) {
            fCrossings.emplace_back(segment, *cursor);
        }
    }

    void handleBeginnings(int32_t y, SkSpan<Segment> inserting) {
        SegmentLessAtY less{y};
        for (const Segment& s : inserting) {
            auto insertionPoint = std::lower_bound(fStatus.begin(), fStatus.end(), s, less);
            this->checkCrossingsLeftAndRight(s, insertionPoint);
            fStatus.insert(insertionPoint, s);
        }
    }

    void handleHorizontal(int32_t y, SkSpan<Segment> inserting) {
        auto less = [](const Segment& s0, const Segment& s1) {
            return upper_s0_less_than_s1(s0, s1);
        };
        for (const Segment& s : inserting) {
            auto insertionPoint = std::lower_bound(fStatus.begin(), fStatus.end(), s, less);
            this->checkCrossingsLeftAndRight(s, insertionPoint);
            fStatus.insert(insertionPoint, s);
        }

        for (const Segment& s : inserting) {
            auto removalPoint = std::lower_bound(fStatus.begin(), fStatus.end(), s, less);
            fStatus.erase(removalPoint);
        }
    }

    void handleEndings(int32_t y, SkSpan<Segment> removing) {
        SegmentLessAtY less{y};
        for (const Segment& s : removing) {
            auto removalPoint = std::lower_bound(fStatus.begin(), fStatus.end(), s, less);
            fStatus.erase(removalPoint);
        }
    }

    StatusLine fStatus;
    std::vector<Crossing> fCrossings;
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

    return sweepLine.finishAndOwnCrossings();
}


// Returns if there is an intersection on one of the segments, but if there is end-point to
// end-point intersection then this is rejected. Those intersections are already in the data.
bool inner_intersect(const Segment& s0, const Segment& s1) {

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

    if (!s0.isDiagonal() || !s1.isDiagonal()) {
        // Since the extents intersect, then these cross unless it's at the endpoints not
        // already checked.
        return s0.upper() != s1.upper() && s0.lower() != s1.lower();
    }

    // Only diagonals left.
    SkASSERT(s0.isDiagonal() && s1.isDiagonal());

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

    if (s0.upper() == s1.upper() && s0.lower() == s1.lower()) {
        // There is an intersection only if the segments are co-linear, otherwise the segments
        // don't intersect.
        return D1xD0 == 0;
    }

    // The vector from the start of one vector to the other.
    const Point R = u1 - u0;

    const int64_t D0xR = cross(D0, R);

    if (D1xD0 == 0) {
        // If R has the same slope as D0, then the segments are co-linear.
        return D0xR == 0;
    }

    const int64_t D1xR = cross(D1, R);

    // t = (D0 x R) / (D1 x D0). s = (D1 x R) / (D1 x D0). Check that t & s are on [0, 1].
    if ((D0xR ^ D1xD0) < 0 || (D1xR ^ D1xD0) < 0) {
        // The division is negative and t < 0 or s < 0.
        return false;
    }

    if (D1xD0 > 0) {
        // Both s and t are less than or equal to 1.
        return D0xR <= D1xD0 && D1xR <= D1xD0;
    } else {
        return D0xR >= D1xD0 && D1xR >= D1xD0;
    }
}

std::vector<Crossing> brute_force_crossings(SkSpan<Segment> segments) {

    SkSpan<const Segment> cleanSegments = remove_zero_segments_and_duplicates(segments);

    std::vector<Crossing> crossings;
    if (cleanSegments.size() >= 2) {
        for (auto i = cleanSegments.begin(); i != std::prev(cleanSegments.end()); ++i) {
            for (auto j = std::next(i); j != cleanSegments.end(); ++j) {
                if (inner_intersect(*i, *j)) {
                    crossings.emplace_back(*i, *j);
                }
            }
        }
    }
    return crossings;
}
}  // namespace myers
