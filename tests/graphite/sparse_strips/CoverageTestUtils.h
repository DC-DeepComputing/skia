/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED
#define skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkString.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace skgpu::graphite::CoverageTestUtils {

// Simplified point-in-polygon verification using Even-Odd rule.
inline bool PointInPolygon(SkPoint pt, const Polyline& polyline) {
    bool inside = false;
    for (auto it = polyline.begin(); it != polyline.end(); ++it) {
        auto [line, idx] = *it;
        if ((line.p0.fY > pt.fY) != (line.p1.fY > pt.fY)) {
            float t = (pt.fY - line.p0.fY) / (line.p1.fY - line.p0.fY);
            float xInt = line.p0.fX + t * (line.p1.fX - line.p0.fX);
            if (pt.fX < xInt) {
                inside = !inside;
            }
        }
    }
    return inside;
}

// Checks if a point is within threshold distance to any line in polyline.
inline bool PointNearLine(SkPoint pt, const Polyline& polyline, float threshold) {
    for (auto it = polyline.begin(); it != polyline.end(); ++it) {
        auto [line, idx] = *it;
        float l2 = (line.p0.fX - line.p1.fX) * (line.p0.fX - line.p1.fX) +
                   (line.p0.fY - line.p1.fY) * (line.p0.fY - line.p1.fY);
        float dist = 0.0f;
        if (l2 == 0.0f) {
            float dx = pt.fX - line.p0.fX;
            float dy = pt.fY - line.p0.fY;
            dist = std::sqrt(dx * dx + dy * dy);
        } else {
            float t = std::max(0.0f,
                               std::min(1.0f,
                                        ((pt.fX - line.p0.fX) * (line.p1.fX - line.p0.fX) +
                                         (pt.fY - line.p0.fY) * (line.p1.fY - line.p0.fY)) /
                                                l2));
            float projX = line.p0.fX + t * (line.p1.fX - line.p0.fX);
            float projY = line.p0.fY + t * (line.p1.fY - line.p0.fY);
            dist = std::sqrt((pt.fX - projX) * (pt.fX - projX) +
                             (pt.fY - projY) * (pt.fY - projY));
        }
        if (dist <= threshold) return true;
    }
    return false;
}

// Prints ASCII map diagnostic comparing expected coverage against actual mask.
template <uint16_t kTileWidth, uint16_t kTileHeight>
inline void PrintTileDiagnostics(skiatest::Reporter* reporter,
                                 const Polyline& polyline,
                                 const Tiles<kTileWidth, kTileHeight>& tiler,
                                 int failX,
                                 int failY,
                                 const SkTDArray<uint8_t>& actualMasks,
                                 size_t tileStartIdx) {
    SkString out("\n--- FAILURE DIAGNOSTICS ---\n");

    out.append("Geometry Lines: {\n");
    for (auto it = polyline.begin(); it != polyline.end(); ++it) {
        auto [l, idx] = *it;
        out.appendf("  {{%f, %f}, {%f, %f}},\n", l.p0.fX, l.p0.fY, l.p1.fX, l.p1.fY);
    }
    out.append("}\n\n");

    out.append("Tile Intersections:\n");
    for (const auto& tile : tiler.getTiles()) {
        uint32_t mask = tile.intersectionMask();
        uint32_t lineIdx = tile.lineIdx();
        out.appendf("  Tile(%u,%u) Line %u Mask: %s\n",
                    tile.x,
                    tile.y,
                    lineIdx,
                    IntersectionBits::MaskToString(mask).c_str());
    }

    out.appendf("\nASCII Map Tile(%d,%d) [Left: Expected | Right: Actual]\n",
                failX / kTileWidth,
                failY / kTileHeight);
    SkString border("+");
    for (int x = 0; x < kTileWidth; ++x) border.append("----------------+");

    for (int y = 0; y < kTileHeight; ++y) {
        out.appendf("%s   %s\n", border.c_str(), border.c_str());
        for (int sy = 0; sy < 8; ++sy) {
            for (int side = 0; side < 2; ++side) {  // 0: Expected, 1: Actual
                out.append("|");
                for (int x = 0; x < kTileWidth; ++x) {
                    for (int sx = 0; sx < 8; ++sx) {
                        SkPoint cp = {failX + x + (sx + 0.5f) / 8.0f,
                                      failY + y + (sy + 0.5f) / 8.0f};
                        bool onLine = PointNearLine(cp, polyline, 0.6f / 8.0f);
                        bool active = false;

                        if (side == 0) {
                            active = PointInPolygon(cp, polyline);
                        } else {
                            int32_t bufIdx = tileStartIdx + (y * kTileWidth + x);
                            uint8_t mask =
                                    (bufIdx < actualMasks.size()) ? actualMasks[bufIdx] : 0;
                            active = (mask & (1 << sy));
                        }

                        if (MSAA_LUT<uint8_t>::kPattern[sy] == sx) {
                            out.append(active ? (onLine ? "*#" : " #")
                                              : (onLine ? "*o" : " o"));
                        } else {
                            out.append(onLine ? "**" : "  ");
                        }
                    }
                    out.append("|");
                }
                if (side == 0) out.append("   ");
            }
            out.append("\n");
        }
    }
    out.appendf("%s   %s\n", border.c_str(), border.c_str());
    INFOF(reporter, "%s", out.c_str());
}

}  // namespace skgpu::graphite::CoverageTestUtils

#endif  // skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED
