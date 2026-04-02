/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_shaders_NonSubgroupWGSL_DEFINED
#define skgpu_graphite_sparse_strips_shaders_NonSubgroupWGSL_DEFINED

#include "include/core/SkString.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConstants.h"

namespace skgpu::graphite {

inline constexpr char kWgslStructs[] = R"(
struct Config {
    tile_count: u32,
    end_tile_count: u32,
    workgroups: u32,
    d: u32,
};

struct PreMergeTile {
    packed_xy: u32,
    packed_winding_line_idx: u32,
    alpha_index: u32,
    packed_scanned_winding: u32,
};
)";

inline std::string GetWgslConstants() {
    SkString constants = SkStringPrintf(R"(
const INTERSECTS_TOP_MASK: u32          = %uu;
const INTERSECTS_BOTTOM_MASK: u32       = %uu;
const INTERSECTS_LEFT_MASK: u32         = %uu;
const INTERSECTS_RIGHT_MASK: u32        = %uu;
const WINDING_MASK: u32                 = %uu;
const INTERSECTION_MASK: u32            = %uu;
const LINE_SHIFT: u32                   = %uu;
const XY_MASK: u32                      = %uu;
const Y_SHIFT: u32                      = %uu;

const SCANNED_WINDING_SHIFT: u32        = %uu;
const INVALID_START_TILE: u32           = %uu;
const FILL_RULE_SHIFT: u32              = %uu;
const FILL_RULE_MASK: u32               = %uu;
const IS_END_TILE_SHIFT: u32            = %uu;
const IS_END_TILE_MASK: u32             = %uu;
const START_TILE_MASK: u32              = %uu;
const EPSILON: f32                      = %ff;
)",
    SparseStripsConstants::T,
    SparseStripsConstants::B,
    SparseStripsConstants::L,
    SparseStripsConstants::R,
    SparseStripsConstants::W,
    SparseStripsConstants::INTERSECTION_MASK,
    SparseStripsConstants::INT_MASK_SHIFT,
    SparseStripsConstants::XY_MASK,
    SparseStripsConstants::Y_SHIFT,

    SparseStripsConstants::SCANNED_WINDING_SHIFT,
    SparseStripsConstants::INVALID_START_TILE,
    SparseStripsConstants::FILL_RULE_SHIFT,
    SparseStripsConstants::FILL_RULE_MASK,
    SparseStripsConstants::IS_END_TILE_SHIFT,
    SparseStripsConstants::IS_END_TILE_MASK,
    SparseStripsConstants::START_TILE_MASK,
    SparseStripsConstants::kStripEpsilon);

    return std::string(constants.c_str());
}

// Config-derived variables (can be overridden in tests)
inline std::string GetWgslConfigDerived() {
    SkString config = SkStringPrintf(R"(
const BLOCK_DIM: u32                    = %uu;
const TILE_WIDTH: u32                   = %uu;
const TILE_HEIGHT: u32                  = %uu;
const TILE_DIM_LOG: u32                 = %uu;
const TILE_DIM_MASK: u32                = TILE_WIDTH - 1;
const MASK_WIDTH: u32                   = %uu;
const MASK_HEIGHT: u32                  = %uu;
const MASK_WIDTH_EXCL: u32              = %uu;

const MSAA_SUB_SAMPLE_MASK: u32         = 0x%02Xu;
const MSAA_SUB_AS_ADD: u32              = 0x%02Xu;
const MSAA_BIAS_SPLAT: vec2<u32>        = vec2<u32>(0x%02X%02X%02X%02Xu);
const MSAA_INV_SAMPLES: f32             = %ff;
const MSAA_SAMPLES_F32: f32             = %ff;

// Should be read as TILE_WIDTH * (num u32s per subsamples) / (num u32s per u32)
const VEC2_COLS: u32                    = TILE_WIDTH * 2u / 2u;
// The number of tiles processed per workgroup, with the binding of 1 thread per row.
const PART_SIZE: u32                    = BLOCK_DIM / TILE_HEIGHT;
)",
    SparseStripsConfig::kWorkgroupSize,
    SparseStripsConfig::kTileWidth,
    SparseStripsConfig::kTileHeight,
    SparseStripsConfig::kTileDimLog,
    SparseStripsConfig::kLutWidth,
    SparseStripsConfig::kLutHeight,
    SparseStripsConfig::kLutWidth - 1,
    SparseStripsConstants::kSubSampleMask,
    SparseStripsConstants::kMSAASubAsAdd,
    SparseStripsConstants::kMSAABias,
    SparseStripsConstants::kMSAABias,
    SparseStripsConstants::kMSAABias,
    SparseStripsConstants::kMSAABias,
    SparseStripsConstants::kMSAAInvSamples,
    SparseStripsConstants::kMSAASamplesF);

    return std::string(config.c_str());
}

inline constexpr char kWgslHelpers[] = R"(
fn get_intersection_data(packed_winding_line_idx: u32) -> u32 { return packed_winding_line_idx & INTERSECTION_MASK; }
fn get_line_idx(packed_winding_line_idx: u32) -> u32 { return packed_winding_line_idx >> LINE_SHIFT; }
fn get_start_tile_idx(packed_scanned_winding: u32) -> u32 { return packed_scanned_winding & START_TILE_MASK; }
fn get_scanned_winding(packed_scanned_winding: u32) -> u32 {
    // Arithmetic right shift to sign-extend the MSB winding delta down
    return u32(i32(packed_scanned_winding) >> SCANNED_WINDING_SHIFT);
}
fn get_fill_rule(packed_scanned_winding: u32) -> u32 {
    return (packed_scanned_winding >> FILL_RULE_SHIFT) & FILL_RULE_MASK;
}
fn is_end_tile(packed_scanned_winding: u32) -> bool {
    return bool((packed_scanned_winding >> IS_END_TILE_SHIFT) & IS_END_TILE_MASK);
}
fn get_tile_x(packed_xy: u32) -> u32 { return packed_xy & XY_MASK; }
fn get_tile_y(packed_xy: u32) -> u32 { return packed_xy >> Y_SHIFT; }
)";

inline constexpr char kWgslBindings[] = R"(
@group(0) @binding(0) var<uniform> config : Config;
@group(0) @binding(1) var<storage, read> msaa_lut: array<u32>;
@group(0) @binding(2) var<storage, read> pmt_in: array<PreMergeTile>;
@group(0) @binding(3) var<storage, read_write> part_indicator: array<u32>;
@group(0) @binding(4) var<storage, read_write> part_red: array<array<array<vec2<u32>, VEC2_COLS>, TILE_HEIGHT>>;
@group(0) @binding(5) var output: texture_storage_2d<rgba32uint, write>;
@group(0) @binding(6) var<storage, read_write> debug: array<vec2<u32>>;
@group(0) @binding(7) var<storage, read> lines: array<vec4<f32>>;
)";

inline constexpr char kWgslWorkgroupVars[] = R"(
var<workgroup> wg_scan: array<array<array<vec2<u32>, VEC2_COLS>, TILE_HEIGHT>, PART_SIZE>;
)";

inline constexpr char kWgslMain[] = R"(
@compute @workgroup_size(BLOCK_DIM, 1, 1)
fn msaa(@builtin(local_invocation_id) tid: vec3<u32>,
        @builtin(global_invocation_id) gid: vec3<u32>,
        @builtin(workgroup_id) wgid: vec3<u32>)  {
    var pmt: PreMergeTile;
    if ((gid.x >> 2u) < config.tile_count) {
        pmt = pmt_in[gid.x >> 2u];
    } else {
        pmt = PreMergeTile(
            0xffffffffu, // packed_xy
            0u,          // packed_winding_line_idx
            0u,          // alpha_index
            0u           // packed_scanned_winding
        );
    }

    let line_pts = lines[get_line_idx(pmt.packed_winding_line_idx)];
    let p0 = line_pts.xy;
    let p1 = line_pts.zw;

    let d = p1 - p0;
    let is_vertical = abs(d.x) <= EPSILON;
    let is_horizontal = abs(d.y) <= EPSILON;
    let idx = select(1.0f / d.x, 0.0f, is_vertical);
    let idy = select(1.0f / d.y, 0.0f, is_horizontal);
    let dxdy = d.x * idy;
    let dydx = d.y * idx;

    let canonical_x_dir = p1.x >= p0.x;
    let canonical_y_dir = p1.y >= p0.y;

    // TODO remove right_in_viewport, this should be handled upstream now!
    let right = select(p0.x, p1.x, canonical_x_dir);
    let right_in_viewport = right >= 0.0f;
)";

inline constexpr char kWgslClip[] = R"(
    var clipped_top = vec2f(0.0f, 0.0f);
    var clipped_bot = vec2f(0.0f, 0.0f);
    let intersection_data = get_intersection_data(pmt.packed_winding_line_idx);
    if (right_in_viewport) {
        var p_entry = p0;
        var p_exit = p1;

        let tile_min_x_u32 = get_tile_x(pmt.packed_xy) * TILE_WIDTH;
        let tile_min_y_u32 = get_tile_y(pmt.packed_xy) * TILE_HEIGHT;
        let tile_min = vec2f(f32(tile_min_x_u32), f32(tile_min_y_u32));
        let tile_max = vec2f(f32(tile_min_x_u32 + TILE_WIDTH), f32(tile_min_y_u32 + TILE_HEIGHT));

        let x_canon = vec2<bool>(canonical_x_dir);
        let v_masks = select(vec2u(INTERSECTS_RIGHT_MASK, INTERSECTS_LEFT_MASK), vec2u(INTERSECTS_LEFT_MASK, INTERSECTS_RIGHT_MASK), x_canon);
        let v_bounds = select(vec2f(tile_max.x, tile_min.x), vec2f(tile_min.x, tile_max.x), x_canon);
        let mask_v_in = v_masks.x;
        let mask_v_out = v_masks.y;
        let bound_v_in = v_bounds.x;
        let bound_v_out = v_bounds.y;

        let y_canon = vec2<bool>(canonical_y_dir);
        let h_masks = select(vec2u(INTERSECTS_BOTTOM_MASK, INTERSECTS_TOP_MASK), vec2u(INTERSECTS_TOP_MASK, INTERSECTS_BOTTOM_MASK), y_canon);
        let h_bounds = select(vec2f(tile_max.y, tile_min.y), vec2f(tile_min.y, tile_max.y), y_canon);
        let mask_h_in = h_masks.x;
        let mask_h_out = h_masks.y;
        let bound_h_in = h_bounds.x;
        let bound_h_out = h_bounds.y;

        let entry_hits = intersection_data & (mask_v_in | mask_h_in);
        {
            let use_h = (intersection_data & mask_h_in) != 0u;
            let p_oriented = select(p0.yx, p0.xy, use_h);
            let bound = select(bound_v_in, bound_h_in, use_h);
            let slope = select(dydx, dxdy, use_h);
            let calculated = p_oriented.x + (bound - p_oriented.y) * slope;
            let candidate = select(vec2f(bound, calculated), vec2f(calculated, bound), use_h);
            p_entry = select(p_entry, candidate, entry_hits != 0u);
        }

        let exit_hits = intersection_data & (mask_v_out | mask_h_out);
        {
            let use_h = (intersection_data & mask_h_out) != 0u;
            let p_oriented = select(p0.yx, p0.xy, use_h);
            let bound = select(bound_v_out, bound_h_out, use_h);
            let slope = select(dydx, dxdy, use_h);
            let calculated = p_oriented.x + (bound - p_oriented.y) * slope;
            let candidate = select(vec2f(bound, calculated), vec2f(calculated, bound), use_h);
            p_exit = select(p_exit, candidate, exit_hits != 0u);
        }

        {
            let exit_is_min = p_exit.y < p_entry.y;
            clipped_top = select(p_entry, p_exit, exit_is_min);
            clipped_bot = select(p_exit, p_entry, exit_is_min);

            clipped_top -= tile_min;
            clipped_bot -= tile_min;

            let tile_size = vec2f(f32(TILE_WIDTH), f32(TILE_HEIGHT));
            let zero_vec = vec2f(0.0);

            clipped_top = clamp(clipped_top, zero_vec, tile_size);
            clipped_bot = clamp(clipped_bot, zero_vec, tile_size);
        }
    }
)";

