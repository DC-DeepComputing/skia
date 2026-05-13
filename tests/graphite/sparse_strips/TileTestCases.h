/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TileTestCases_DEFINED
#define skgpu_graphite_TileTestCases_DEFINED

#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <initializer_list>
#include <vector>

namespace skgpu::graphite {

namespace TileTestCases {

    struct TileTestCase {
        TileTestCase(const char* n,
                    std::initializer_list<Line> unscaledLines,
                    std::initializer_list<Tile> expectedTiles,
                    float scale)
                : fName(n), fExpected(expectedTiles) {
            fLines.reserve(unscaledLines.size());
            for (const auto& line : unscaledLines) {
                fLines.push_back({{line.p0.fX * scale, line.p0.fY * scale},
                                  {line.p1.fX * scale, line.p1.fY * scale}});
            }
        }
        const char* fName;
        SkTDArray<Line> fLines;
        SkTDArray<Tile> fExpected;
    };

    std::vector<TileTestCase> Get(float scale, uint16_t viewportDim);
} // namespace TileTestCases

} // namespace skgpu::graphite

#endif // skgpu_graphite_TileTestCases_DEFINED
