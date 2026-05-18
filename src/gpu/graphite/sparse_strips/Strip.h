/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_Strip_DEFINED
#define skgpu_graphite_sparse_strips_Strip_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkVx.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>

namespace skgpu::graphite {

#if defined(GPU_TEST_UTILS)
using MsaaExactMaskObserver = std::function<void(uint8_t exactMask)>;
#endif

class Strip : IntersectionBits {
public:
    Strip(uint16_t x, uint16_t y, uint32_t alphaIdx, bool shouldFill)
            : x(x)
            , y(y)
            , fFillAlphaIdx(alphaIdx | (static_cast<uint32_t>(shouldFill) << FILL_SHIFT)) {}

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void MakeStripsMsaaScalar(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                                     SkTDArray<Strip>& stripBuf,
                                     SkTDArray<uint8_t>& alphaBuf,
                                     SkPathFillType fillType,
                                     const Polyline& polyline,
                                     const SkTDArray<uint8_t>& maskLut
#if defined(GPU_TEST_UTILS)
                                     , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            MakeStripsScalarImpl<kTileWidth, kTileHeight, kIsWinding>(tileContainer,
                                                                    stripBuf,
                                                                    alphaBuf,
                                                                    isInverse,
                                                                    polyline,
                                                                    maskLut
#if defined(GPU_TEST_UTILS)
                                                                    , observer
#endif
            );
        });
    }

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static void MakeStripsMsaaSimd(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                                   SkTDArray<Strip>& stripBuf,
                                   SkTDArray<uint8_t>& alphaBuf,
                                   SkPathFillType fillType,
                                   const Polyline& polyline,
                                   const SkTDArray<uint8_t>& maskLut
#if defined(GPU_TEST_UTILS)
                                   , MsaaExactMaskObserver observer = nullptr
#endif
    ) {
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            MakeStripsSimdImpl<kTileWidth, kTileHeight, kIsWinding>(tileContainer,
                                                                    stripBuf,
                                                                    alphaBuf,
                                                                    isInverse,
                                                                    polyline,
                                                                    maskLut
#if defined(GPU_TEST_UTILS)
                                                                    , observer
#endif
            );
        });
    }

private:
#if defined(GPU_TEST_UTILS)
    template <uint16_t, uint16_t> friend class ClipTestRunner;
    template <uint16_t, uint16_t> friend class CoverageTestRunner;