const std::string kWgslRowSetup = R"(
    // 1. Thread and Row Setup
    let local_y = tid.x & TILE_DIM_MASK;
    let startY = u32(floor(clipped_top.y));
    let endY = u32(ceil(clipped_bot.y));
    let xDir = clipped_top.x <= clipped_bot.x;
    let is_winding = get_fill_rule(pmt.packed_scanned_winding) != 0u;

    // 2. Declare and Initialize Row Mask
    var mask: array<vec2u, TILE_WIDTH>;
    {
        let winding = get_scanned_winding(pmt.packed_scanned_winding);
        let is_start_tile = get_start_tile_idx(pmt.packed_scanned_winding) == (tid.x >> 2u);
        let w_base = winding * select(0u, 0x1010101u, is_start_tile);

        let w = MSAA_BIAS_SPLAT + vec2u(w_base);
        for (var col = 0u; col < TILE_WIDTH; col += 1u) {
            mask[col] = w;
        }
    }
)";

const std::string kWgslLeftEdgeFill = R"(
    // 3. Wide SIMD L Boundary Fill (Evaluated per row)
    if ((intersection_data & INTERSECTS_LEFT_MASK) != 0u) {
        let y_edge = select(clipped_bot.y, clipped_top.y, xDir);
        if (f32(local_y) >= ceil(y_edge)) {
            let fill_byte = select(1u, select(MSAA_SUB_AS_ADD, 1u, canonical_x_dir), is_winding);
            let p_fill = vec2u(fill_byte * 0x01010101u);
            for (var col = 0u; col < TILE_WIDTH; col += 1u) {
                if (is_winding) {
                    mask[col] += p_fill;
                } else {
                    mask[col] ^= p_fill;
                }
            }
        }
    }
)";

// Hardcoded for MSAA8 fix this in the future!
const std::string kWgslProcessPixel = R"(
                let u = clamp(t_fixed >> 16u, 0, i32(MASK_WIDTH_EXCL));
                let byte_idx = u32(lut_row_offset) + u32(u);
                let word_idx = byte_idx >> 2u;
                let bit_shift = (byte_idx & 3u) << 3u;

                // Shift the targeted 8-bit segment down. The & local_mask naturally
                // strips out the upper bits of the u32, preventing bleed-through.
                let maskVal = (msaa_lut[word_idx] >> bit_shift) & local_mask;

                let mask_lo = ((maskVal & 0x0Fu) * 0x00204081u) & 0x01010101u;
                let mask_hi = (((maskVal >> 4u) & 0x0Fu) * 0x00204081u) & 0x01010101u;

                var p_res = vec2u(mask_lo, mask_hi);
                let p_bump = vec2u(local_bump * 0x01010101u);

                if (is_winding) {
                    p_res = p_res - p_bump;
                    p_res = select(vec2u(0u) - p_res, p_res, canonical_y_dir);
                    mask[local_x] += p_res;
                } else {
                    p_res = p_res ^ p_bump;
                    mask[local_x] ^= p_res;
                }
)";

