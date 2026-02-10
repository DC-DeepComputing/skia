/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace skgpu::graphite {

namespace {

static SK_ALWAYS_INLINE uint16_t div_ceil(uint16_t a, uint16_t b) { return (a + b - 1) / b; }

// Mimic rust casting behavior
static SK_ALWAYS_INLINE uint16_t f32_to_u16_sat(float v) {
    if (v < 0.0f) return 0;
    if (v > 65535.0f) return 65535;
    return static_cast<uint16_t>(v);
}

struct LineContext {
    std::vector<Tile>& tile_buf;
    uint32_t line_idx;
    float    p0_x, p0_y;
    float    x_slope;
    float    line_left_x, line_right_x;
    uint32_t dx_dir;     // 1 if right, 0 if left
    uint32_t not_dx_dir; // Inverse
    uint32_t w_start_base;
    uint32_t w_end_base;
    int32_t  p0_tile_x, p0_tile_y;
    int32_t  p1_tile_x, p1_tile_y;
    uint16_t tile_columns;
};

template <bool kCheckStart, bool kCheckEnd>
static SK_ALWAYS_INLINE void push_edge(const LineContext& ctx,
                                       uint16_t x_idx,
                                       uint16_t y,
                                       float row_top_x,
                                       float row_bottom_x,
                                       int32_t canonical_start,
                                       uint16_t canonical_end,
                                       uint32_t winding_input) {
    uint32_t unc_row_start = (static_cast<int32_t>(x_idx) == canonical_start) ? 1 : 0;
    uint32_t unc_row_end = (x_idx == canonical_end) ? 1 : 0;

    uint32_t canonical_row_start = (ctx.dx_dir & unc_row_start) | (ctx.not_dx_dir & unc_row_end);
    uint32_t canonical_row_end = (ctx.not_dx_dir & unc_row_start) | (ctx.dx_dir & unc_row_end);

    uint32_t not_start_tile = 1;
    if constexpr (kCheckStart) {
        not_start_tile ^= static_cast<uint32_t>((static_cast<int32_t>(x_idx) == ctx.p0_tile_x) &&
                                                (static_cast<int32_t>(y) == ctx.p0_tile_y));
    }

    uint32_t not_end_tile = 1;
    if constexpr (kCheckEnd) {
        not_end_tile ^= static_cast<uint32_t>((static_cast<int32_t>(x_idx) == ctx.p1_tile_x) &&
                                              (static_cast<int32_t>(y) == ctx.p1_tile_y));
    }

    uint32_t mask = winding_input;
    mask |= canonical_row_start & not_start_tile;
    mask |= (1 ^ canonical_row_start) << ctx.not_dx_dir << Tile::LEFT_SHIFT;
    mask |= (canonical_row_end & not_end_tile) << Tile::BOT_SHIFT;
    mask |= (1 ^ canonical_row_end) << ctx.dx_dir << Tile::LEFT_SHIFT;

    float x_left_f = static_cast<float>(x_idx);
    float x_right_f = static_cast<float>(x_idx + 1);

    uint32_t trc = ((row_top_x == x_right_f)    ? 1 : 0) & not_start_tile;
    uint32_t tlc = ((row_top_x == x_left_f)     ? 1 : 0) & not_start_tile;
    uint32_t brc = ((row_bottom_x == x_right_f) ? 1 : 0) & not_end_tile;
    uint32_t blc = ((row_bottom_x == x_left_f)  ? 1 : 0) & not_end_tile;

    uint32_t tie_break = tlc & (canonical_row_start ^ 1);

    if (tie_break | blc) mask |= Tile::L;
    if (trc | brc) mask |= Tile::R;
    if (tie_break | trc) mask &= ~Tile::T;
    if (blc | brc) mask &= ~Tile::B;

    ctx.tile_buf.push_back(Tile::new_tile(x_idx, y, ctx.line_idx, mask));
}

template <bool kNeedsClamping, bool kCheckStart, bool kCheckEnd>
static SK_ALWAYS_INLINE void process_row(const LineContext& ctx,
                                         uint16_t y_idx,
                                         float row_top_y,
                                         float row_bottom_y,
                                         uint32_t w_mask) {
    float row_top_x = ctx.p0_x + (row_top_y - ctx.p0_y) * ctx.x_slope;
    float row_bottom_x = ctx.p0_x + (row_bottom_y - ctx.p0_y) * ctx.x_slope;

    float lx, rx;
    uint16_t x_end_val;

    if constexpr (kNeedsClamping) {
        // Use fmin/fmax to ensure NaN erasure
        float min_row_x = std::fmin(row_top_x, row_bottom_x);
        float max_row_x = std::fmax(row_top_x, row_bottom_x);
        lx = std::fmax(min_row_x, ctx.line_left_x);
        rx = std::fmin(max_row_x, ctx.line_right_x);
        x_end_val =
                std::min(f32_to_u16_sat(rx),
                         static_cast<uint16_t>(ctx.tile_columns > 0 ? ctx.tile_columns - 1 : 0));
    } else {
        lx = std::fmin(row_top_x, row_bottom_x);
        rx = std::fmax(row_top_x, row_bottom_x);
        x_end_val = f32_to_u16_sat(rx);
    }

    int32_t canonical_x_start = static_cast<int32_t>(std::floor(lx));
    uint16_t canonical_x_end = f32_to_u16_sat(rx);
    uint16_t x_start = f32_to_u16_sat(lx);

    if (x_start <= x_end_val) {
        uint32_t is_single = (x_start == x_end_val) ? 1 : 0;
        uint32_t w_left = (ctx.w_start_base | (is_single ? Tile::W : 0)) & w_mask;

        push_edge<kCheckStart, kCheckEnd>(ctx,
                                          x_start,
                                          y_idx,
                                          row_top_x,
                                          row_bottom_x,
                                          canonical_x_start,
                                          canonical_x_end,
                                          w_left);

        for (uint16_t x_k = x_start + 1; x_k < x_end_val; ++x_k) {
            ctx.tile_buf.push_back(Tile::new_tile(x_k, y_idx, ctx.line_idx, Tile::R | Tile::L));
        }

        if (x_start < x_end_val) {
            uint32_t w_right = ctx.w_end_base & w_mask;
            push_edge<kCheckStart, kCheckEnd>(ctx,
                                              x_end_val,
                                              y_idx,
                                              row_top_x,
                                              row_bottom_x,
                                              canonical_x_start,
                                              canonical_x_end,
                                              w_right);
        }
    }
}

template <bool kNeedsClamping>
static SK_ALWAYS_INLINE void run_loops(const LineContext& ctx,
                                       float line_top_y,
                                       float line_bottom_y,
                                       uint16_t y_top_tiles,
                                       uint16_t tile_rows) {
    bool is_start_culled = (line_top_y < 0.0f);
    if (!is_start_culled) {
        float y = static_cast<float>(y_top_tiles);
        float row_bottom_y = std::min(y + 1.0f, line_bottom_y);
        uint32_t mask = (y >= line_top_y) ? Tile::W : 0;
        process_row<kNeedsClamping, true, true>(ctx, y_top_tiles, line_top_y, row_bottom_y, mask);
    }

    uint16_t y_start_middle = is_start_culled ? y_top_tiles : (y_top_tiles + 1);
    float line_bottom_floor = std::floor(line_bottom_y);
    uint16_t y_end_middle = std::min(f32_to_u16_sat(line_bottom_floor), tile_rows);

    bool has_separate_bottom_row = (line_bottom_y != line_bottom_floor) &&
                                   (y_end_middle < tile_rows) &&
                                   (is_start_culled || y_end_middle != y_top_tiles);

    if (y_start_middle < y_end_middle) {
        uint16_t middle_limit = y_end_middle;
        if (!has_separate_bottom_row && middle_limit > y_start_middle) {
            middle_limit--;
        }

        for (uint16_t y_idx = y_start_middle; y_idx < middle_limit; ++y_idx) {
            float y = static_cast<float>(y_idx);
            process_row<kNeedsClamping, false, false>(ctx, y_idx, y, y + 1.0f, 0xffffffff);
        }

        if (!has_separate_bottom_row && y_end_middle > y_start_middle) {
            uint16_t y_idx = y_end_middle - 1;
            float y = static_cast<float>(y_idx);
            float row_bottom_y = std::min(y + 1.0f, line_bottom_y);
            process_row<kNeedsClamping, false, true>(ctx, y_idx, y, row_bottom_y, 0xffffffff);
        }
    }

    if (has_separate_bottom_row) {
        uint16_t y_idx = y_end_middle;
        float y = static_cast<float>(y_idx);
        process_row<kNeedsClamping, false, true>(ctx, y_idx, y, line_bottom_y, 0xffffffff);
    }
}

}  // namespace

void Tiles::make_tiles_msaa(const std::vector<Line>& lines, uint16_t width, uint16_t height) {
    SkASSERT(lines.size() <= Tile::MAX_LINES_PER_PATH);
    this->reset();

    if (width == 0 || height == 0) {
        return;
    }

    uint16_t tile_columns = div_ceil(width, Tile::WIDTH);
    uint16_t tile_rows = div_ceil(height, Tile::HEIGHT);
    for (uint32_t line_idx = 0; line_idx < lines.size(); ++line_idx) {
        const Line& line = lines[line_idx];

        float p0_x = line.p0.x / static_cast<float>(Tile::WIDTH);
        float p0_y = line.p0.y / static_cast<float>(Tile::HEIGHT);
        float p1_x = line.p1.x / static_cast<float>(Tile::WIDTH);
        float p1_y = line.p1.y / static_cast<float>(Tile::HEIGHT);

        float line_left_x, line_right_x;
        if (p0_x < p1_x) {
            line_left_x = p0_x;
            line_right_x = p1_x;
        } else {
            line_left_x = p1_x;
            line_right_x = p0_x;
        }

        // Lines whose left-most endpoint exceed the right edge of the viewport are culled
        if (line_left_x > static_cast<float>(tile_columns)) {
            continue;
        }

        float line_top_y, line_top_x, line_bottom_y, line_bottom_x;
        if (p0_y < p1_y) {
            line_top_y = p0_y;
            line_top_x = p0_x;
            line_bottom_y = p1_y;
            line_bottom_x = p1_x;
        } else {
            line_top_y = p1_y;
            line_top_x = p1_x;
            line_bottom_y = p0_y;
            line_bottom_x = p0_x;
        }

        // Clamp y coordinates to tile rows
        uint16_t y_top_tiles = std::min(f32_to_u16_sat(line_top_y), tile_rows);
        float line_bottom_y_ceil = std::ceil(line_bottom_y);
        uint16_t y_bottom_tiles = std::min(f32_to_u16_sat(line_bottom_y_ceil), tile_rows);

        // Cull checks
        if (y_top_tiles >= y_bottom_tiles) {
            continue;
        }

        // Tile coords for start/end points
        int32_t p0_tile_x = static_cast<int32_t>(std::floor(line_top_x));
        int32_t p0_tile_y = static_cast<int32_t>(std::floor(line_top_y));
        int32_t p1_tile_x = static_cast<int32_t>(std::floor(line_bottom_x));

        int32_t p1_tile_y;
        if (line_bottom_y == line_bottom_y_ceil) {
            p1_tile_y = static_cast<int32_t>(line_bottom_y) - 1;
        } else {
            p1_tile_y = static_cast<int32_t>(std::floor(line_bottom_y));
        }

        bool not_same_tile = (p0_tile_y != p1_tile_y) || (p0_tile_x != p1_tile_x);
        if (not_same_tile) {
            if (line_left_x == line_right_x) {  // Purely vertical lines
                uint16_t x =
                        std::min(f32_to_u16_sat(line_left_x),
                                 static_cast<uint16_t>(tile_columns > 0 ? tile_columns - 1 : 0));

                bool is_start_culled = (line_top_y < 0.0f);
                if (!is_start_culled) {
                    uint32_t winding =
                            (static_cast<float>(y_top_tiles) >= line_top_y) ? Tile::W : 0;
                    uint32_t intersection_mask = Tile::B | winding;
                    tileBuf.push_back(
                            Tile::new_clamped(x, y_top_tiles, line_idx, intersection_mask));
                }

                uint16_t y_start = is_start_culled ? y_top_tiles : (y_top_tiles + 1);
                float line_bottom_floor = std::floor(line_bottom_y);
                uint16_t y_end_idx = std::min(f32_to_u16_sat(line_bottom_floor), tile_rows);

                if (y_start < y_end_idx) {
                    uint16_t y_last = y_end_idx - 1;
                    for (uint16_t y_idx = y_start; y_idx < y_last; ++y_idx) {
                        uint32_t intersection_mask = Tile::W | Tile::B | Tile::T;
                        tileBuf.push_back(
                                Tile::new_clamped(x, y_idx, line_idx, intersection_mask));
                    }

                    {
                        bool is_end_tile = ((int32_t)y_last == p1_tile_y);
                        uint32_t intersection_mask =
                                Tile::W | Tile::T | (is_end_tile ? 0 : Tile::B);
                        tileBuf.push_back(
                                Tile::new_clamped(x, y_last, line_idx, intersection_mask));
                    }
                }

                if (line_bottom_y != line_bottom_floor && y_end_idx < tile_rows) {
                    uint32_t intersection_mask = Tile::W | Tile::T;
                    tileBuf.push_back(
                            Tile::new_clamped(x, y_end_idx, line_idx, intersection_mask));
                }
            } else {  // --- General slanted line case ---
                float dx = p1_x - p0_x;
                float dy = p1_y - p0_y;
                float x_slope = dx / dy;

                uint32_t dx_dir = (line_bottom_x >= line_top_x) ? 1 : 0;
                uint32_t not_dx_dir = dx_dir ^ 1;

                uint32_t w_start_base = dx_dir ? Tile::W : 0;
                uint32_t w_end_base = not_dx_dir ? Tile::W : 0;

                LineContext ctx {
                    this->tileBuf,
                    line_idx,
                    p0_x,
                    p0_y,
                    x_slope,
                    line_left_x,
                    line_right_x,
                    dx_dir,
                    not_dx_dir,
                    w_start_base,
                    w_end_base,
                    p0_tile_x,
                    p0_tile_y,
                    p1_tile_x,
                    p1_tile_y,
                    tile_columns
                };

                float min_x = std::min(p0_x, p1_x);
                float max_x = std::max(p0_x, p1_x);
                bool needs_clamping = (min_x < line_left_x) || (max_x >= line_right_x);

                if (needs_clamping) {
                    run_loops<true>(ctx, line_top_y, line_bottom_y, y_top_tiles, tile_rows);
                } else {
                    run_loops<false>(ctx, line_top_y, line_bottom_y, y_top_tiles, tile_rows);
                }
            }

        } else {
            // Case: Line is fully contained within a single tile
            uint16_t x_clamped =
                    std::min(f32_to_u16_sat(line_left_x), static_cast<uint16_t>(tile_columns + 1));
            uint32_t winding = (static_cast<float>(y_top_tiles) >= line_top_y) ? Tile::W : 0;

            tileBuf.push_back(Tile::new_clamped(x_clamped, y_top_tiles, line_idx, winding));
        }
    }
}

}  // namespace skgpu::graphite
