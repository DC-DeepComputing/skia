/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/graphite/geom/QuadTreeBoundsManager.h"

namespace skgpu::graphite {

DEF_TEST(BoundsManager, r) {
    // A height of 4 produces a root node 128x128, four nodes 64x64, sixteen nodes 32x32, and
    // sixty-four nodes 16x16.
    float nodeSizes[] = {128.f, 64.f, 32.f, 16.f};
    int h = 4;
    int l = 1 << (h - 1);
    std::unique_ptr<BoundsManager> bm = QuadTreeBoundsManager::MakeDepth({128, 128}, h);

    CompressedPaintersOrder order = CompressedPaintersOrder::First();
    for (int y = 0; y < l; ++y) {
        for (int x = 0; x < l; ++x) {
            order = order.next();
            Rect b = Rect::XYWH((x + 0.1f) * nodeSizes[h - 1], (y + 0.1f) * nodeSizes[h - 1],
                                0.8f * nodeSizes[h - 1], 0.8f * nodeSizes[h - 1]);
            bm->recordDraw(b, order, PaintersDepth::First(), false);
        }
    }

    // TODO: repeat these queries using bounds that intersect across levels as well
    order = CompressedPaintersOrder::First();
    for (int y = 0; y < l; ++y) {
        for (int x = 0; x < l; ++x) {
            order = order.next();
            Rect b = Rect::XYWH((x + 0.1f) * nodeSizes[h - 1], (y + 0.1f) * nodeSizes[h - 1],
                                0.8f * nodeSizes[h - 1], 0.8f * nodeSizes[h - 1]);

            CompressedPaintersOrder actual = bm->getMostRecentDraw(b);
            REPORTER_ASSERT(r, actual == order);
        }
    }

    bm = GridBoundsManager::Make({128, 128}, 8); // 64 nodes to match quad tree leaves

    order = CompressedPaintersOrder::First();
    for (int y = 0; y < l; ++y) {
        for (int x = 0; x < l; ++x) {
            order = order.next();
            Rect b = Rect::XYWH((x + 0.1f) * nodeSizes[h - 1], (y + 0.1f) * nodeSizes[h - 1],
                                0.8f * nodeSizes[h - 1], 0.8f * nodeSizes[h - 1]);
            bm->recordDraw(b, order, PaintersDepth::First(), false);
        }
    }

    // TODO: repeat these queries using bounds that intersect across levels as well
    order = CompressedPaintersOrder::First();
    for (int y = 0; y < l; ++y) {
        for (int x = 0; x < l; ++x) {
            order = order.next();
            Rect b = Rect::XYWH((x + 0.1f) * nodeSizes[h - 1], (y + 0.1f) * nodeSizes[h - 1],
                                0.8f * nodeSizes[h - 1], 0.8f * nodeSizes[h - 1]);

            CompressedPaintersOrder actual = bm->getMostRecentDraw(b);
            REPORTER_ASSERT(r, actual == order);
        }
    }

    // TODO: Then call recordDraw with new values that write to larger levels (either covering
    // or just intersecting).

    // TODO: Then test calls where the new value is not larger than the current max, and where the
    // new value is not larger than the current min.

    // FIXME: How valuable or possible is it to be able to handle returning a lower compressed
    // painters order when we know something was fully opaque?
    //    - it makes the int code stuff trickier for sure.
}

}  // namespace skgpu::graphite
