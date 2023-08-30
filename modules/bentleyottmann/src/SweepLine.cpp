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

template<class... Ts>
struct Visitor : Ts... { using Ts::operator()...; };
template<class... Ts>
Visitor(Ts...) -> Visitor<Ts...>;

std::optional<Event> possible_intersect(const Segment& s0, const Segment& s1) {
    if (auto intersectionOpt = intersect(s0, s1)) {
        const Point intersection = intersectionOpt.value();
        return Event{intersection, Cross{s0, s1}};
    }
    return std::nullopt;
}

void SweepLine::handleEvent(const Event& event, EventQueue* queue) {

    const Point& where = event.where;

    auto handleLower = [this, &where, queue](const Lower& l) {
        auto matchLower = [&](const Segment& segment){
            return segment.lower() == where;
        };

        auto segment = std::find_if(fSweepLine.begin(), fSweepLine.end(), matchLower);
        auto left = segment - 1;
        auto right = segment + 1;
        if (auto event = possible_intersect(*left, *right)) {
            queue->add(event.value());
        }
        fSweepLine.erase(segment);
    };

    auto handleCross = [](const Cross& c) {
        SK_ABORT("Not implemented");
    };

    auto handleUpper = [](const Upper& u) {
        SK_ABORT("Not implemented");
    };

    Visitor visitor{handleLower, handleCross, handleUpper};

    std::visit(visitor, event.type);
}

void SweepLine::verify(int32_t y) const {
    for(auto cursor = fSweepLine.begin(); (cursor + 1) != fSweepLine.end(); ++cursor) {
        [[maybe_unused]] const Segment& left = *cursor;
        [[maybe_unused]] const Segment& right = *(cursor + 1);
        SkASSERT(lessThanAt(left, right, y));
    }
}
}  // namespace bentleyottmann
