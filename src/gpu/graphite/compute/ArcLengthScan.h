/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_ArcLengthScan_DEFINED
#define skgpu_graphite_compute_ArcLengthScan_DEFINED

#include <string_view>

namespace skgpu::graphite {

/**
 * ArcLengthScan defines concrete SkSL shader policies for arc length
 * prefix scan compute passes.
 */
namespace ArcLengthScan {

// Approximate length of cubic Bezier curve using Gauss-Legendre / Gauss-Lobatto quadrature with n=5
// points. Evaluates derivative magnitude |C'(t)| at sample points t in {0, 0.5 ± sqrt(3/7)/2, 0.5,
// 1}, weighted 1/20, 49/180, 32/90, 49/180, 1/20 respectively. Linear combinations of control
// points are pre-factored into compile-time constants. We assume that CPU preprocessing
// (FindCubicConvex180Chops) has already eliminated 180° cusps and inflection points.
// https://en.wikipedia.org/wiki/Gaussian_quadrature#Gauss%E2%80%93Lobatto_rules

// Approximate conic arc length using Arithmetic-Geometric Mean (AGM) to evaluate elliptic
// integrals. Uses a fixed 3-pass unrolled execution. We assume CPU preprocessing (ConicHasCusp) has
// already detected degenerate flat lines with 180° cusps and converted them to lines/caps. Since we
// the lowest two bits in the float are used to pack the chained scan status flags, we have a
// maximum of 30 bits of fp precision. For 30-bit fp precision (21 mantissa bits, eps ~ 4.77e-7), 3
// AGM iterations guarantee convergence below machine precision (c3 < 1.86e-9) for all uploaded
// curves (b/a >= 0.01).
// https://en.wikipedia.org/wiki/Arithmetic%E2%80%93geometric_mean#Application_to_elliptic_integrals

struct GenericPreambleBase {
    static inline constexpr std::string_view kPreamble =
            "const uint BLOCK_DIM = 256u;\n"
            "const uint PART_SIZE = BLOCK_DIM;\n"
            "const uint S_MEM_SIZE = BLOCK_DIM;\n"
            "\n"
            "float eval_cubic_arclength(float2 c0, float2 c1, float2 c2, float2 c3) {\n"
            "    float v0 = length(c1 - c0) * 0.15;\n"
            "    float v1 = length(-0.558983582205757 * c0 + 0.325650248872424 * c1 +\n"
            "                      0.208983582205757 * c2 + 0.024349751127576 * c3);\n"
            "    float v2 = length(c3 - c0 + c2 - c1) * 0.26666666666666666;\n"
            "    float v3 = length(-0.024349751127576 * c0 - 0.208983582205757 * c1 -\n"
            "                      0.325650248872424 * c2 + 0.558983582205757 * c3);\n"
            "    float v4 = length(c3 - c2) * 0.15;\n"
            "    return v0 + v1 + v2 + v3 + v4;\n"
            "}\n"
            "\n"
            "float eval_conic_arclength(float2 p0, float2 p1, float2 p2, float w) {\n"
            "    float a = length(p1 - p0);\n"
            "    float b = length(p2 - p1);\n"
            "    if (a < b) {\n"
            "        float tmp = a; a = b; b = tmp;\n"
            "    }\n"
            "    if (b == 0.0) {\n"
            "        return 4.0 * a;\n"
            "    }\n"
            "    float a_n = 1.0;\n"
            "    float b_n = b / a;\n"
            "    float sum = 0.5 * (1.0 - b_n * b_n);\n"
            "\n"
            "    // Pass 1\n"
            "    float a1 = (a_n + b_n) * 0.5;\n"
            "    float b1 = sqrt(a_n * b_n);\n"
            "    float c1 = (a_n - b_n) * 0.5;\n"
            "    sum += 1.0 * (c1 * c1);\n"
            "\n"
            "    // Pass 2\n"
            "    float a2 = (a1 + b1) * 0.5;\n"
            "    float b2 = sqrt(a1 * b1);\n"
            "    float c2 = (a1 - b1) * 0.5;\n"
            "    sum += 2.0 * (c2 * c2);\n"
            "\n"
            "    // Pass 3\n"
            "    float c3 = (a2 - b2) * 0.5;\n"
            "    sum += 4.0 * (c3 * c3);\n"
            "\n"
            "    return 6.283185307 * a * (1.0 - sum);\n"
            "}\n"
            "\n"
            // Assumes will be called by the last thread in the block
            "bool is_tile_inclusive(bool start_pred, uint tile_id) {\n"
            "    return tile_id == 0 || start_pred;\n"
            "}\n"
            "uint set_locked_condition(bool is_inclusive) {\n"
            "    return is_inclusive ? UNLOCKED : LOCKED;\n"
            "}\n";
};

template <bool kHasCurveType = false>
struct GenericPreamble;

template <>
struct GenericPreamble<false> : public GenericPreambleBase {
    static inline constexpr std::string_view kBufferPreamble =
            "float calculate_arclength(float2 p0, float2 p1, float2 p2, float2 p3, float curveType) {\n"
            "    if (isinf(p3.y)) {\n"
            "        return eval_conic_arclength(p0, p1, p2, p3.x);\n"
            "    } else {\n"
            "        return eval_cubic_arclength(p0, p1, p2, p3);\n"
            "    }\n"
            "}\n";
};

template <>
struct GenericPreamble<true> : public GenericPreambleBase {
    static inline constexpr std::string_view kBufferPreamble =
            "float calculate_arclength(float2 p0, float2 p1, float2 p2, float2 p3, float curveType) {\n"
            "    if (curveType != 0.0) {\n"
            "        return eval_conic_arclength(p0, p1, p2, p3.x);\n"
            "    } else {\n"
            "        return eval_cubic_arclength(p0, p1, p2, p3);\n"
            "    }\n"
            "}\n";
};

using Preamble = GenericPreamble<false>;
using PreambleWithCurveType = GenericPreamble<true>;

struct Conversion {
    static inline constexpr std::string_view kScanType = "float";
    static inline constexpr std::string_view kPreamble =
            "float convertToScanType(uint p) {\n"
            "    return uintBitsToFloat(p << 2);\n"
            "}\n"
            "uint convertFromScanType(float v) {\n"
            "    return floatBitsToUint(v) >> 2;\n"
            "}\n"
            "float combine(float a, float b) {\n"
            "    return a + b;\n"
            "}\n";
};

struct LocalScan {
    static inline constexpr std::string_view kScan =
            "    float arc_length = 0.0;\n"
            "    StrokePatch patch;\n"
            "    if (curve_id < size) {\n"
            "        patch = patches[curve_id];\n"
            "        float2 p0 = patch.p01.xy;\n"
            "        float2 p1 = patch.p01.zw;\n"
            "        float2 p2 = patch.p23.xy;\n"
            "        float2 p3 = patch.p23.zw;\n"
            "        arc_length = calculate_arclength(p0, p1, p2, p3, patch.curveType);\n"
            "        uint path_idx = patch.pathIndex;\n"
            "        start_pred = path_attributes[path_idx] >= tile_id * PART_SIZE;\n"
            "    }\n"
            "    wg_partials[sk_LocalInvocationID.x] = arc_length;\n"
            "    workgroupBarrier();\n";

