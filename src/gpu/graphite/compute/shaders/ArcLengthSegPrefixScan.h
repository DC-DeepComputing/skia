/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_shaders_ArcLengthSegPrefixScan_DEFINED
#define skgpu_graphite_compute_shaders_ArcLengthSegPrefixScan_DEFINED

#include <string_view>

namespace skgpu::graphite {

// Contains the policies for the segmented prefix sum over arc lengths.
namespace ArcLengthSegPrefixScan {

// Approximate length of cubic Bezier curve using Gauss-Legendre / Gauss-Lobatto quadrature with n=5
// points. Evaluates derivative magnitude |C'(t)| at sample points t in {0, 0.5 ± sqrt(3/7)/2, 0.5,
// 1}, weighted 1/20, 49/180, 32/90, 49/180, 1/20 respectively. Linear combinations of control
// points are pre-factored into compile-time constants. We assume that CPU preprocessing
// (FindCubicConvex180Chops) has already eliminated 180° cusps and inflection points.
// https://en.wikipedia.org/wiki/Gaussian_quadrature#Gauss%E2%80%93Lobatto_rules

// Approximates a rational conic using a cubic bezier. This implements the G1 mid-point
// approximation from Proposition 5 of:
// Floater, M. S. (1996). An analysis of cubic approximation schemes for conic sections.
// Advances in Computational Mathematics, 5(1), 361–379. https://doi.org/10.1007/bf02124751
// Then, the cubic arc length is evaluated using the 5-point Gauss-Lobatto quadrature.

struct GenericPreambleBase {
    static inline constexpr std::string_view kPreamble =
            "const uint BLOCK_DIM = 256u;\n"
            "const uint PART_SIZE = BLOCK_DIM;\n"
            "const uint S_MEM_SIZE = BLOCK_DIM;\n"
            "\n"
            "float eval_cubic_arclength(float2 d1, float2 d2, float2 d3, float2 d3d2) {\n"
            "    float v0 = length(d1) * 0.15;\n"
            "    float v1 = length(0.325650248872424 * d1 + 0.208983582205757 * d2 + 0.024349751127576 * d3);\n"
            "    float v2 = length(d3 + d2 - d1) * 0.26666666666666666;\n"
            "    float v3 = length(-0.208983582205757 * d1 - 0.325650248872424 * d2 + 0.558983582205757 * d3);\n"
            "    float v4 = length(d3d2) * 0.15;\n"
            "    return v0 + v1 + v2 + v3 + v4;\n"
            "}\n"
            "\n";
};

template <bool kHasCurveType = false>
struct GenericPreamble;

template <>
struct GenericPreamble<false> : public GenericPreambleBase {
    static inline constexpr std::string_view kBufferPreamble =
            "float calculate_arclength(float2 p0, float2 p1, float2 p2, float2 p3, float curveType) {\n"
            "    float2 d1;\n"
            "    float2 d2;\n"
            "    float2 d3;\n"
            "    float2 d3d2;\n"
            "    if (isinf(p3.y)) {\n"
            "        float w = p3.x;\n"
            "        float k = (4.0 / 3.0) * (w / (1.0 + w));\n"
            "        float2 v1 = p1 - p0;\n"
            "        float2 v2 = p2 - p0;\n"
            "        d1 = k * v1;\n"
            "        d2 = v2 + k * (v1 - v2);\n"
            "        d3 = v2;\n"
            "        d3d2 = k * (p2 - p1);\n"
            "    } else {\n"
            "        d1 = p1 - p0;\n"
            "        d2 = p2 - p0;\n"
            "        d3 = p3 - p0;\n"
            "        d3d2 = p3 - p2;\n"
            "    }\n"
            "    return eval_cubic_arclength(d1, d2, d3, d3d2);\n"
            "}\n";
};

template <>
struct GenericPreamble<true> : public GenericPreambleBase {
    static inline constexpr std::string_view kBufferPreamble =
            "float calculate_arclength(float2 p0, float2 p1, float2 p2, float2 p3, float curveType) {\n"
            "    float2 d1;\n"
            "    float2 d2;\n"
            "    float2 d3;\n"
            "    float2 d3d2;\n"
            "    if (curveType != 0.0) {\n"
            "        float w = p3.x;\n"
            "        float k = (4.0 / 3.0) * (w / (1.0 + w));\n"
            "        float2 v1 = p1 - p0;\n"
            "        float2 v2 = p2 - p0;\n"
            "        d1 = k * v1;\n"
            "        d2 = v2 + k * (v1 - v2);\n"
            "        d3 = v2;\n"
            "        d3d2 = k * (p2 - p1);\n"
            "    } else {\n"
            "        d1 = p1 - p0;\n"
            "        d2 = p2 - p0;\n"
            "        d3 = p3 - p0;\n"
            "        d3d2 = p3 - p2;\n"
            "    }\n"
            "    return eval_cubic_arclength(d1, d2, d3, d3d2);\n"
            "}\n";
};

using Preamble = GenericPreamble<false>;
using PreambleWithCurveType = GenericPreamble<true>;

struct Conversion {
    static inline constexpr std::string_view kPrefixScanType = "float";
    static inline constexpr std::string_view kPreamble =
            "float convertToPrefixScanType(uint p) {\n"
            "    return uintBitsToFloat(p << 2);\n"
            "}\n"
            "uint convertFromPrefixScanType(float v) {\n"
            "    return floatBitsToUint(v) >> 2;\n"
            "}\n"
            "float combine(float a, float b) {\n"
            "    return a + b;\n"
            "}\n";
};

struct LocalPrefixScan {
    static inline constexpr std::string_view kPrefixScan =
            "    float arc_length = 0.0;\n"
            "    StrokePatch patch;\n"
            "    if (curve_id < size) {\n"
            "        patch = patches[curve_id];\n"
            "        float2 p0 = patch.p01.xy;\n"
            "        float2 p1 = patch.p01.zw;\n"
            "        float2 p2 = patch.p23.xy;\n"
            "        float2 p3 = patch.p23.zw;\n"
            "        arc_length = calculate_arclength(p0, p1, p2, p3, patch.curveType);\n"
            "        is_inclusive = pathData[patch.pathIndex].startIndex >= tile_id * PART_SIZE;\n"
            "    }\n"
            "    wg_partials[sk_LocalInvocationID.x] = arc_length;\n"
            "    workgroupBarrier();\n";

    static inline constexpr std::string_view kFanout =
            "    workgroupBarrier();\n"
            "    if (curve_id < size) {\n"
            "        float t = wg_partials[sk_LocalInvocationID.x] + (is_inclusive ? 0.0 : wg_prev);\n"
            "        patches[curve_id].t = t;\n"
            "        uint comp = 0u;\n"
            "        if (patch.pathIndex == num_paths - 1u) {\n"
            "            comp = size - 1u;\n"
            "        } else {\n"
            "            comp = pathData[patch.pathIndex + 1u].startIndex - 1u;\n"
            "        }\n"
            "        bool is_last = (curve_id == comp);\n"
            "        if (is_last) {\n"
            "            pathData[patch.pathIndex].totalLength = t;\n"
            "        }\n"
            "    }\n";
};

struct WorkgroupPrefixScan {
    static inline constexpr std::string_view kPrefixScan =
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
            "            uint start_idx = pathData[path_idx].startIndex;\n"
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
            "                        f_is_inclusive = pathData[f_path_idx].startIndex >= fallback_id * PART_SIZE;\n"
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
            "                        uint start_idx = pathData[path_idx].startIndex;\n"
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

}  // namespace ArcLengthSegPrefixScan

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_shaders_ArcLengthSegPrefixScan_DEFINED
