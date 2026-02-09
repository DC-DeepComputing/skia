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

class Strip {
public:
    Strip() {}

    static SK_ALWAYS_INLINE std::array<std::array<float, 2>, 2> ClipToTile(
        const Line& line,
        const std::array<float, 4>& bounds,
        const std::array<float, 4>& derivatives,
        uint32_t intersection_data,
        bool cannonical_x_dir,
        bool cannonical_y_dir
    ) {
        std::array<float, 2> p_entry = {line.p0.x, line.p0.y};
        std::array<float, 2> p_exit  = {line.p1.x, line.p1.y};

        const float tile_min_x = bounds[0];
        const float tile_min_y = bounds[1];
        const float tile_max_x = bounds[2];
        const float tile_max_y = bounds[3];

        const float dx = derivatives[0];
        const float dy = derivatives[1];

        uint32_t mask_v_in, mask_v_out;
        float bound_v_in, bound_v_out;
        if (cannonical_x_dir) {
            mask_v_in  = Tile::L;
            bound_v_in = tile_min_x;
            mask_v_out = Tile::R;
            bound_v_out= tile_max_x;
        } else {
            mask_v_in  = Tile::R;
            bound_v_in = tile_max_x;
            mask_v_out = Tile::L;
            bound_v_out= tile_min_x;
        }

        uint32_t mask_h_in, mask_h_out;
        float bound_h_in, bound_h_out;
        if (cannonical_y_dir) {
            mask_h_in  = Tile::T;
            bound_h_in = tile_min_y;
            mask_h_out = Tile::B;
            bound_h_out= tile_max_y;
        } else {
            mask_h_in  = Tile::B;
            bound_h_in = tile_max_y;
            mask_h_out = Tile::T;
            bound_h_out= tile_min_y;
        }

        const float idx = derivatives[2];
        const float idy = derivatives[3];

        const uint32_t entry_hits = intersection_data & (mask_v_in | mask_h_in);
        if (entry_hits != 0) {
            const bool use_h = (intersection_data & mask_h_in) != 0;

            const float bound = use_h ? bound_h_in : bound_v_in;
            const float start = use_h ? line.p0.y  : line.p0.x;
            const float inv_d = use_h ? idy        : idx;

            const float t = (bound - start) * inv_d;

            p_entry[0] = line.p0.x + t * dx;
            p_entry[1] = line.p0.y + t * dy;
            p_entry[use_h ? 1 : 0] = bound;
        }

        const uint32_t exit_hits = intersection_data & (mask_v_out | mask_h_out);
        if (exit_hits != 0) {
            const bool use_h = (intersection_data & mask_h_out) != 0;

            const float bound = use_h ? bound_h_out : bound_v_out;
            const float start = use_h ? line.p0.y   : line.p0.x;
            const float inv_d = use_h ? idy         : idx;

            const float t = (bound - start) * inv_d;

            p_exit[0] = line.p0.x + t * dx;
            p_exit[1] = line.p0.y + t * dy;
            p_exit[use_h ? 1 : 0] = bound;
        }

        std::array<std::array<float, 2>, 2> result;
        if (p_exit[1] >= p_entry[1]) {
            result = {p_entry, p_exit};
        } else {
            result = {p_exit, p_entry};
        }

        result[0][0] -= tile_min_x;
        result[0][1] -= tile_min_y;
        result[1][0] -= tile_min_x;
        result[1][1] -= tile_min_y;

        // Clamping serves three purposes here:
        // 1) Points which are slightly outside the tile due to floating point error are coerced
        //    inside.
        // 2) More subtly, perfectly horizontal or vertical lines have their reciprocal derivatives
        //    set to 0 in the upstream stirp logic. This causes the intersection calculation to
        //    return the original coordinate. While the coordinate fixed to the tile edge is
        //    explicitly set (and guaranteed valid), clamping forces the coordinate along that edge
        //    to be in bounds and watertight.
        // 3) In the case that there is only a single edge crossing (either from a corner touch
        //    tiebroken to a single dimension or an inclusive touch on the vertical edge of a tile),
        //    the opposing point must be somewhere in the direction normal to the edge. Clamping
        //    forces the segment to snap onto the tile edge. This creates zero-width geometry which
        //    is gets discarded by downstream pixel filling logic.
        const float width  = tile_max_x - tile_min_x;
        const float height = tile_max_y - tile_min_y;

        result[0][0] = std::clamp(result[0][0], 0.0f, width);
        result[0][1] = std::clamp(result[0][1], 0.0f, height);
        result[1][0] = std::clamp(result[1][0], 0.0f, width);
        result[1][1] = std::clamp(result[1][1], 0.0f, height);

        return result;
    }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Strip_DEFINED