#endif
    template <uint16_t, uint16_t> friend class StripGenerator;

    template <typename F> static void Dispatch(SkPathFillType fillType, F&& f) {
        switch (fillType) {
            case SkPathFillType::kWinding:
                f(std::bool_constant<true>{}, /*isInverse=*/false);
                return;
            case SkPathFillType::kInverseWinding:
                f(std::bool_constant<true>{}, /*isInverse=*/true);
                return;
            case SkPathFillType::kEvenOdd:
                f(std::bool_constant<false>{}, /*isInverse=*/false);
                return;
            case SkPathFillType::kInverseEvenOdd:
                f(std::bool_constant<false>{}, /*isInverse=*/true);
                return;
        }
        SkUNREACHABLE;
    }

    template <uint16_t TileWidth, uint16_t TileHeight, bool kIsWinding>
    static void MakeStripsScalarImpl(const Tiles<TileWidth, TileHeight>& tileContainer,
                                     SkTDArray<Strip>& stripBuf,
                                     SkTDArray<uint8_t>& alphaBuf,
                                     bool isInverse,
                                     const Polyline& polyline,
                                     const SkTDArray<uint8_t>& maskLut
#if defined(GPU_TEST_UTILS)
                                     , MsaaExactMaskObserver observer = nullptr
#endif
    );

    template <uint16_t TileWidth, uint16_t TileHeight, bool kIsWinding>
    static void MakeStripsSimdImpl(const Tiles<TileWidth, TileHeight>& tileContainer,
                                   SkTDArray<Strip>& stripBuf,
                                   SkTDArray<uint8_t>& alphaBuf,
                                   bool isInverse,
                                   const Polyline& polyline,
                                   const SkTDArray<uint8_t>& maskLut
#if defined(GPU_TEST_UTILS)
                                   , MsaaExactMaskObserver observer = nullptr
#endif
    );

    static constexpr uint32_t FILL_SHIFT = 31;

    static constexpr int kNumSubSamples  = 8;
    static constexpr float kPackingScale = 0.5f;
    static constexpr int kMaskWidth      = 64;
    static constexpr int kMaskHeight     = 64;
    static constexpr float kMaskWidthF   = static_cast<float>(kMaskWidth);
    static constexpr float kMaskHeightF  = static_cast<float>(kMaskHeight);
    static constexpr int kMaskWidthExcl  = kMaskWidth - 1;
    static constexpr int kMaskHeightExcl = kMaskHeight - 1;

    static constexpr float kStripEpsilon = 1e-5f;

    // Responsible for consuming the tile coordinates and intersection mask produced by the Tiler
    // and producing the exact points at which the parent line intersects the edges of the tile.
    // It also safely handles when there is only one edge intersection, either from the line ending
    // inside the tile body, or a horizontal edge touch.
    //
    // This function provides the following guarantees:
    //  1) Y-Sorting: The resulting points are always sorted top-to-bottom.
    //  2) Interior Endpoints: If a line ends inside the tile, its exact endpoint is preserved and
    //     returned.
    //  3) Zero-Width Degenerates: A single-point intersection (e.g., a horizontal graze or corner
    //     touch) produces a second point along that exact same edge, yielding safe, zero-width
    //     geometry.
    //  4) Boundary Snapping: Edge intersections are explicitly snapped to the exact bounding
    //     coordinate of the tile.
    //
    // (Note: Guarantees 2 and 4 ensure downstream logic can safely rely on floating-point equality
    // checks).
    //
    // Note, this function is tightly coupled to Tiler and MakeStrips. If the incoming tile
    // coordinates are incorrect, the results from this function are meaningless. It assumes that:
    //  1) Derivatives resulting from division by zero (e.g. perfectly vertical or horizontal lines)
    //     will be set to zero.
    //  2) Only two bits are ever set in the mask. (Expects that perfect corner touches are tie
    //     broken by the Tiler.)
    static SK_ALWAYS_INLINE std::array<SkPoint, 2> ClipToTile(
        const Line& line,
        const std::array<float, 4>& bounds,
        const std::array<float, 4>& derivatives,
        uint32_t intersectionMask,
        bool canonicalXDir,
        bool canonicalYDir) {
        SkPoint pEntry = line.p0;
        SkPoint pExit  = line.p1;

        const float tileMinX = bounds[0];
        const float tileMinY = bounds[1];
        const float tileMaxX = bounds[2];
        const float tileMaxY = bounds[3];

        const float dx = derivatives[0];
        const float dy = derivatives[1];

        // A line's direction dictates which edges it can cross from the outside-in (entry) versus
        // inside-out (exit). For example, if dx > 0 (canonicalXDir = true), the vector is monotonic
        // in the positive X direction. It is impossible to intersect the right edge as an entry
        // point, or the left edge as an exit point. This reduces the number of edges which must be
        // checked from four to two candidate entry and two candidate exit edges.
        uint32_t maskVIn, maskVOut;
        float boundVIn, boundVOut;
        if (canonicalXDir) {
            maskVIn  = L;
            boundVIn = tileMinX;
            maskVOut = R;
            boundVOut= tileMaxX;
        } else {
            maskVIn  = R;
            boundVIn = tileMaxX;
            maskVOut = L;
            boundVOut= tileMinX;
        }

        uint32_t maskHIn, maskHOut;
        float boundHIn, boundHOut;
        if (canonicalYDir) {
            maskHIn  = T;
            boundHIn = tileMinY;
            maskHOut = B;
            boundHOut= tileMaxY;
        } else {
            maskHIn  = B;
            boundHIn = tileMaxY;
            maskHOut = T;
            boundHOut= tileMinY;
        }

        const float idx = derivatives[2];
        const float idy = derivatives[3];

        // Check the candidate edges against the intersection mask
        const uint32_t entryHits = intersectionMask & (maskVIn | maskHIn);
        if (entryHits != 0) {
            // Since the Tiler guarantees strict tie-breaking upstream (never outputting conflicting
            // entry bits for a perfect corner hit), we can safely isolate the specific entry axis.
            const bool useH = (intersectionMask & maskHIn) != 0;

            const float bound = useH ? boundHIn   : boundVIn;
            const float start = useH ? line.p0.fY : line.p0.fX;
            const float invD  = useH ? idy        : idx;

            // Evaluate the parametric equation t = (bound - start) * (1 / delta)
            const float t = (bound - start) * invD;

            pEntry.fX = line.p0.fX + t * dx;
            pEntry.fY = line.p0.fY + t * dy;

            // Explicitly snap the specific axis to the bound to eliminate floating point drift
            if (useH) {
                pEntry.fY = bound;
            } else {
                pEntry.fX = bound;
            }
        }

        // Repeat for exit
        const uint32_t exitHits = intersectionMask & (maskVOut | maskHOut);
        if (exitHits != 0) {
            const bool useH = (intersectionMask & maskHOut) != 0;

            const float bound = useH ? boundHOut   : boundVOut;
            const float start = useH ? line.p0.fY  : line.p0.fX;
            const float invD  = useH ? idy         : idx;

            const float t = (bound - start) * invD;

            pExit.fX = line.p0.fX + t * dx;
            pExit.fY = line.p0.fY + t * dy;

            // Explicitly snap the specific axis to the bound
            if (useH) {
                pExit.fY = bound;
            } else {
                pExit.fX = bound;
            }
        }

        // Guarantee predictable winding order for downstream rasterization stages.
        std::array<SkPoint, 2> result;
        if (pExit.fY >= pEntry.fY) {
            result = {pEntry, pExit};
        } else {
            result = {pExit, pEntry};
        }

        // Shift from global viewport coordinates into tile-local space.
        result[0].fX -= tileMinX;
        result[0].fY -= tileMinY;
        result[1].fX -= tileMinX;
        result[1].fY -= tileMinY;

        // Clamping serves three functions here:
        //  1) Floating-Point Correction: Points which are slightly outside the tile due to floating
        //     point error are coerced inside.
        //  2) Confinement: Upstream logic sets reciprocal derivatives to 0 for perfectly horizontal
        //     or vertical lines. This causes the parametric math above to return the original
        //     unclipped coordinate. Clamping confines this "free" axis to the tile boundaries.
        //  3) Zero-Width Degenerates: If there is only a single edge crossing (e.g., a tie-broken
        //     corner hit or an inclusive touch), the opposing uncalculated endpoint is forced onto
        //     that same edge. This flattens the segment into zero-width geometry that the
        //     downstream coverage calculation will safely ignore.
        const float width  = tileMaxX - tileMinX;
        const float height = tileMaxY - tileMinY;

        result[0].fX = std::clamp(result[0].fX, 0.0f, width);
        result[0].fY = std::clamp(result[0].fY, 0.0f, height);
        result[1].fX = std::clamp(result[1].fX, 0.0f, width);
        result[1].fY = std::clamp(result[1].fY, 0.0f, height);

        return result;
    }

    uint16_t x;
    uint16_t y;
    uint32_t fFillAlphaIdx;
};

} // namespace skgpu::graphite

#include "src/gpu/graphite/sparse_strips/MakeStripsScalar_impl.h"
#include "src/gpu/graphite/sparse_strips/MakeStripsSimd_impl.h"

#endif // skgpu_graphite_sparse_strips_Strip_DEFINED
