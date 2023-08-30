// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/SweepLine.h"

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

void SweepLine::handleEvent(bentleyottmann::Event e) {
}

void SweepLine::verify(Point p) const {
    for(auto cursor = fSweepLine.begin(); (cursor + 1) != fSweepLine.end(); ++cursor) {
        SkASSERT(lessAt(p.y, *cursor, *(cursor + 1)));
    }
}
}  // namespace bentleyottmann
