/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkPoint.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/base/SkRandom.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConstants.h"
#include "src/gpu/graphite/sparse_strips/shaders/NonSubgroupWGSL.h"
#include "src/gpu/graphite/task/ComputeTask.h"

#include "tests/graphite/sparse_strips/compute/TestUtilsCompute.h"
#include "tools/graphite/GraphiteTestContext.h"

#include <cmath>
#include <vector>
#include <array>

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

enum class CoverageTestPhase {
    kLeftEdgeOnly,
    kSinglePixel,
    kFullRow,
    kFullTile
};

struct TestCaseGPU {
    float p0[2];
    float p1[2];
    float derivatives[4];
    uint32_t intersectionData;
    uint32_t canonicalXDir;
    uint32_t canonicalYDir;
    uint32_t pad;
};

template <uint16_t WIDTH, uint16_t HEIGHT>
SkPoint pick_random_tile_point(SkRandom* rand) {
    int edge = rand->nextU() % 4;
    float tW = rand->nextF() * static_cast<float>(WIDTH);
    float tH = rand->nextF() * static_cast<float>(HEIGHT);
    switch (edge) {
        case 0: return {tW, 0.0f}; // Top
        case 1: return {static_cast<float>(WIDTH), tH}; // Right
        case 2: return {tW, static_cast<float>(HEIGHT)}; // Bottom
        default: return {0.0f, tH}; // Left
    }
}

template <uint16_t WIDTH, uint16_t HEIGHT>
std::vector<TestCaseGPU> GeneratePixelCoverageData(uint32_t numTests, uint32_t seed) {
    SkRandom rand(seed);
    std::vector<TestCaseGPU> gpuInputs;
    gpuInputs.reserve(numTests);

    for (uint32_t i = 0; i < numTests; ++i) {
        SkPoint p0 = pick_random_tile_point<WIDTH, HEIGHT>(&rand);
        SkPoint p1 = pick_random_tile_point<WIDTH, HEIGHT>(&rand);

        float dx = p1.fX - p0.fX;
        float dy = p1.fY - p0.fY;

        bool isVertical = std::abs(dx) <= skgpu::graphite::SparseStripsConstants::kStripEpsilon;
        bool isHorizontal = std::abs(dy) <= skgpu::graphite::SparseStripsConstants::kStripEpsilon;

        float idx = isVertical ? 0.0f : 1.0f / dx;
        float idy = isHorizontal ? 0.0f : 1.0f / dy;

        TestCaseGPU gpuCase;
        gpuCase.p0[0] = p0.fX; gpuCase.p0[1] = p0.fY;
        gpuCase.p1[0] = p1.fX; gpuCase.p1[1] = p1.fY;

        gpuCase.derivatives[0] = dx; gpuCase.derivatives[1] = dy;
        gpuCase.derivatives[2] = idx; gpuCase.derivatives[3] = idy;

        uint32_t intersections = 0;
        if (p0.fX == 0.0f || p1.fX == 0.0f) intersections |= skgpu::graphite::SparseStripsConstants::L;
        if (p0.fY == 0.0f || p1.fY == 0.0f) intersections |= skgpu::graphite::SparseStripsConstants::T;
        if (p0.fX == static_cast<float>(WIDTH)  || p1.fX == static_cast<float>(WIDTH))  intersections |= skgpu::graphite::SparseStripsConstants::R;
        if (p0.fY == static_cast<float>(HEIGHT) || p1.fY == static_cast<float>(HEIGHT)) intersections |= skgpu::graphite::SparseStripsConstants::B;

        gpuCase.intersectionData = intersections;
        gpuCase.canonicalXDir = (dx >= 0.0f) ? 1 : 0;
        gpuCase.canonicalYDir = (dy >= 0.0f) ? 1 : 0;
        gpuCase.pad = 0;

        gpuInputs.push_back(gpuCase);
    }
    return gpuInputs;
}

// ============================================================================
// FULL TILE VERIFICATION
// ============================================================================