const std::string kWgslMaskGen_SlopeIntercept = R"(
    // 4. LUT Setup (Calculated redundantly across the 4 threads per tile)
    var nx = d.y;
    var ny = -d.x;
    if (nx < 0.0) {
        nx = -nx;
        ny = -ny;
    }

    let D = nx + abs(ny);
    let invD = select(1.0 / D, 0.0, D < EPSILON);
    let isPos = ny <= 0.0;

    let c_local = nx * p0.x + ny * p0.y;
    let s = abs(ny) * invD;

    let half_mask_height = i32(MASK_HEIGHT / 2u);
    let v_mod = clamp(i32(floor(s * f32(half_mask_height))), 0, half_mask_height - 1);
    let v = select(v_mod, v_mod + half_mask_height, isPos);

    let stepX = nx * invD;
    let stepY = ny * invD;
    let t_base = (select(D, nx, isPos) - c_local) * invD;

    let lut_row_offset = v * i32(MASK_WIDTH);
    let stepX_fixed = i32(stepX * 64.0 * 65536.0);

    // 5. Per-Row Line Intersection & LUT Fetch
    let is_perfect_horizontal = is_horizontal && clipped_top.y == floor(clipped_top.y);

    if (!is_perfect_horizontal && local_y >= startY && local_y < endY) {
        let is_startY = (local_y == startY);
        let is_endY_minus_1 = (local_y == endY - 1u);

        let py_top = select(f32(local_y), clipped_top.y, is_startY);
        let py_bot = select(f32(local_y + 1u), clipped_bot.y, is_endY_minus_1);

        let px_top = select(clipped_top.x + (py_top - clipped_top.y) * dxdy, clipped_top.x, is_horizontal || is_startY);
        let px_bot = select(clipped_top.x + (py_bot - clipped_top.y) * dxdy, clipped_bot.x, is_horizontal || is_endY_minus_1);

        let xMin = min(px_top, px_bot);
        let xMax = max(px_top, px_bot);
        let xStart = clamp(u32(floor(xMin)), 0u, TILE_WIDTH - 1u);
        let xEnd = clamp(u32(floor(xMax)), 0u, TILE_WIDTH - 1u);

        var t_val = t_base + stepY * f32(local_y) + stepX * f32(xStart);
        var t_fixed = i32(t_val * 64.0 * 65536.0);

        let pixelTopTouch = (py_top != floor(py_top));
        let default_bumped = pixelTopTouch && xDir;

        var start_mask_val = MSAA_SUB_SAMPLE_MASK;
        var start_bumped = default_bumped;
        if (is_startY) {
            start_bumped = (px_top == 0.0 && pixelTopTouch);
            if (!start_bumped) {
                let shift = u32(round(MSAA_SAMPLES_F32 * (py_top - f32(local_y))));
                start_mask_val = MSAA_SUB_SAMPLE_MASK << shift;
            }
        }

        var end_mask_val = MSAA_SUB_SAMPLE_MASK;
        if (is_endY_minus_1 && px_bot != 0.0) {
            let shift = u32(round(MSAA_SAMPLES_F32 * (py_bot - f32(local_y))));
            end_mask_val = ~(MSAA_SUB_SAMPLE_MASK << shift) & MSAA_SUB_SAMPLE_MASK;
        }

        // 6. Pixel Processing Loops
        if (xStart == xEnd) {
            let local_mask = start_mask_val & end_mask_val;
            let local_bump = u32(start_bumped);
            let local_x = xStart;
)" + kWgslProcessPixel + R"(
            t_fixed += stepX_fixed;
        } else {
            // --- Left Edge ---
            {
                let local_mask = select(end_mask_val, start_mask_val, xDir);
                let local_bump = u32(select(default_bumped, start_bumped, xDir));
                let local_x = xStart;
)" + kWgslProcessPixel + R"(
                t_fixed += stepX_fixed;
            }

            // --- Mid Pixels ---
            {
                let midBump = u32(default_bumped);
                for (var x = xStart + 1u; x < xEnd; x += 1u) {
                    let local_mask = MSAA_SUB_SAMPLE_MASK;
                    let local_bump = midBump;
                    let local_x = x;
)" + kWgslProcessPixel + R"(
                    t_fixed += stepX_fixed;
                }
            }

            // --- Right Edge ---
            {
                let local_mask = select(start_mask_val, end_mask_val, xDir);
                let local_bump = u32(select(start_bumped, default_bumped, xDir));
                let local_x = xEnd;
)" + kWgslProcessPixel + R"(
            }
        }

        // 7. Post-Edge SIMD Fill (crossedTop logic)
        let crossedTop = (local_y > startY) || (py_top == floor(py_top));
        if (crossedTop) {
            let fill_byte = select(1u, select(MSAA_SUB_AS_ADD, 1u, canonical_y_dir), is_winding);
            let p_fill = vec2u(fill_byte * 0x01010101u);
            for (var x = xEnd + 1u; x < TILE_WIDTH; x += 1u) {
                if (is_winding) {
                    mask[x] += p_fill;
                } else {
                    mask[x] ^= p_fill;
                }
            }
        }
    }
)";

