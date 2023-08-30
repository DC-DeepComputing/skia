// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/SweepLine.h"

#include "modules/bentleyottmann/include/EventQueue.h"

#include <algorithm>

namespace bentleyottmann {

SweepLine::SweepLine() {
    static constexpr Segment LeftSentinel = {{-std::numeric_limits<int32_t>::max(),
                                                     -std::numeric_limits<int32_t>::max()},
                                             {-std::numeric_limits<int32_t>::max(),
                                                     std::numeric_limits<int32_t>::max()}};
    static constexpr Segment RightSentinel = {{std::numeric_limits<int32_t>::max(),
                                                      -std::numeric_limits<int32_t>::max()},
                                              {std::numeric_limits<int32_t>::max(),
                                                      std::numeric_limits<int32_t>::max()}};

    fSweepLine.reserve(8);
    fSweepLine.push_back(LeftSentinel);
    fSweepLine.push_back(RightSentinel);
}

std::optional<Event> possible_intersect(const Segment& s0, const Segment& s1) {
    if (auto intersectionOpt = intersect(s0, s1)) {
        const Point intersection = intersectionOpt.value();
        return Event{intersection, Cross{s0, s1}};
    }
    return std::nullopt;
}

void SweepLine::verify(int32_t y) const {
    for(auto cursor = fSweepLine.begin(); (cursor + 1) != fSweepLine.end(); ++cursor) {
        [[maybe_unused]] const Segment& left = *cursor;
        [[maybe_unused]] const Segment& right = *(cursor + 1);
        SkASSERT(lessThanAt(left, right, y));
    }
}

void
SweepLine::handle(Point eventPoint,
                  const EventQueue::SegmentSet& removing,
                  const EventQueue::SlopeSegmentSet& inserting,
                  EventQueue* queue) {

    // Remove all ending and crossing segments.
    auto toRemove = [eventPoint, removing](Segment s) {
        return s.lower() == eventPoint || removing.find(s) != removing.end();
    };
    auto endCursor = std::remove_if(fSweepLine.begin(), fSweepLine.end(), toRemove);
    fSweepLine.erase(endCursor, fSweepLine.end());

    // Reinsert the crossing and insert starting segments. The SlopSegmentSet makes sure that
    // these segments are in the right order for insertion.
    auto comp = [](const Segment& s, Point p) {
        return !pointLessThanSegment(p, s);
    };
    auto sweepLineInsertionCursor = std::lower_bound(
            fSweepLine.begin(), fSweepLine.end(), eventPoint, comp);

    auto leftOfInsertion = sweepLineInsertionCursor - 1;
    auto rightOfInsertion = sweepLineInsertionCursor;

    // Check if the left most inserted segment crosses the segment immediately to the left of the
    // insertion cursor.
    if (auto event = possible_intersect(*leftOfInsertion, *inserting.begin())) {
        queue->add((event.value()));
        queue->addCrossing(event.value());
    }

    // Check if the right most inserted segment crosses the segment immediately to the right of
    // the insertion cursor.
    if (auto event = possible_intersect(*inserting.rbegin(), *rightOfInsertion)) {
        queue->add((event.value()));
        queue->addCrossing(event.value());
    }

    // Insert the set in the sweep line.
    fSweepLine.insert(sweepLineInsertionCursor, inserting.begin(), inserting.end());
}
}  // namespace bentleyottmann
