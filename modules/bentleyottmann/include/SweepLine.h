// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SweepLine_DEFINED
#define SweepLine_DEFINED

#include "modules/bentleyottmann/include/EventQueue.h"
#include "modules/bentleyottmann/include/Point.h"
#include "modules/bentleyottmann/include/Segment.h"

#include <limits>
#include <vector>

namespace bentleyottmann {

class SweepLine {
public:
    SweepLine();
    void handleEvent(Event e);

private:
    friend struct SweepLineTestingPeer;
    void verify(Point p) const;

    std::vector<Segment> fSweepLine;
};

}  // namespace bentleyottmann
#endif  // SweepLine_DEFINED
