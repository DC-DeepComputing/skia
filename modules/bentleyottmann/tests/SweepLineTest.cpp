// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/SweepLine.h"

#include "modules/bentleyottmann/include/Point.h"

#include "tests/Test.h"

using namespace bentleyottmann;

namespace bentleyottmann {
struct SweepLineTestingPeer {
    static void VerifySweepLine(Point p, const SweepLine& sweepLine) {
        sweepLine.verify(p);
    }
};
}  // namespace bentleyottmann

DEF_TEST(BO_SweepLineBasic, reporter) {
    SweepLine sweepLine;
    SweepLineTestingPeer::VerifySweepLine({0, 0}, sweepLine);
}
