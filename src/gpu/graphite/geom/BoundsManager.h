/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_BoundsManager_DEFINED
#define skgpu_graphite_geom_BoundsManager_DEFINED

#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/geom/Rect.h"

#include "src/core/SkTBlockList.h"

#include <cstdint>

namespace skgpu::graphite {

/**
 * BoundsManager is an acceleration structure for device-space related pixel bounds queries.
 * The BoundsManager relies on two related ordinal values: the CompressedPaintersOrder of a draw
 * and the Z/depth value of the draw. The CompressedPaintersOrder enforces a specific submission
 * order of draws to the GPU but can re-arrange draws out of their original painter's order if the
 * GREATER depth test and the draw's Z value resolve out-of-order rendering.
 *
 * It supports querying the most recent draw intersecting a bounding rect (represented as a
 * CompressedPaintersOrder value), recording a (bounds, CompressedPaintersOrder, and Z value) tuple,
 * and querying if a (bounds, Z value) pair is fully occluded by another draw.
 */
class BoundsManager {
public:
    virtual ~BoundsManager() {}

    virtual CompressedPaintersOrder getMostRecentDraw(const Rect& bounds) const = 0;

    virtual bool isOccluded(const Rect& bounds, PaintersDepth z) const = 0;

    virtual void recordDraw(const Rect& bounds,
                            CompressedPaintersOrder order,
                            PaintersDepth z,
                            bool fullyOpaque=false) = 0;

    virtual void reset() = 0;
};

// TODO: Select one most-effective BoundsManager implementation, make it the only option, and remove
// virtual-ness. For now, this seems useful for correctness testing by comparing against trivial
// implementations and for identifying how much "smarts" are actually worthwhile.

// A BoundsManager that produces exact painter's order and assumes nothing is occluded.
class NaiveBoundsManager final : public BoundsManager {
public:
    ~NaiveBoundsManager() override {}

    CompressedPaintersOrder getMostRecentDraw(const Rect& bounds) const override {
        return fLatestDraw;
    }

    bool isOccluded(const Rect& bounds, PaintersDepth z) const override { return false; }

    void recordDraw(const Rect& bounds, CompressedPaintersOrder order, PaintersDepth z,
                    bool fullyOpaque=false) override {
        if (fLatestDraw < order) {
            fLatestDraw = order;
        }
    }

    void reset() override {
        fLatestDraw = CompressedPaintersOrder::First();
    }

private:
    CompressedPaintersOrder fLatestDraw = CompressedPaintersOrder::First();
};

// A BoundsManager that tracks every draw and can exactly determine all queries
// using a brute force search.
class BruteForceBoundsManager : public BoundsManager {
public:
    ~BruteForceBoundsManager() override {}

    CompressedPaintersOrder getMostRecentDraw(const Rect& bounds) const override {
        CompressedPaintersOrder max = CompressedPaintersOrder::First();
        for (const Record& r : fRects.items()) {
            if (max < r.fOrder && r.fBounds.intersects(bounds)) {
                max = r.fOrder;
            }
        }
        return max;
    }

    bool isOccluded(const Rect& bounds, PaintersDepth z) const override {
        // Iterate in reverse since the records were likely recorded in increasing Z
        // for (const Record& r : fRects.ritems()) {
        //     if (r.fOpaque && z < r.fZ && r.fBounds.contains(bounds)) {
        //         return true;
        //     }
        // }
        return false;
    }

    void recordDraw(const Rect& bounds, CompressedPaintersOrder order, PaintersDepth z,
                    bool fullyOpaque=false) override {
        fRects.push_back({bounds, order, z, fullyOpaque});
    }

    void reset() override {
        fRects.reset();
    }

private:
    struct Record {
        Rect fBounds;
        CompressedPaintersOrder fOrder;
        PaintersDepth fZ;
        bool fOpaque;
    };

    SkTBlockList<Record> fRects{16, SkBlockAllocator::GrowthPolicy::kFibonacci};
};

class GridBoundsManager : public BoundsManager {
public:

    static std::unique_ptr<GridBoundsManager> Make(const SkISize& deviceBounds,
                                                   int numSides) {
        SkASSERT(deviceBounds.width() > 0 && deviceBounds.height() > 0);
        SkASSERT(numSides > 0);
        size_t alloc = sizeof(GridBoundsManager) + sizeof(CompressedPaintersOrder) * numSides * numSides;
        void* quadTree = ::operator new(alloc);
        return std::unique_ptr<GridBoundsManager>(new (quadTree) GridBoundsManager(deviceBounds, numSides));
    }

    ~GridBoundsManager() override {
        // If this isn't true, then relying solely on ::operator delete won't work to clean up
        // the tree's nodes.
        static_assert(std::is_trivially_destructible<CompressedPaintersOrder>::value);
    }
    void operator delete(void* p) { ::operator delete(p); }


    CompressedPaintersOrder getMostRecentDraw(const Rect& bounds) const override {
        Rect clampedBounds = bounds.makeIntersect(Rect::WH(fWidth, fHeight));
        if (clampedBounds.isEmptyNegativeOrNaN()) {
            return CompressedPaintersOrder::First();
        }

        auto ltrb = this->getRectCells(clampedBounds);

        int w = ltrb[2] - ltrb[0];
        int h = ltrb[3] - ltrb[1];
        CompressedPaintersOrder* p = fNodes + fNumSides * ltrb[1] + ltrb[0];

        CompressedPaintersOrder max = CompressedPaintersOrder::First();
        for (int y = 0; y <= h; ++y) {
            for (int x = 0; x <= w; ++x) {
                CompressedPaintersOrder v = *(p + x);
                if (v > max) {
                    max = v;
                }
            }
            p += fNumSides;
        }

        return max;
    }

    bool isOccluded(const Rect& bounds, PaintersDepth z) const override {
        return false;
    }

    void recordDraw(const Rect& bounds, CompressedPaintersOrder order, PaintersDepth z,
                    bool fullyOpaque=false) override {
        Rect clampedBounds = bounds.makeIntersect(Rect::WH(fWidth, fHeight));
        if (clampedBounds.isEmptyNegativeOrNaN()) {
            return;
        }

        auto ltrb = this->getRectCells(clampedBounds);

        int w = ltrb[2] - ltrb[0];
        int h = ltrb[3] - ltrb[1];

        CompressedPaintersOrder* p = fNodes + fNumSides * ltrb[1] + ltrb[0];
        for (int y = 0; y <= h; ++y) {
            for (int x = 0; x <= w; ++x) {
                if (*(p + x) < order) {
                    *(p + x) = order;
                }
            }
            p += fNumSides;
        }
    }

    void reset() override {
        memset(fNodes, 0, sizeof(CompressedPaintersOrder) * fNumSides * fNumSides);
    }

private:

    GridBoundsManager(const SkISize& deviceBounds, int numSides)
            : fWidth(deviceBounds.width())
            , fHeight(deviceBounds.height())
            , fScaleX(numSides / fWidth)
            , fScaleY(numSides / fHeight)
            , fNumSides(numSides)
            , fNodes(reinterpret_cast<CompressedPaintersOrder*>(reinterpret_cast<uintptr_t>(this) + sizeof(GridBoundsManager))) {
        this->reset();
    }

    skvx::int4 getRectCells(const Rect& bounds) const {
        // Normalize bounds by 1/wh of device bounds, then scale up to 2^(D-1)-1 and round out.
        // fScaleXY includes both 1/wh and the 2^(D-1)-1 scale factor. bounds should already have
        // been intersected with device bounds (and have a non-empty intersection).
        SkASSERT(!bounds.isEmptyNegativeOrNaN() && Rect::WH(fWidth, fHeight).contains(bounds));
        // These should fit into uint16_t, but we are immediately interleaving bits into two uint32s
        // so keep in the wider type with high bits being 0.
        SkASSERT((bounds.right() * fScaleX) <= fNumSides &&
                 (bounds.bot() * fScaleY) <= fNumSides);

        return min(skvx::cast<int32_t>(bounds.ltrb() * skvx::float2(fScaleX, fScaleY).xyxy()),
                   skvx::int4(fNumSides - 1));
    }

    const float fWidth;
    const float fHeight;
    const float fScaleX;
    const float fScaleY;
    const int   fNumSides;

    CompressedPaintersOrder* fNodes;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_BoundsManager_DEFINED