template <uint16_t WIDTH, uint16_t HEIGHT>
void verify_full_tile_result(skiatest::Reporter* reporter,
                             const TestCaseGPU& testCase,
                             const uint32_t* gpuOutput,
                             int testIdx,
                             std::array<uint32_t, 3>& errorCounts) {
    SkPoint p0 = {testCase.p0[0], testCase.p0[1]};
    SkPoint p1 = {testCase.p1[0], testCase.p1[1]};

    // 1. Establish the line's orientation
    bool exit_is_min = p1.fY < p0.fY;
    SkPoint clipped_top = exit_is_min ? p1 : p0;
    SkPoint clipped_bot = exit_is_min ? p0 : p1;
    bool xDir = clipped_top.fX <= clipped_bot.fX;

    int startY = static_cast<int>(std::floor(clipped_top.fY));
    int endY = static_cast<int>(std::ceil(clipped_bot.fY));

    float dxdy = (p1.fX - p0.fX) / (p1.fY - p0.fY);
    bool is_horizontal = std::abs(p1.fY - p0.fY) <= skgpu::graphite::SparseStripsConstants::kStripEpsilon;
    bool is_perfect_horizontal = is_horizontal && (clipped_top.fY == std::floor(clipped_top.fY));

    uint8_t expected[HEIGHT][WIDTH];
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            expected[y][x] = 0x00;
        }
    }

    // Line normal setup for naive mask
    float nx = (p1.fY - p0.fY);
    float ny = -(p1.fX - p0.fX);
    float len = std::hypot(nx, ny);
    nx /= len; ny /= len;
    if (nx < 0.0f) { nx = -nx; ny = -ny; }
    float c = nx * p0.fX + ny * p0.fY;

    for (int y = 0; y < HEIGHT; ++y) {
        // 2. Mirror kWgslLeftEdgeFill: Base state for the row below the intersection
        uint8_t row_base_mask = 0x00;
        if ((testCase.intersectionData & skgpu::graphite::SparseStripsConstants::L) != 0) {
            float y_edge = xDir ? clipped_top.fY : clipped_bot.fY;
            if (static_cast<float>(y) >= std::ceil(y_edge)) {
                row_base_mask = 0xFF;
            }
        }

        // Apply base mask to the whole row initially
        for (int x = 0; x < WIDTH; ++x) {
            expected[y][x] = row_base_mask;
        }

        // 3. Mirror WGSL bounds check
        if (is_perfect_horizontal || y < startY || y >= endY) {
            continue;
        }

        // 4. Calculate xStart / xEnd boundaries for this specific row
        bool is_startY = (y == startY);
        bool is_endY_minus_1 = (y == endY - 1);

        float py_top = is_startY ? clipped_top.fY : static_cast<float>(y);
        float py_bot = is_endY_minus_1 ? clipped_bot.fY : static_cast<float>(y + 1);

        float px_top = (is_horizontal || is_startY) ? clipped_top.fX : clipped_top.fX + (py_top - clipped_top.fY) * dxdy;
        float px_bot = (is_horizontal || is_endY_minus_1) ? clipped_bot.fX : clipped_top.fX + (py_bot - clipped_top.fY) * dxdy;

        float xMin = std::min(px_top, px_bot);
        float xMax = std::max(px_top, px_bot);

        uint32_t xStart = std::clamp(static_cast<uint32_t>(std::floor(xMin)), 0u, static_cast<uint32_t>(WIDTH - 1));
        uint32_t xEnd = std::clamp(static_cast<uint32_t>(std::floor(xMax)), 0u, static_cast<uint32_t>(WIDTH - 1));

        // 5. Calculate the Even/Odd state toggle (start_bumped) and boundary masks
        bool pixelTopTouch = (py_top != std::floor(py_top));
        bool default_bumped = pixelTopTouch && xDir;

        uint8_t start_mask_val = 0xFF;
        bool start_bumped = default_bumped;
        if (is_startY) {
            start_bumped = (px_top == 0.0f && pixelTopTouch);
            if (!start_bumped) {
                uint32_t shift = static_cast<uint32_t>(std::round(8.0f * (py_top - static_cast<float>(y))));
                start_mask_val = static_cast<uint8_t>((0xFF << shift) & 0xFF);
            }
        }

        uint8_t end_mask_val = 0xFF;
        if (is_endY_minus_1 && px_bot != 0.0f) {
            uint32_t shift = static_cast<uint32_t>(std::round(8.0f * (py_bot - static_cast<float>(y))));
            end_mask_val = static_cast<uint8_t>(~(0xFF << shift) & 0xFF);
        }

        // 6. March across the constrained X bounds and apply naive math + bumps
        for (uint32_t x = xStart; x <= xEnd; ++x) {
            uint8_t pixel_coverage = 0;

            for (int i = 0; i < 8; ++i) {
                float sx = static_cast<float>(x) + (MSAA_LUT<uint8_t>::kPattern[i] + 0.5f) / 8.0f;
                float sy = static_cast<float>(y) + (static_cast<float>(i) + 0.5f) / 8.0f;

                if (nx * sx + ny * sy - c > 0.0f) {
                    pixel_coverage |= (1 << i);
                }
            }

            uint8_t local_mask = 0xFF;
            bool local_bump = false;

            if (xStart == xEnd) {
                local_mask = start_mask_val & end_mask_val;
                local_bump = start_bumped;
            } else if (x == xStart) {
                local_mask = xDir ? start_mask_val : end_mask_val;
                local_bump = xDir ? start_bumped : default_bumped;
            } else if (x == xEnd) {
                local_mask = xDir ? end_mask_val : start_mask_val;
                local_bump = xDir ? default_bumped : start_bumped;
            } else {
                local_mask = 0xFF;
                local_bump = default_bumped;
            }

            pixel_coverage &= local_mask;
            uint8_t current_mask = row_base_mask ^ pixel_coverage;
            if (local_bump) {
                current_mask ^= 0xFF;
            }
            expected[y][x] = current_mask;
        }

        // 7. Post-Edge Fill (crossedTop)
        bool crossedTop = (y > startY) || (py_top == std::floor(py_top));
        if (crossedTop) {
            for (uint32_t x = xEnd + 1; x < WIDTH; ++x) {
                expected[y][x] = row_base_mask ^ 0xFF;
            }
        }
    }

    // 8. Compare with GPU output
    size_t baseOffset = testIdx * HEIGHT * WIDTH * 2;
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            size_t pixelOffset = baseOffset + (y * WIDTH + x) * 2;
            uint32_t mask_lo = gpuOutput[pixelOffset];
            uint32_t mask_hi = gpuOutput[pixelOffset + 1];

            uint8_t actual = 0;
            for (int i = 0; i < 4; ++i) {
                if ((mask_lo >> (i * 8)) & 1) actual |= (1 << i);
                if ((mask_hi >> (i * 8)) & 1) actual |= (1 << (i + 4));
            }

            uint8_t diff = expected[y][x] ^ actual;
            uint32_t bitErrors = std::popcount(diff);

            constexpr uint32_t kErrorLimit = 3;
            if (bitErrors > kErrorLimit) {
                ERRORF(reporter,
                       "Test %d: CRITICAL Tile mismatch at (%d, %d). Expected 0x%02X, Got 0x%02X (%u bits diff)\n"
                       "Line: (%f, %f) -> (%f, %f)",
                       testIdx, x, y, expected[y][x], actual, bitErrors, p0.fX, p0.fY, p1.fX, p1.fY);
                //return;
            } else if (bitErrors > 0) {
                errorCounts[bitErrors - 1]++;
            }
        }
    }
}