    static inline constexpr std::string_view kFanout =
            "    workgroupBarrier();\n"
            "    if (curve_id < size) {\n"
            "        uint path_idx = patches[curve_id].pathIndex;\n"
            "        uint start_idx = path_attributes[path_idx];\n"
            "        float add_val = (start_idx >= tile_id * PART_SIZE) ? 0.0 : wg_prev;\n"
            "        float t = wg_partials[sk_LocalInvocationID.x] + add_val;\n"
            "        patches[curve_id].t = t;\n"
            "        uint comp = 0u;\n"
            "        if (path_idx == num_paths - 1u) {\n"
            "            comp = size - 1u;\n"
            "        } else {\n"
            "            comp = path_attributes[path_idx + 1u] - 1u;\n"
            "        }\n"
            "        bool is_last = (curve_id == comp);\n"
            "        if (is_last) {\n"
            "            path_attributes[path_idx] = floatBitsToUint(t);\n"
            "        }\n"
            "    }\n";
};

struct WorkgroupScan {
    static inline constexpr std::string_view kScan =
            "    {\n"
            "        uint end = S_MEM_SIZE >> 1;\n"
            "        for (uint i = 1; i <= end; i <<= 1) {\n"
            "            float t;\n"
            "            bool above = sk_LocalInvocationID.x >= i;\n"
            "            if (above) {\n"
            "                t = wg_partials[sk_LocalInvocationID.x - i];\n"
            "            }\n"
            "            workgroupBarrier();\n"
            "\n"
            "            if (above) {\n"
            "                wg_partials[sk_LocalInvocationID.x] += t;\n"
            "            }\n"
            "            workgroupBarrier();\n"
            "        }\n"
            "    }\n"
            "    {\n"
            "        float local_subtract = 0.0;\n"
            "        if (curve_id < size) {\n"
            "            uint path_idx = patches[curve_id].pathIndex;\n"
            "            uint start_idx = path_attributes[path_idx];\n"
            "            uint tile_start_idx = tile_id * PART_SIZE;\n"
            "            if (start_idx > tile_start_idx) {\n"
            "                local_subtract = wg_partials[start_idx - 1u - tile_start_idx];\n"
            "            }\n"
            "        }\n"
            "        workgroupBarrier();\n"
            "        wg_partials[sk_LocalInvocationID.x] -= local_subtract;\n"
            "        workgroupBarrier();\n"
            "    }\n";