inline constexpr char kWgslScan_Base[] = R"(
    let row_index = tid.x & 3u;
    let tile_index = tid.x >> 2u;
    for (var col = 0u; col < TILE_WIDTH; col += 1u) {
        wg_scan[tile_index][row_index][col] = mask[col];
    }

    for (var i = 1u; i < PART_SIZE; i <<= 1u) {
        workgroupBarrier();
        let ii = tile_index - i;
        if (tile_index >= i) {
            for (var col = 0u; col < TILE_WIDTH; col += 1u) {
                let unbiased = wg_scan[ii][row_index][col] - MSAA_BIAS_SPLAT;
                mask[col] += unbiased;
            }
        }
        workgroupBarrier();

        if (tile_index >= i) {
            for (var col = 0u; col < TILE_WIDTH; col += 1u) {
                wg_scan[tile_index][row_index][col] = mask[col];
            }
        }
    }
    workgroupBarrier();
)";

inline constexpr char kWgslScan_SegmentedReduction[] = R"(
    let tile_start_id = get_start_tile_idx(pmt.packed_scanned_winding);
    let is_valid = tile_start_id != INVALID_START_TILE;
    let is_last_tile = (tid.x >> 2u) == PART_SIZE - 1u;
    let is_end = is_end_tile(pmt.packed_scanned_winding);

    if (is_end || is_last_tile) {
        if (is_valid && tile_start_id != 0u) {
            let pred_id = tile_start_id - 1u;
            for (var col = 0u; col < TILE_WIDTH; col += 1u) {
                let unbiased = wg_scan[pred_id][row_index][col] - MSAA_BIAS_SPLAT;
                mask[col] -= unbiased;
            }
        }
    }
    workgroupBarrier();

    if (is_end && is_valid && tile_start_id != 0u) {
        for (var col = 0u; col < TILE_WIDTH; col += 1u) {
            wg_scan[tile_index][row_index][col] = mask[col];
        }
    }
    workgroupBarrier();
)";

inline constexpr char kWgslBitCountingInner[] = R"(
                    let diff = cov_mask ^ MSAA_BIAS_SPLAT;
                    let subbed = diff - 0x01010101u;
                    let zeros = subbed & (~diff) & MSAA_BIAS_SPLAT;
                    let actives = zeros ^ MSAA_BIAS_SPLAT;
                    let counts = ((actives >> vec2u(7)) * 0x01010101u) >> vec2u(24);
                    row_pixels[col] = f32(counts.x + counts.y) * MSAA_INV_SAMPLES;
)";

inline const std::string kWgslBitCounting = std::string(R"(
            for (var row = 0u; row < TILE_HEIGHT; row += 1u) {
                var row_pixels: vec4<f32>;
                for (var col = 0u; col < TILE_WIDTH; col += 1u) {
                    let cov_mask = tile_masks[row][col];
)") + kWgslBitCountingInner + R"(
                }
                final_alphas[row] = pack4x8unorm(abs(row_pixels));
            }
)";

inline constexpr char kWgslTextureStore[] = R"(
            let tex_dims = textureDimensions(output);
            let tex_width = tex_dims.x;
            let output_coords = vec2<u32>(
                alpha_index % tex_width,
                alpha_index / tex_width
            );
            if (output_coords.y < tex_dims.y) {
                textureStore(output, output_coords, final_alphas);
            }
)";

}  // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_shaders_NonSubgroupWGSL_DEFINED