// ============================================================================
// LEFT EDGE VERIFICATION
// ============================================================================

template <uint16_t WIDTH, uint16_t HEIGHT>
void compute_left_edge_only_mask(const TestCaseGPU& tc, uint8_t expectedMasks[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            expectedMasks[y][x] = 0x00;
        }
    }

    if ((tc.intersectionData & skgpu::graphite::SparseStripsConstants::L) != 0) {
        bool exit_is_min = tc.p1[1] < tc.p0[1];
        float clipped_top_x = exit_is_min ? tc.p1[0] : tc.p0[0];
        float clipped_top_y = exit_is_min ? tc.p1[1] : tc.p0[1];
        float clipped_bot_x = exit_is_min ? tc.p0[0] : tc.p1[0];
        float clipped_bot_y = exit_is_min ? tc.p0[1] : tc.p1[1];

        bool xDir = clipped_top_x <= clipped_bot_x;
        float y_edge = xDir ? clipped_top_y : clipped_bot_y;

        int start_y = static_cast<int>(std::ceil(y_edge));

        for (int y = std::max(0, start_y); y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                expectedMasks[y][x] = 0xFF;
            }
        }
    }
}

template <uint16_t WIDTH, uint16_t HEIGHT>
void verify_left_edge_result(skiatest::Reporter* reporter,
                             const TestCaseGPU& testCase,
                             const uint32_t* gpuOutput,
                             int testIdx) {
    uint8_t expected[HEIGHT][WIDTH];
    compute_left_edge_only_mask<WIDTH, HEIGHT>(testCase, expected);

    size_t baseOffset = testIdx * HEIGHT * WIDTH * 2;

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            size_t pixelOffset = baseOffset + (y * WIDTH + x) * 2;
            uint32_t mask_lo = gpuOutput[pixelOffset];
            uint32_t mask_hi = gpuOutput[pixelOffset + 1];

            uint8_t actualMask = 0;
            for (int i = 0; i < 4; ++i) {
                if ((mask_lo >> (i * 8)) & 1) actualMask |= (1 << i);
                if ((mask_hi >> (i * 8)) & 1) actualMask |= (1 << (i + 4));
            }

            if (actualMask != expected[y][x]) {
                ERRORF(reporter,
                       "Test %d: Left Edge mismatch at pixel (%d, %d). "
                       "Expected: 0x%02X, Actual: 0x%02X\n"
                       "Line: (%f, %f) -> (%f, %f) [Intersection Data: %d]",
                       testIdx, x, y, expected[y][x], actualMask,
                       testCase.p0[0], testCase.p0[1], testCase.p1[0], testCase.p1[1],
                       testCase.intersectionData);
                return;
            }
        }
    }
}

// ============================================================================
// SINGLE PIXEL VERIFICATION
// ============================================================================