    static inline constexpr std::string_view kFallback =
            "                {\n"
            "                    float f_arc_length = 0.0;\n"
            "                    StrokePatch f_patch;\n"
            "                    if (f_curve_id < size) {\n"
            "                        f_patch = patches[f_curve_id];\n"
            "                        float2 p0 = f_patch.p01.xy;\n"
            "                        float2 p1 = f_patch.p01.zw;\n"
            "                        float2 p2 = f_patch.p23.xy;\n"
            "                        float2 p3 = f_patch.p23.zw;\n"
            "                        f_arc_length = calculate_arclength(p0, p1, p2, p3, f_patch.curveType);\n"
            "                        uint f_path_idx = f_patch.pathIndex;\n"
            "                        f_start_pred = path_attributes[f_path_idx] >= fallback_id * PART_SIZE;\n"
            "                    }\n"
            "                    wg_fallback[sk_LocalInvocationID.x] = f_arc_length;\n"
            "                    workgroupBarrier();\n"
            "\n"
            "                    uint end = S_MEM_SIZE >> 1;\n"
            "                    for (uint i = 1; i <= end; i <<= 1) {\n"
            "                        float t;\n"
            "                        bool above = sk_LocalInvocationID.x >= i;\n"
            "                        if (above) {\n"
            "                            t = wg_fallback[sk_LocalInvocationID.x - i];\n"
            "                        }\n"
            "                        workgroupBarrier();\n"
            "\n"
            "                        if (above) {\n"
            "                            wg_fallback[sk_LocalInvocationID.x] += t;\n"
            "                        }\n"
            "                        workgroupBarrier();\n"
            "                    }\n"
            "                }\n"
            "                {\n"
            "                    float f_local_subtract = 0.0;\n"
            "                    if (f_curve_id < size) {\n"
            "                        uint path_idx = patches[f_curve_id].pathIndex;\n"
            "                        uint start_idx = path_attributes[path_idx];\n"
            "                        uint f_tile_start_idx = fallback_id * PART_SIZE;\n"
            "                        if (start_idx > f_tile_start_idx) {\n"
            "                            f_local_subtract = wg_fallback[start_idx - 1u - f_tile_start_idx];\n"
            "                        }\n"
            "                    }\n"
            "                    workgroupBarrier();\n"
            "                    wg_fallback[sk_LocalInvocationID.x] -= f_local_subtract;\n"
            "                    workgroupBarrier();\n"
            "                }\n";
};

}  // namespace ArcLengthScan

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_ArcLengthScan_DEFINED
