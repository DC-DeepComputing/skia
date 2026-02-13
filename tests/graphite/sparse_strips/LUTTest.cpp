/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint.h"
#include "src/base/SkRandom.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "tests/Test.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>

namespace skgpu::graphite {

namespace {
    const std::array<uint8_t, 8> samplePattern8 = {0, 5, 3, 7, 1, 4, 6, 2};

    uint8_t computeNaiveMask(SkPoint p0, SkPoint p1) {
        float dx = p1.fX - p0.fX;
        float dy = p1.fY - p0.fY;
        if (std::abs(dx) < 1e-9f && std::abs(dy) < 1e-9f) return 0;
        float nx = dy;
        float ny = -dx;
        float len = std::hypot(nx, ny);
        nx /= len;
        ny /= len;

        if (nx < 0.0f) {
            nx = -nx;
            ny = -ny;
        }
        float c = nx * p0.fX + ny * p0.fY;

        uint8_t mask = 0;
        for (int i = 0; i < 8; ++i) {
            float sx = (samplePattern8[i] + 0.5f) / 8.0f;
            float sy = (i + 0.5f) / 8.0f;
            if (nx * sx + ny * sy - c > 0.0f) {
                mask |= (1 << i);
            }
        }
        return mask;
    }

    uint8_t lookupLut(SkPoint p0, SkPoint p1, const uint8_t* lut, float packing, int dim) {
        float dx = p1.fX - p0.fX;
        float dy = p1.fY - p0.fY;
        float nx = dy;
        float ny = -dx;

        float len = std::hypot(nx, ny);
        if (len < 1e-9f) len = 1e-9f;
        nx /= len;
        ny /= len;

        float c = nx * p0.fX + ny * p0.fY;
        c -= 0.5f * (nx + ny);

        float sign = (c < 0.0f) ? -1.0f : 1.0f;
        float c2 = std::max(0.0f, 1.0f - c * packing * sign);

        float nRevX = c2 * nx * sign;
        float nRevY = c2 * ny * sign;

        float uCoord = nRevX * 0.5f + 0.5f;
        float vCoord = nRevY * 0.5f + 0.5f;

        if (sign < 0.0f && uCoord == 0.5f) {
            uCoord = std::nextafter(0.5f, 0.0f);
        }

        int u = std::clamp((int)std::floor(uCoord * dim), 0, dim - 1);
        int v = std::clamp((int)std::floor(vCoord * dim), 0, dim - 1);

        return lut[v * dim + u];
    }

    static SkPoint pickRandomSquarePoint(SkRandom* rand) {
        int edge = rand->nextU() % 4;
        float t = rand->nextF();
        switch (edge) {
            case 0:
                return {t, 0.0f};
            case 1:
                return {1.0f, t};
            case 2:
                return {t, 1.0f};
            default:
                return {0.0f, t};
        }
    }
} // namespace anonymous

DEF_TEST(graphiteLutTestRandom, reporter) {
    const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();

    auto now = std::chrono::high_resolution_clock::now();
    uint32_t seed = static_cast<uint32_t>(now.time_since_epoch().count());
    SkRandom rand(seed);

    constexpr int iterations = 10000;
    constexpr uint32_t errorLimit = 3;
    std::array<uint32_t, errorLimit> minorErrorCount = {0, 0, 0};

    for (int i = 0; i < iterations; ++i) {
        SkPoint p0 = pickRandomSquarePoint(&rand);
        SkPoint p1 = pickRandomSquarePoint(&rand);

        uint8_t expected = computeNaiveMask(p0, p1);
        uint8_t actual = lookupLut(
                p0, p1, lut.begin(), MSAA_LUT<uint8_t>::kPackingScale, MSAA_LUT<uint8_t>::kDim);

        uint8_t diff = expected ^ actual;
        uint32_t bitErrors = 0;
        for (int b = 0; b < 8; ++b) {
            bitErrors += (diff >> b) & 1;
        }

        if (bitErrors > 0) {
            uint32_t hexP0X, hexP0Y, hexP1X, hexP1Y;
            std::memcpy(&hexP0X, &p0.fX, 4);
            std::memcpy(&hexP0Y, &p0.fY, 4);
            std::memcpy(&hexP1X, &p1.fX, 4);
            std::memcpy(&hexP1Y, &p1.fY, 4);

            if (bitErrors > errorLimit) {
                ERRORF(reporter,
                       "CRITICAL Fail at iter %d (Seed: %u). Expected 0x%02x, Got 0x%02x (%u bits "
                       "diff)\n"
                       "P0: %f, %f (0x%08x, 0x%08x)\n"
                       "P1: %f, %f (0x%08x, 0x%08x)",
                       i,
                       seed,
                       expected,
                       actual,
                       bitErrors,
                       p0.fX,
                       p0.fY,
                       hexP0X,
                       hexP0Y,
                       p1.fX,
                       p1.fY,
                       hexP1X,
                       hexP1Y);
            } else {
                minorErrorCount[bitErrors - 1]++;
            }
        }
    }

    INFOF(reporter,
          "Minor Error Summary (Seed: %u): 1-bit: %u, 2-bit: %u, 3-bit: %u",
          seed,
          minorErrorCount[0],
          minorErrorCount[1],
          minorErrorCount[2]);
}

}  // namespace skgpu::graphite
