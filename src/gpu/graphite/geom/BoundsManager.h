/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_BoundsManager_DEFINED
#define skgpu_graphite_geom_BoundsManager_DEFINED

#include "include/core/SkSize.h"

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
        for (const Record& r : fRects.ritems()) {
            if (r.fOpaque && z < r.fZ && r.fBounds.contains(bounds)) {
                return true;
            }
        }
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

    static std::unique_ptr<GridBoundsManager> Make(const SkISize& deviceSize,
                                                   const SkISize& gridSize) {
        SkASSERT(deviceSize.width() > 0 && deviceSize.height() > 0);
        SkASSERT(gridSize.width() >= 1 && gridSize.height() >= 1);

        size_t alloc = sizeof(GridBoundsManager) +
                       sizeof(CompressedPaintersOrder) * gridSize.width() * gridSize.height();
        void* gridData = ::operator new(alloc);
        return std::unique_ptr<GridBoundsManager>(
                new (gridData) GridBoundsManager(deviceSize, gridSize));
    }

    static std::unique_ptr<GridBoundsManager> Make(const SkISize& deviceSize, int gridSize) {
        return Make(deviceSize, {gridSize, gridSize});
    }

    static std::unique_ptr<GridBoundsManager> MakeRes(const SkISize& deviceSize, int gridCellSize) {
        SkASSERT(deviceSize.width() > 0 && deviceSize.height() > 0);
        SkASSERT(gridCellSize >= 1);

        int gridWidth = SkScalarCeilToInt(deviceSize.width() / (float) gridCellSize);
        int gridHeight = SkScalarCeilToInt(deviceSize.height() / (float) gridCellSize);

        // This keeps the grid cells exactly at the requested resolution, but pads the right and
        // bottom edges out to a multiple of the cell size. The alternative is pass in the unpadded
        // device size, which would mean the actual cell size will be smaller than the requested
        // (by (deviceSize % gridCellSize)/gridDims).
        SkISize paddedDeviceSize = {gridWidth * gridCellSize, gridHeight * gridCellSize};
        return Make(paddedDeviceSize, {gridWidth, gridHeight});
    }

    ~GridBoundsManager() override {
        // If this isn't true, then relying solely on ::operator delete won't work to clean up
        // the tree's nodes.
        static_assert(std::is_trivially_destructible<CompressedPaintersOrder>::value);
    }
    void operator delete(void* p) { ::operator delete(p); }


    CompressedPaintersOrder getMostRecentDraw(const Rect& bounds) const override {
        SkASSERT(!bounds.isEmptyNegativeOrNaN());

        auto ltrb = this->getGridCoords(bounds);
        const CompressedPaintersOrder* p = this->nodes() + ltrb[1] * fGridWidth + ltrb[0];
        int h = ltrb[3] - ltrb[1];
        int w = ltrb[2] - ltrb[0];

        CompressedPaintersOrder max = CompressedPaintersOrder::First();
        for (int y = 0; y <= h; ++y ) {
            for (int x = 0; x <= w; ++x) {
                CompressedPaintersOrder v = *(p + x);
                if (v > max) {
                    max = v;
                }
            }
            p = p + fGridWidth;
        }

        return max;
    }

    bool isOccluded(const Rect& bounds, PaintersDepth z) const override {
        // Not implemented, occlusion culling may not remain responsibility of BoundsManager
        return false;
    }

    void recordDraw(const Rect& bounds,
                    CompressedPaintersOrder order,
                    // ignored since they are for occlusion culling
                    PaintersDepth, bool) override {
        SkASSERT(!bounds.isEmptyNegativeOrNaN());

        auto ltrb = this->getGridCoords(bounds);
        CompressedPaintersOrder* p = this->nodes() + ltrb[1] * fGridWidth + ltrb[0];
        int h = ltrb[3] - ltrb[1];
        int w = ltrb[2] - ltrb[0];

        for (int y = 0; y <= h; ++y) {
            for (int x = 0; x <= w; ++x) {
                CompressedPaintersOrder v = *(p + x);
                if (order > v) {
                    *(p + x) = order;
                }
            }
            p = p + fGridWidth;
        }
    }

    void reset() override {
        memset(this->nodes(), 0, sizeof(CompressedPaintersOrder) * fGridWidth * fGridHeight);
    }

private:
    GridBoundsManager(const SkISize& deviceSize, const SkISize& gridSize)
            : fScaleX(gridSize.width() / (float) deviceSize.width())
            , fScaleY(gridSize.height() / (float) deviceSize.height())
            , fGridWidth(gridSize.width())
            , fGridHeight(gridSize.height()) {
        this->reset();
    }

    skvx::int4 getGridCoords(const Rect& bounds) const {
        // Normalize bounds by 1/wh of device bounds, then scale up to number of cells per side.
        // fScaleXY includes both 1/wh and the grid dimension scaling, then clamp to [0, gridDim-1].
        return pin(skvx::cast<int32_t>(bounds.ltrb() * skvx::float2(fScaleX, fScaleY).xyxy()),
                   skvx::int4(0),
                   skvx::int2(fGridWidth, fGridHeight).xyxy() - 1);
    }

    // fGridWidth*fGridHeight values allocated immediately after the GridBoundsManager
    CompressedPaintersOrder* nodes() {
        return reinterpret_cast<CompressedPaintersOrder*>(
                reinterpret_cast<uintptr_t>(this) + sizeof(GridBoundsManager));
    }
    const CompressedPaintersOrder* nodes() const {
        return reinterpret_cast<const CompressedPaintersOrder*>(
                reinterpret_cast<uintptr_t>(this) + sizeof(GridBoundsManager));
    }

    const float fScaleX;
    const float fScaleY;

    const int   fGridWidth;
    const int   fGridHeight;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_BoundsManager_DEFINED
