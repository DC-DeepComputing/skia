// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/BentleyOttmann1.h"

#include "modules/bentleyottmann/include/EventQueue.h"

#include <optional>
#include <vector>

namespace bentleyottmann {
std::optional<std::vector<Point>> bentley_ottmann_1(SkSpan<const Segment> segments) {
    if (auto possibleEQ = EventQueue::Make(segments)) {
        EventQueue eventQueue = possibleEQ.value();
        while(eventQueue.hasMoreEvents()) {
            Event event = eventQueue.nextEvent();
        }

    }
    return std::nullopt;
}
}  // namespace bentleyottmann
