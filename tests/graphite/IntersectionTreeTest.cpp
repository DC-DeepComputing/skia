/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/geom/IntersectionTree.h"
#include "include/utils/SkRandom.h"
#include "tests/Test.h"

namespace skgpu {

class SimpleIntersectionTree {
public:
    bool add(Rect rect) {
        Rect::ComplementRect comp = rect;
        for (const Rect& r : fRects) {
            if (r.intersects(comp)) {
                return false;
            }
        }
        fRects.push_back(rect);
        return true;
    }

private:
    std::vector<Rect> fRects;
};

#define CHECK(A) REPORTER_ASSERT(reporter, A)

DEF_GRAPHITE_TEST(skgpu_IntersectionTree, reporter) {
    SkRandom rand;
    {
        SimpleIntersectionTree simpleTree;
        IntersectionTree tree;
        for (int i = 0; i < 1000; ++i) {
            Rect rect = Rect::XYWH(rand.nextRangeF(0, 500),
                                   rand.nextRangeF(0, 500),
                                   rand.nextRangeF(0, 70),
                                   rand.nextRangeF(0, 70));
            CHECK(tree.add(rect) == simpleTree.add(rect));
        }
    }
    {
        SimpleIntersectionTree simpleTree;
        IntersectionTree tree;
        for (int i = 0; i < 100; ++i) {
            Rect rect = Rect::XYWH(rand.nextRangeF(0, 500),
                                   rand.nextRangeF(0, 500),
                                   rand.nextRangeF(0, 200),
                                   rand.nextRangeF(0, 200));
            CHECK(tree.add(rect) == simpleTree.add(rect));
        }
    }
}

}  // namespace skgpu
