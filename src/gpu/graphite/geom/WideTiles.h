/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_WideTiles_DEFINED
#define skgpu_graphite_geom_WideTiles_DEFINED

#include "src/gpu/graphite/geom/Rect.h"
#include <cstdint>
#include <vector>

namespace skgpu::graphite {

template <uint16_t kTileHeight>
class WideTiles {
public:
    struct WideTile {
        WideTile(uint16_t x, uint16_t y, uint16_t width)
            : fX(x), fY(y), fWidth(width), fPadding(0xffff) {}
        uint16_t fX;
        uint16_t fY;
        uint16_t fWidth;
        uint16_t fPadding;
    };

    WideTiles() = default;

    void addTile(uint16_t x, uint16_t y, uint16_t width) {
        fTiles.push_back(WideTile(x, y, width));
    }

    const std::vector<WideTile>& tiles() const { return fTiles; }
    bool empty() const { return fTiles.empty(); }
    size_t size() const { return fTiles.size(); }
    void clear() { fTiles.clear(); }

private:
    std::vector<WideTile> fTiles;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_WideTiles_DEFINED