std::vector<TestCaseGPU> GenerateSinglePixelData(uint32_t numTests, uint32_t seed) {
    SkRandom rand(seed);
    std::vector<TestCaseGPU> gpuInputs;
    gpuInputs.reserve(numTests);

    for (uint32_t i = 0; i < numTests; ++i) {
        // Restrict points to a 1x1 pixel square
        auto pick_point = [&rand]() -> SkPoint {
            int edge = rand.nextU() % 4;
            float t = rand.nextF();
            switch (edge) {
                case 0: return {t, 0.0f};
                case 1: return {1.0f, t};
                case 2: return {t, 1.0f};
                default: return {0.0f, t};
            }
        };

        SkPoint p0 = pick_point();
        SkPoint p1 = pick_point();

        float dx = p1.fX - p0.fX;
        float dy = p1.fY - p0.fY;

        bool isVertical = std::abs(dx) <= skgpu::graphite::SparseStripsConstants::kStripEpsilon;
        bool isHorizontal = std::abs(dy) <= skgpu::graphite::SparseStripsConstants::kStripEpsilon;

        float idx = isVertical ? 0.0f : 1.0f / dx;
        float idy = isHorizontal ? 0.0f : 1.0f / dy;

        TestCaseGPU gpuCase = {};
        gpuCase.p0[0] = p0.fX; gpuCase.p0[1] = p0.fY;
        gpuCase.p1[0] = p1.fX; gpuCase.p1[1] = p1.fY;

        // CRITICAL: The WGSL reads these!
        gpuCase.derivatives[0] = dx; gpuCase.derivatives[1] = dy;
        gpuCase.derivatives[2] = idx; gpuCase.derivatives[3] = idy;
        gpuCase.canonicalXDir = (dx >= 0.0f) ? 1 : 0;
        gpuCase.canonicalYDir = (dy >= 0.0f) ? 1 : 0;

        // Since it's restricted to 1x1, intersection data is mostly irrelevant for
        // the single pixel test, but we should clear it.
        gpuCase.intersectionData = 0;

        gpuInputs.push_back(gpuCase);
    }
    return gpuInputs;
}

uint8_t compute_single_pixel_naive_mask(SkPoint p0, SkPoint p1) {
    float dx = p1.fX - p0.fX;
    float dy = p1.fY - p0.fY;
    if (std::abs(dx) < 1e-9f && std::abs(dy) < 1e-9f) return 0;

    float nx = dy;
    float ny = -dx;
    float len = std::hypot(nx, ny);
    nx /= len; ny /= len;

    if (nx < 0.0f) { nx = -nx; ny = -ny; }
    float c = nx * p0.fX + ny * p0.fY;

    uint8_t mask = 0;
    for (int i = 0; i < 8; ++i) {
        float sx = (MSAA_LUT<uint8_t>::kPattern[i] + 0.5f) / 8.0f;
        float sy = (i + 0.5f) / 8.0f;
        if (nx * sx + ny * sy - c > 0.0f) {
            mask |= (1 << i);
        }
    }
    return mask;
}

uint8_t apply_shader_bumping_rules(uint8_t raw_mask, SkPoint p0, SkPoint p1, uint16_t WIDTH) {
    bool exit_is_min = p1.fY < p0.fY;
    SkPoint clipped_top = exit_is_min ? p1 : p0;
    SkPoint clipped_bot = exit_is_min ? p0 : p1;

    int startY = static_cast<int>(std::floor(clipped_top.fY));
    int endY = static_cast<int>(std::ceil(clipped_bot.fY));

    // For the 1x1 test case, we are evaluating local_y = 0.
    // If the line doesn't span y=0 at all, the shader ignores it.
    if (0 < startY || 0 >= endY) {
        return 0;
    }

    bool is_startY = (0 == startY);
    bool is_endY_minus_1 = (0 == endY - 1);

    // Because is_startY and is_endY_minus_1 are true for the 1x1 tile at local_y=0,
    // px_top and px_bot perfectly equal the clipped X coordinates.
    float px_top = clipped_top.fX;
    float px_bot = clipped_bot.fX;

    uint32_t xStart = std::clamp(static_cast<uint32_t>(std::floor(std::min(px_top, px_bot))), 0u, static_cast<uint32_t>(WIDTH - 1));
    uint32_t xEnd   = std::clamp(static_cast<uint32_t>(std::floor(std::max(px_top, px_bot))), 0u, static_cast<uint32_t>(WIDTH - 1));

    bool xDir = clipped_top.fX <= clipped_bot.fX;
    bool pixelTopTouch = (clipped_top.fY != std::floor(clipped_top.fY));
    bool default_bumped = pixelTopTouch && xDir;

    uint8_t start_mask_val = 0xFF;
    bool start_bumped = default_bumped;
    if (is_startY) {
        start_bumped = (px_top == 0.0f && pixelTopTouch);
        if (!start_bumped) {
            uint32_t shift = static_cast<uint32_t>(std::round(8.0f * clipped_top.fY));
            start_mask_val = static_cast<uint8_t>((0xFF << shift) & 0xFF);
        }
    }

    uint8_t end_mask_val = 0xFF;
    if (is_endY_minus_1 && px_bot != 0.0f) {
        uint32_t shift = static_cast<uint32_t>(std::round(8.0f * clipped_bot.fY));
        end_mask_val = static_cast<uint8_t>(~(0xFF << shift) & 0xFF);
    }

    uint8_t local_mask = 0xFF;
    bool local_bump = false;

    if (xStart == xEnd) {
        local_mask = start_mask_val & end_mask_val;
        local_bump = start_bumped;
    } else {
        // Because xStart is bounded by min(px_top, px_bot) and points are in [0,1],
        // xStart is always 0. So local_x=0 always hits the WGSL "Left Edge" block.
        local_mask = xDir ? start_mask_val : end_mask_val;
        local_bump = xDir ? start_bumped : default_bumped;
    }

    uint8_t final_mask = raw_mask & local_mask;
    if (local_bump) {
        final_mask ^= 0xFF; // Execute the XOR bump!
    }

    return final_mask;
}

void verify_single_pixel_result(skiatest::Reporter* reporter,
                                const TestCaseGPU& testCase,
                                const uint32_t* gpuOutput,
                                int testIdx,
                                uint16_t WIDTH,
                                uint16_t HEIGHT,
                                std::array<uint32_t, 3>& errorCounts) {
    SkPoint p0 = {testCase.p0[0], testCase.p0[1]};
    SkPoint p1 = {testCase.p1[0], testCase.p1[1]};

    // Get the infinite-line naive mask
    uint8_t naive_mask = compute_single_pixel_naive_mask(p0, p1);

    // Apply the WGSL Y-clipping and XOR bumping rules
    uint8_t expected = apply_shader_bumping_rules(naive_mask, p0, p1, WIDTH);

    // Pixel (0,0) is at the very beginning of this test's output block
    size_t baseOffset = testIdx * HEIGHT * WIDTH * 2;
    uint32_t mask_lo = gpuOutput[baseOffset];
    uint32_t mask_hi = gpuOutput[baseOffset + 1];

    // Decode SWAR layout back to 8-bit mask
    uint8_t actual = 0;
    for (int i = 0; i < 4; ++i) {
        if ((mask_lo >> (i * 8)) & 1) actual |= (1 << i);
        if ((mask_hi >> (i * 8)) & 1) actual |= (1 << (i + 4));
    }

    uint8_t diff = expected ^ actual;
    uint32_t bitErrors = std::popcount(diff);

    constexpr uint32_t kErrorLimit = 3;
    if (bitErrors > kErrorLimit) {
        ERRORF(reporter,
                "Test %d: CRITICAL Single Pixel mismatch. Expected 0x%02x (Naive: 0x%02x), Got 0x%02x (%u bits diff)\n"
                "Line: (%f, %f) -> (%f, %f)",
                testIdx, expected, naive_mask, actual, bitErrors, p0.fX, p0.fY, p1.fX, p1.fY);
    } else if (bitErrors > 0) {
        errorCounts[bitErrors - 1]++;
    }
}

// ============================================================================
// FULL ROW VERIFICATION
// ============================================================================

template <uint16_t WIDTH, uint16_t HEIGHT>
std::vector<TestCaseGPU> GenerateFullRowData(uint32_t numTests, uint32_t seed) {
    SkRandom rand(seed);
    std::vector<TestCaseGPU> gpuInputs;
    gpuInputs.reserve(numTests);

    for (uint32_t i = 0; i < numTests; ++i) {
        // Force the line to perfectly span Y=0 to Y=1
        float x0 = rand.nextF() * static_cast<float>(WIDTH);
        float x1 = rand.nextF() * static_cast<float>(WIDTH);

        SkPoint p0 = {x0, 0.0f};
        SkPoint p1 = {x1, 1.0f};

        // Randomly swap direction so we test both y-up and y-down
        if (rand.nextBool()) {
            std::swap(p0, p1);
        }

        float dx = p1.fX - p0.fX;
        float dy = p1.fY - p0.fY;

        float idx = std::abs(dx) <= skgpu::graphite::SparseStripsConstants::kStripEpsilon ? 0.0f : 1.0f / dx;
        float idy = std::abs(dy) <= skgpu::graphite::SparseStripsConstants::kStripEpsilon ? 0.0f : 1.0f / dy;

        TestCaseGPU gpuCase;
        gpuCase.p0[0] = p0.fX; gpuCase.p0[1] = p0.fY;
        gpuCase.p1[0] = p1.fX; gpuCase.p1[1] = p1.fY;

        gpuCase.derivatives[0] = dx; gpuCase.derivatives[1] = dy;
        gpuCase.derivatives[2] = idx; gpuCase.derivatives[3] = idy;
        gpuCase.canonicalXDir = (dx >= 0.0f) ? 1 : 0;
        gpuCase.canonicalYDir = (dy >= 0.0f) ? 1 : 0;

        // Clear intersection data so left-edge logic is completely disabled
        gpuCase.intersectionData = 0;
        gpuCase.pad = 0;

        gpuInputs.push_back(gpuCase);
    }
    return gpuInputs;
}

template <uint16_t WIDTH, uint16_t HEIGHT>
void verify_full_row_result(skiatest::Reporter* reporter,
                            const TestCaseGPU& testCase,
                            const uint32_t* gpuOutput,
                            int testIdx,
                            std::array<uint32_t, 3>& errorCounts) {
    SkPoint p0 = {testCase.p0[0], testCase.p0[1]};
    SkPoint p1 = {testCase.p1[0], testCase.p1[1]};

    uint8_t expected[WIDTH] = {0};

    // Calculate DDA X-bounds
    float xMin = std::min(p0.fX, p1.fX);
    float xMax = std::max(p0.fX, p1.fX);
    int xStart = std::clamp(static_cast<int>(std::floor(xMin)), 0, WIDTH - 1);
    int xEnd   = std::clamp(static_cast<int>(std::floor(xMax)), 0, WIDTH - 1);

    // 1. Pixels before xStart remain 0x00.

    // 2. Pixels touching the line get the naive mask.
    for (int x = xStart; x <= xEnd; ++x) {
        uint8_t mask = 0;
        float nx = (p1.fY - p0.fY);
        float ny = -(p1.fX - p0.fX);
        float len = std::hypot(nx, ny);
        nx /= len; ny /= len;
        if (nx < 0.0f) { nx = -nx; ny = -ny; }
        float c = nx * p0.fX + ny * p0.fY;

        for (int i = 0; i < 8; ++i) {
            float sx = static_cast<float>(x) + (MSAA_LUT<uint8_t>::kPattern[i] + 0.5f) / 8.0f;
            float sy = 0.0f + (static_cast<float>(i) + 0.5f) / 8.0f; // Evaluating row Y=0
            if (nx * sx + ny * sy - c > 0.0f) {
                mask |= (1 << i);
            }
        }
        expected[x] = mask;
    }

    // 3. Pixels after xEnd hit the crossedTop fill.
    for (int x = xEnd + 1; x < WIDTH; ++x) {
        expected[x] = 0xFF;
    }

    // Compare with GPU output for Row 0
    size_t baseOffset = testIdx * HEIGHT * WIDTH * 2;
    for (int x = 0; x < WIDTH; ++x) {
        size_t pixelOffset = baseOffset + (x * 2);
        uint32_t mask_lo = gpuOutput[pixelOffset];
        uint32_t mask_hi = gpuOutput[pixelOffset + 1];

        uint8_t actual = 0;
        for (int i = 0; i < 4; ++i) {
            if ((mask_lo >> (i * 8)) & 1) actual |= (1 << i);
            if ((mask_hi >> (i * 8)) & 1) actual |= (1 << (i + 4));
        }

        uint8_t diff = expected[x] ^ actual;
        uint32_t bitErrors = std::popcount(diff);

        if (bitErrors > 3) {
            ERRORF(reporter,
                    "Test %d: CRITICAL Row mismatch at X=%d. Expected 0x%02X, Got 0x%02X (%u bits diff)\n"
                    "Line: (%f, %f) -> (%f, %f)",
                    testIdx, x, expected[x], actual, bitErrors, p0.fX, p0.fY, p1.fX, p1.fY);
            return;
        } else if (bitErrors > 0) {
            errorCounts[bitErrors - 1]++;
        }
    }
}

}  // namespace

// ============================================================================
// PARAMETERIZED TEST RUNNER
// ============================================================================

template <uint16_t WIDTH, uint16_t HEIGHT>
class GPUCoverageTestRunner {
public:
    static void runAll(skiatest::Reporter* reporter,
                       Context* context,
                       GraphiteTestContext* testContext,
                       CoverageTestPhase phase) {
        if (WIDTH != skgpu::graphite::SparseStripsConfig::kTileWidth ||
            HEIGHT != skgpu::graphite::SparseStripsConfig::kTileHeight) {
            return;
        }
        constexpr uint32_t kNumTests = 1000;
        constexpr uint32_t kWorkgroupSize = 256;
        std::vector<TestCaseGPU> gpuInputs;
        if (phase == CoverageTestPhase::kSinglePixel) {
            gpuInputs = GenerateSinglePixelData(kNumTests, 42);
        } else if (phase == CoverageTestPhase::kFullRow) {
            gpuInputs = GenerateFullRowData<WIDTH, HEIGHT>(kNumTests, 42);
        } else {
            gpuInputs = GeneratePixelCoverageData<WIDTH, HEIGHT>(kNumTests, 42);
        }
        const SkTDArray<uint8_t> msaaLUT = GenerateMSAALUT<uint8_t>();

        class CoverageComputeStep : public ComputeStep {
        public:
            CoverageComputeStep(const std::vector<TestCaseGPU>& testData,
                                const SkTDArray<uint8_t>& lutData,
                                CoverageTestPhase phase)
                : ComputeStep(
                    "SparseStripsParameterizedTest",
                    {kWorkgroupSize, 1, 1},
                    {
                        {ResourceType::kUniformBuffer, DataFlow::kShared, ResourcePolicy::kMapped, 0,
                         "testInfo { num_tests: u32, pad0: u32, pad1: u32, pad2: u32 }"},
                        {ResourceType::kReadOnlyStorageBuffer, DataFlow::kShared, ResourcePolicy::kMapped, 1,
                         "msaa_lutBlock { msaa_lut: array<u32> }"},
                        {ResourceType::kReadOnlyStorageBuffer, DataFlow::kShared, ResourcePolicy::kMapped, 2,
                         "inputBlock { in_data: array<TestCaseGPU> }"},
                        {ResourceType::kStorageBuffer, DataFlow::kShared, ResourcePolicy::kMapped, 3,
                         "outputBlock { out_data: array<vec2<u32>> }"}
                    },
                    {},
                    Flags::kSupportsNativeShader),
                  fTestData(testData),
                  fLutData(lutData) {

                fShaderText = GetWgslConstants();
                fShaderText += GetWgslConfigDerived();

                fShaderText += R"(
                    struct TestCaseGPU {
                        p0: vec2<f32>,
                        p1: vec2<f32>,
                        derivatives: vec4<f32>,
                        intersectionData: u32,
                        canonicalXDir: u32,
                        canonicalYDir: u32,
                        pad: u32,
                    };
                    struct PreMergeTile {
                        packed_scanned_winding: u32,
                    };
                    struct TestInfo { num_tests: u32, pad0: u32, pad1: u32, pad2: u32 };

                    @group(0) @binding(0) var<uniform> test_info: TestInfo;
                    @group(0) @binding(1) var<storage, read> msaa_lut: array<u32>;
                    @group(0) @binding(2) var<storage, read> in_data: array<TestCaseGPU>;
                    @group(0) @binding(3) var<storage, read_write> out_data: array<vec2<u32>>;
                )";

                fShaderText += kWgslHelpers;

                fShaderText += "@compute @workgroup_size(" + std::to_string(kWorkgroupSize) + ", 1, 1)\n";
                fShaderText += R"(
                    fn main(@builtin(global_invocation_id) global_id: vec3<u32>,
                            @builtin(local_invocation_id) tid: vec3<u32>) {

                        let total_threads = test_info.num_tests * TILE_HEIGHT;
                        if (global_id.x >= total_threads) { return; }

                        let test_idx = global_id.x / TILE_HEIGHT;
                        let tc = in_data[test_idx];

                        var pmt: PreMergeTile;
                        pmt.packed_scanned_winding = 0u;

                        let p0 = tc.p0;
                        let p1 = tc.p1;
                        let d = p1 - p0;

                        let dxdy = tc.derivatives.x * tc.derivatives.w;
                        let dydx = tc.derivatives.y * tc.derivatives.z;
                        let canonical_x_dir = tc.canonicalXDir != 0u;
                        let canonical_y_dir = tc.canonicalYDir != 0u;
                        let intersection_data = tc.intersectionData;

                        let is_horizontal = abs(d.y) <= EPSILON;
                        let is_vertical = abs(d.x) <= EPSILON;

                        let exit_is_min = p1.y < p0.y;
                        let clipped_top = select(p0, p1, exit_is_min);
                        let clipped_bot = select(p1, p0, exit_is_min);
                )";

                fShaderText += kWgslRowSetup;

                if (phase == CoverageTestPhase::kFullTile || phase == CoverageTestPhase::kLeftEdgeOnly) {
                    fShaderText += kWgslLeftEdgeFill;
                }

                if (phase != CoverageTestPhase::kLeftEdgeOnly) {
                    fShaderText += kWgslMaskGen_SlopeIntercept;
                }

                fShaderText += R"(
                        // Write current thread's row (mask array) to the global output buffer
                        let base_out_idx = test_idx * TILE_HEIGHT * TILE_WIDTH + local_y * TILE_WIDTH;
                        for (var x = 0u; x < TILE_WIDTH; x += 1u) {
                            out_data[base_out_idx + x] = mask[x];
                        }
                    }
                )";
            }

            NativeShaderSource nativeShaderSource(NativeShaderFormat format) const override {
                SkASSERT(format == NativeShaderFormat::kWGSL);
                return {fShaderText, "main"};
            }

            size_t calculateBufferSize(int index, const ResourceDesc& r) const override {
                if (index == 1) return fLutData.size() * sizeof(uint8_t);
                if (index == 2) return sizeof(TestCaseGPU) * fTestData.size();
                if (index == 3) return sizeof(uint32_t) * 2 * WIDTH * HEIGHT * fTestData.size();
                return 0;
            }

            WorkgroupSize calculateGlobalDispatchSize() const override {
                uint32_t totalThreads = fTestData.size() * HEIGHT;
                uint32_t groups = (totalThreads + kWorkgroupSize - 1) / kWorkgroupSize;
                return WorkgroupSize(groups, 1, 1);
            }

            void prepareStorageBuffer(int resourceIndex,
                                      const ResourceDesc&,
                                      skgpu::BufferWriter&& writer) const override {
                if (resourceIndex == 1) {
                    writer.write(fLutData.data(), fLutData.size() * sizeof(uint8_t));
                } else if (resourceIndex == 2) {
                    writer.write(fTestData.data(), fTestData.size() * sizeof(TestCaseGPU));
                }
            }

            void prepareUniformBuffer(int resourceIndex,
                                      const ResourceDesc&,
                                      UniformManager* manager) const override {
                if (resourceIndex == 0) {
                    manager->write(static_cast<uint32_t>(fTestData.size()));
                    manager->write(0u); manager->write(0u); manager->write(0u);
                }
            }

        private:
            const std::vector<TestCaseGPU>& fTestData;
            const SkTDArray<uint8_t>& fLutData;
            std::string fShaderText;
        };

        CoverageComputeStep step(gpuInputs, msaaLUT, phase);
        std::unique_ptr<Recorder> recorder = context->makeRecorder();
        DispatchGroup::Builder builder(recorder.get());
        if (!builder.appendStep(&step)) {
            ERRORF(reporter, "Failed to add ComputeStep");
            return;
        }

        BindBufferInfo outputInfo = builder.getSharedBufferResource(3);

        ComputeTask::DispatchGroupList groups;
        groups.push_back(builder.finalize());
        recorder->priv().add(ComputeTask::Make(std::move(groups)));

        auto outputBuffer = sync_buffer_to_cpu(recorder.get(), outputInfo.fBuffer);
        submit_recording(context, testContext, recorder.get());

        const uint32_t* outData = static_cast<const uint32_t*>(
                map_buffer(context, testContext, outputBuffer.get(), outputInfo.fOffset));
        SkASSERT(outputBuffer->isMapped() && outData != nullptr);

        std::array<uint32_t, 3> errorCounts = {0, 0, 0};

        for (size_t i = 0; i < 5; ++i) {
            if (phase == CoverageTestPhase::kLeftEdgeOnly) {
                verify_left_edge_result<WIDTH, HEIGHT>(reporter, gpuInputs[i], outData, i);
            } else if (phase == CoverageTestPhase::kSinglePixel) {
                verify_single_pixel_result(reporter, gpuInputs[i], outData, i, WIDTH, HEIGHT, errorCounts);
            } else if (phase == CoverageTestPhase::kFullRow) {
                verify_full_row_result<WIDTH, HEIGHT>(reporter, gpuInputs[i], outData, i, errorCounts);
            } else if (phase == CoverageTestPhase::kFullTile) {
                verify_full_tile_result<WIDTH, HEIGHT>(reporter, gpuInputs[i], outData, i, errorCounts);
            }
        }

        if (phase != CoverageTestPhase::kLeftEdgeOnly) {
            INFOF(reporter, "[%d] Minor Quantization Errors: 1-bit: %u, 2-bit: %u, 3-bit: %u",
                  static_cast<int>(phase), errorCounts[0], errorCounts[1], errorCounts[2]);
        }

        if (outputBuffer->isMapped()) {
            outputBuffer->unmap();
        }
    }
};

// ============================================================================
// DAWN TEST MACROS
// ============================================================================

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_LeftEdgeOnly4x4, reporter, context, testContext) {
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) { return; }
    GPUCoverageTestRunner<4, 4>::runAll(reporter, context, testContext, CoverageTestPhase::kLeftEdgeOnly);
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_LeftEdgeOnly8x8, reporter, context, testContext) {
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) { return; }
    GPUCoverageTestRunner<8, 8>::runAll(reporter, context, testContext, CoverageTestPhase::kLeftEdgeOnly);
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_SinglePixel4x4, reporter, context, testContext) {
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) { return; }
    GPUCoverageTestRunner<4, 4>::runAll(reporter, context, testContext, CoverageTestPhase::kSinglePixel);
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_SinglePixel8x8, reporter, context, testContext) {
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) { return; }
    GPUCoverageTestRunner<8, 8>::runAll(reporter, context, testContext, CoverageTestPhase::kSinglePixel);
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_FullRow4x4, reporter, context, testContext) {
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) { return; }
    GPUCoverageTestRunner<4, 4>::runAll(reporter, context, testContext, CoverageTestPhase::kFullRow);
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_FullRow8x8, reporter, context, testContext) {
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) { return; }
    GPUCoverageTestRunner<8, 8>::runAll(reporter, context, testContext, CoverageTestPhase::kFullRow);
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_CoverageSimple4x4, reporter, context, testContext) {
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) { return; }
    GPUCoverageTestRunner<4, 4>::runAll(reporter, context, testContext, CoverageTestPhase::kFullTile);
}

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_CoverageSimple8x8, reporter, context, testContext) {
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) { return; }
    GPUCoverageTestRunner<8, 8>::runAll(reporter, context, testContext, CoverageTestPhase::kFullTile);
}
