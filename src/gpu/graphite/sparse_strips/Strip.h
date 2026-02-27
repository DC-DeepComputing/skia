/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_Strip_DEFINED
#define skgpu_graphite_Strip_DEFINED

#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace skgpu::graphite {

class Strip : IntersectionBits {
public:
    Strip() {}

    static SK_ALWAYS_INLINE std::array<Point, 2> clipToTile(
        const Line& line,
        const std::array<float, 4>& bounds,
        const std::array<float, 4>& derivatives,
        uint32_t intersectionData,
        bool cannonicalXDir,
        bool cannonicalYDir
    ) {
        Point pEntry = line.p0;
        Point pExit  = line.p1;

        const float tileMinX = bounds[0];
        const float tileMinY = bounds[1];
        const float tileMaxX = bounds[2];
        const float tileMaxY = bounds[3];

        const float dx = derivatives[0];
        const float dy = derivatives[1];

        uint32_t maskVIn, maskVOut;
        float boundVIn, boundVOut;
        if (cannonicalXDir) {
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
        if (cannonicalYDir) {
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

        const uint32_t entryHits = intersectionData & (maskVIn | maskHIn);
        if (entryHits != 0) {
            const bool useH = (intersectionData & maskHIn) != 0;

            const float bound = useH ? boundHIn : boundVIn;
            const float start = useH ? line.p0.y  : line.p0.x;
            const float invD  = useH ? idy        : idx;

            const float t = (bound - start) * invD;

            pEntry.x = line.p0.x + t * dx;
            pEntry.y = line.p0.y + t * dy;

            // Explicitly snap the specific axis to the bound
            if (useH) {
                pEntry.y = bound;
            } else {
                pEntry.x = bound;
            }
        }

        const uint32_t exitHits = intersectionData & (maskVOut | maskHOut);
        if (exitHits != 0) {
            const bool useH = (intersectionData & maskHOut) != 0;

            const float bound = useH ? boundHOut : boundVOut;
            const float start = useH ? line.p0.y   : line.p0.x;
            const float invD  = useH ? idy         : idx;

            const float t = (bound - start) * invD;

            pExit.x = line.p0.x + t * dx;
            pExit.y = line.p0.y + t * dy;

            // Explicitly snap the specific axis to the bound
            if (useH) {
                pExit.y = bound;
            } else {
                pExit.x = bound;
            }
        }

        std::array<Point, 2> result;
        if (pExit.y >= pEntry.y) {
            result = {pEntry, pExit};
        } else {
            result = {pExit, pEntry};
        }

        result[0].x -= tileMinX;
        result[0].y -= tileMinY;
        result[1].x -= tileMinX;
        result[1].y -= tileMinY;

        // Clamping serves three purposes here:
        // 1) Points which are slightly outside the tile due to floating point error are coerced
        //    inside.
        // 2) More subtly, perfectly horizontal or vertical lines have their reciprocal derivatives
        //    set to 0 in the upstream strip logic. This causes the intersection calculation to
        //    return the original coordinate. While the coordinate fixed to the tile edge is
        //    explicitly set (and guaranteed valid), clamping forces the coordinate along that edge
        //    to be in bounds and watertight.
        // 3) In the case that there is only a single edge crossing (either from a corner touch
        //    tiebroken to a single dimension or an inclusive touch on the vertical edge of a tile),
        //    the opposing point must be somewhere in the direction normal to the edge. Clamping
        //    forces the segment to snap onto the tile edge. This creates zero-width geometry which
        //    gets discarded by downstream pixel filling logic.
        const float width  = tileMaxX - tileMinX;
        const float height = tileMaxY - tileMinY;

        result[0].x = std::clamp(result[0].x, 0.0f, width);
        result[0].y = std::clamp(result[0].y, 0.0f, height);
        result[1].x = std::clamp(result[1].x, 0.0f, width);
        result[1].y = std::clamp(result[1].y, 0.0f, height);

        return result;
    }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Strip_DEFINED
