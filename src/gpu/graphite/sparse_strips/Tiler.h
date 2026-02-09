/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Tiler_DEFINED
#define skgpu_graphite_Tiler_DEFINED

#include "include/private/base/SkAssert.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"

#include <vector>
#include <cstdint>
#include <algorithm>

namespace skgpu::graphite {

struct Tile {
    static constexpr uint32_t T = 0b00001;
    static constexpr uint32_t B = 0b00010;
    static constexpr uint32_t L = 0b00100;
    static constexpr uint32_t R = 0b01000;
    static constexpr uint32_t W = 0b10000;

    static constexpr uint32_t BOT_SHIFT        = 1;
    static constexpr uint32_t LEFT_SHIFT       = 2;
    static constexpr uint32_t RIGHT_SHIFT      = 3;
    static constexpr uint32_t WINDING_SHIFT    = 4;
    static constexpr uint32_t INT_MASK_SHIFT   = 5;

    static constexpr uint32_t INTERSECTION_MASK  = W | R | L | B | T;
    static constexpr uint32_t MAX_LINES_PER_PATH = 1 << (32 - INT_MASK_SHIFT);

    static constexpr uint16_t WIDTH = 4;
    static constexpr uint16_t HEIGHT = 4;

    uint16_t y;
    uint16_t x;

    uint32_t packed_winding_line_idx;

    Tile() : y(0), x(0), packed_winding_line_idx(0) {}

    static Tile new_tile(uint16_t x, uint16_t y, uint32_t line_idx, uint32_t intersection_mask) {
        Tile t;
        t.x = x;
        t.y = y;
        t.packed_winding_line_idx = (line_idx << INT_MASK_SHIFT) | intersection_mask;
        return t;
    }

    static Tile new_clamped(uint16_t x, uint16_t y, uint32_t line_idx, uint32_t intersection_mask) {
        uint16_t clamped_x = std::min(x, (uint16_t)(0xFFFF / WIDTH));
        uint16_t clamped_y = std::min(y, (uint16_t)(0xFFFF / HEIGHT));
        return new_tile(clamped_x, clamped_y, line_idx, intersection_mask);
    }

    uint64_t to_bits() const {
        return ((uint64_t)y << 48) | ((uint64_t)x << 32) | (uint64_t)packed_winding_line_idx;
    }

    bool operator<(const Tile& other) const {
        return to_bits() < other.to_bits();
    }

    bool operator==(const Tile& other) const {
        return to_bits() == other.to_bits();
    }
};

class Tiles {
public:
    Tiles() {}

    void reset() {
        tile_buf.clear();
        SkDEBUGCODE(sorted = false;)
    }

    void sort_tiles() {
        std::sort(tile_buf.begin(), tile_buf.end());
        SkDEBUGCODE(sorted = true;)
    }

    const std::vector<Tile>& get_tiles() const {
        return tile_buf;
    }

    void make_tiles_msaa(const std::vector<Line>& lines, uint16_t width, uint16_t height);

private:
    std::vector<Tile> tile_buf;
    SkDEBUGCODE(bool sorted;)
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Tiler_DEFINED
