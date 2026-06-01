/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <array>
#include <cmath>
#include <cstdint>

namespace skgpu::graphite {

namespace {

// Simplified point-in-polygon verification using Even-Odd rule.
bool IsInside(SkPoint pt, const Polyline& polyline) {
    bool inside = false;
    for (auto it = polyline.begin(); it != polyline.end(); ++it) {
        auto [line, idx] = *it;
        if ((line.p0.fY > pt.fY) != (line.p1.fY > pt.fY)) {
            float t = (pt.fY - line.p0.fY) / (line.p1.fY - line.p0.fY);
            float xInt = line.p0.fX + t * (line.p1.fX - line.p0.fX);
            if (pt.fX < xInt) {
                inside = !inside;
            }
        }
    }
    return inside;
}

}  // namespace

template <uint16_t TileW, uint16_t TileH>
class CoverageTestRunner {
public:
    using StripFunc = void (*)(const Tiles<TileW, TileH>&,
                               SkTDArray<Strip>&,
                               SkTDArray<uint8_t>&,
                               bool,
                               const Polyline&,
                               const SkTDArray<uint8_t>&,
                               MsaaExactMaskObserver);

    static void RunScalarWinding(const Tiles<TileW, TileH>& tileContainer,
                                 SkTDArray<Strip>& stripBuf,
                                 SkTDArray<uint8_t>& alphaBuf,
                                 bool isInverse,
                                 const Polyline& polyline,
                                 const SkTDArray<uint8_t>& maskLut,
                                 MsaaExactMaskObserver observer) {
        SkPathFillType fillType =
                isInverse ? SkPathFillType::kInverseWinding : SkPathFillType::kWinding;
        MakeStrips::MsaaScalar<TileW, TileH>(
                tileContainer, stripBuf, alphaBuf, fillType, polyline, maskLut, observer);
    }

    static void RunSimdWinding(const Tiles<TileW, TileH>& tileContainer,
                               SkTDArray<Strip>& stripBuf,
                               SkTDArray<uint8_t>& alphaBuf,
                               bool isInverse,
                               const Polyline& polyline,
                               const SkTDArray<uint8_t>& maskLut,
                               MsaaExactMaskObserver observer) {
        // Stub
    }

    CoverageTestRunner(StripFunc func, const char* implName) : fFunc(func), fImplName(implName) {}

    void runAll(skiatest::Reporter* reporter) {
        const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();
        constexpr int errorLimit = 3;
        std::array<uint32_t, errorLimit> minorErrorCount = {0, 0, 0};
        int totalTestsRun = 0;

        struct TestCase {
            SkPath path;
            const char* name;
        };
        std::vector<TestCase> baseGeometries;

        auto addRect = [&](float w, float h, const char* name) {
            baseGeometries.push_back(
                    {SkPathBuilder().addRect(SkRect::MakeWH(w, h)).detach(), name});
        };

        addRect(TileW * 0.5f, TileH * 0.5f, "Rect(Small)");
        addRect(TileW, TileH, "Rect(ExactTile)");
        addRect(TileW * 2.5f, TileH * 1.5f, "Rect(MultiTile)");
        addRect(TileW * 4.0f, 0.2f, "Rect(HorizSliver)");
        addRect(0.2f, TileH * 4.0f, "Rect(VertSliver)");

        baseGeometries.push_back(
                {SkPathBuilder().addCircle(TileW * 1.5f, TileH * 1.5f, TileW * 1.2f).detach(),
                 "Circle"});

        // TODO FIX ME! :)
        // baseGeometries.push_back({
        //     SkPathBuilder().addOval(SkRect::MakeWH(TileW * 4.0f, 0.5f)).detach(),
        //     "ThinOval"
        // });

        SkPathBuilder inset;
        inset.addRect(SkRect::MakeWH(TileW * 3.0f, TileH * 3.0f), SkPathDirection::kCW);
        inset.addRect(SkRect::MakeXYWH(TileW * 1.0f, TileH * 1.0f, TileW * 1.0f, TileH * 1.0f),
                      SkPathDirection::kCCW);
        baseGeometries.push_back({inset.detach(), "InsetRect"});

        // Tile-relative alignments (dx, dy)
        const SkPoint alignments[] = {
                {0.0f, 0.0f},                    // Top & Left aligned
                {0.0f, TileH * 0.5f},            // Left aligned, offset top
                {TileW * 0.5f, 0.0f},            // Top aligned, offset left
                {TileW * 0.33f, TileH * 0.33f},  // Strictly inside
                {TileW - 0.01f, TileH - 0.01f}   // Right on a tile boundary edge
        };

        for (const TestCase& geom : baseGeometries) {
            // Sweep through every single degree
            for (int angleDeg = 0; angleDeg < 360; ++angleDeg) {
                float angle = static_cast<float>(angleDeg);

                for (const SkPoint& alignment : alignments) {
                    SkMatrix rotMatrix;
                    rotMatrix.setRotate(angle);
                    SkRect rotatedBounds = geom.path.makeTransform(rotMatrix).getBounds();
                    SkMatrix transMatrix;
                    transMatrix.setTranslate(alignment.fX - rotatedBounds.fLeft,
                                             alignment.fY - rotatedBounds.fTop);
                    SkMatrix ctm = SkMatrix::Concat(transMatrix, rotMatrix);

                    SkPath deviceSpacePath = geom.path.makeTransform(ctm);

                    uint16_t reqWidth = 400;
                    uint16_t reqHeight = 400;

                    SkString testName;
                    testName.printf("%s - %s Rot(%.1f) Align(%.2f,%.2f)",
                                    fImplName,
                                    geom.name,
                                    angle,
                                    alignment.fX,
                                    alignment.fY);

                    if (!this->runSingleTest(reporter,
                                             deviceSpacePath,
                                             reqWidth,
                                             reqHeight,
                                             testName.c_str(),
                                             lut,
                                             minorErrorCount)) {
                        return;
                    }
                    totalTestsRun++;
                }
            }
        }

        INFOF(reporter,
              "[%s] Coverage LUT Test Complete. Ran %d variants."
              "Minor Error Summary: 1-sample: %u, 2-sample: %u, 3-sample: %u\n",
              fImplName,
              totalTestsRun,
              minorErrorCount[0],
              minorErrorCount[1],
              minorErrorCount[2]);
    }

private:
    StripFunc fFunc;
    const char* fImplName;

    static bool IsNearLine(SkPoint pt, const Polyline& polyline, float threshold) {
        for (auto it = polyline.begin(); it != polyline.end(); ++it) {
            auto [line, idx] = *it;
            float l2 = (line.p0.fX - line.p1.fX) * (line.p0.fX - line.p1.fX) +
                       (line.p0.fY - line.p1.fY) * (line.p0.fY - line.p1.fY);
            float dist = 0.0f;
            if (l2 == 0.0f) {
                float dx = pt.fX - line.p0.fX;
                float dy = pt.fY - line.p0.fY;
                dist = std::sqrt(dx * dx + dy * dy);
            } else {
                float t = std::max(0.0f,
                                   std::min(1.0f,
                                            ((pt.fX - line.p0.fX) * (line.p1.fX - line.p0.fX) +
                                             (pt.fY - line.p0.fY) * (line.p1.fY - line.p0.fY)) /
                                                    l2));
                float projX = line.p0.fX + t * (line.p1.fX - line.p0.fX);
                float projY = line.p0.fY + t * (line.p1.fY - line.p0.fY);
                dist = std::sqrt((pt.fX - projX) * (pt.fX - projX) +
                                 (pt.fY - projY) * (pt.fY - projY));
            }
            if (dist <= threshold) return true;
        }
        return false;
    }

    static void DumpDiagnostics(skiatest::Reporter* reporter,
                                const Polyline& polyline,
                                const Tiles<TileW, TileH>& tiler,
                                int failX,
                                int failY,
                                const SkTDArray<uint8_t>& actualMasks,
                                size_t tileStartIdx) {
        SkString out("\n--- FAILURE DIAGNOSTICS ---\n");

        out.append("Geometry Lines: {\n");
        for (auto it = polyline.begin(); it != polyline.end(); ++it) {
            auto [l, idx] = *it;
            out.appendf("  {{%f, %f}, {%f, %f}},\n", l.p0.fX, l.p0.fY, l.p1.fX, l.p1.fY);
        }
        out.append("}\n\n");

        out.append("Tile Intersections:\n");
        for (const auto& tile : tiler.getTiles()) {
            uint32_t mask = tile.intersectionMask();
            uint32_t lineIdx = tile.lineIdx();
            out.appendf("  Tile(%u,%u) Line %u Mask: %s\n",
                        tile.x,
                        tile.y,
                        lineIdx,
                        IntersectionBits::maskToString(mask).c_str());
        }

        out.appendf("\nASCII Map Tile(%d,%d) [Left: Expected | Right: Actual]\n",
                    failX / TileW,
                    failY / TileH);
        SkString border("+");
        for (int x = 0; x < TileW; ++x) border.append("----------------+");

        for (int y = 0; y < TileH; ++y) {
            out.appendf("%s   %s\n", border.c_str(), border.c_str());
            for (int sy = 0; sy < 8; ++sy) {
                for (int side = 0; side < 2; ++side) {  // 0: Expected, 1: Actual
                    out.append("|");
                    for (int x = 0; x < TileW; ++x) {
                        for (int sx = 0; sx < 8; ++sx) {
                            SkPoint cp = {failX + x + (sx + 0.5f) / 8.0f,
                                          failY + y + (sy + 0.5f) / 8.0f};
                            bool onLine = IsNearLine(cp, polyline, 0.6f / 8.0f);
                            bool active = false;

                            if (side == 0) {
                                active = IsInside(cp, polyline);
                            } else {
                                int32_t bufIdx = tileStartIdx + (y * TileW + x);
                                uint8_t mask =
                                        (bufIdx < actualMasks.size()) ? actualMasks[bufIdx] : 0;
                                active = (mask & (1 << sy));
                            }

                            if (MSAA_LUT<uint8_t>::kPattern[sy] == sx) {
                                out.append(active ? (onLine ? "*#" : " #")
                                                  : (onLine ? "*o" : " o"));
                            } else {
                                out.append(onLine ? "**" : "  ");
                            }
                        }
                        out.append("|");
                    }
                    if (side == 0) out.append("   ");
                }
                out.append("\n");
            }
        }
        out.appendf("%s   %s\n", border.c_str(), border.c_str());
        INFOF(reporter, "%s", out.c_str());
    }

    bool runSingleTest(skiatest::Reporter* reporter,
                       const SkPath& path,
                       uint16_t width,
                       uint16_t height,
                       const char* name,
                       const SkTDArray<uint8_t>& lut,
                       std::array<uint32_t, 3>& minorErrorCount) {
        Flatten flattener;
        Polyline polyline;
        flattener.processPaths<FlattenMode::kSimd>(
                path, SkMatrix(), static_cast<float>(width), static_cast<float>(height), &polyline);

        Tiles<TileW, TileH> tiler;
        tiler.makeTilesMSAA(polyline, width, height);
        tiler.sortTiles();

        SkTDArray<Strip> stripBuf;
        SkTDArray<uint8_t> alphaBuf;
        SkTDArray<uint8_t> exactMasks;

        auto observer = [&](uint8_t exactMask) { exactMasks.push_back(exactMask); };

        // Call the configured function pointer (SIMD or Scalar)
        fFunc(tiler, stripBuf, alphaBuf, /*isInverse=*/false, polyline, lut, observer);

        if (stripBuf.empty()) {
            bool bufferSizeMatch = alphaBuf.empty();
            REPORTER_ASSERT(
                    reporter, bufferSizeMatch, "[%s] No strips but alpha buffer has data.", name);
            return bufferSizeMatch;
        }

        int32_t alphaIdx = 0;

        for (int32_t i = 0; i < stripBuf.size() - 1; ++i) {
            const Strip& curr = stripBuf[i];
            const Strip& next = stripBuf[i + 1];

            uint32_t startIdx = curr.fFillAlphaIdx & 0x7FFFFFFF;
            uint32_t endIdx = next.fFillAlphaIdx & 0x7FFFFFFF;
            uint16_t spannedTiles = (endIdx - startIdx) / (TileW * TileH);

            uint16_t currX = curr.x;
            uint16_t currY = curr.y;

            for (int32_t s = 0; s < spannedTiles; ++s) {
                int32_t tileStartIdx = alphaIdx;
                for (int32_t y = 0; y < TileH; ++y) {
                    for (int32_t x = 0; x < TileW; ++x) {
                        uint8_t expectedMask = 0;
                        int expectedSamples = 0;
                        for (int k = 0; k < 8; ++k) {
                            if (IsInside({currX + x + (MSAA_LUT<uint8_t>::kPattern[k] + 0.5f) / 8.0f,
                                          currY + y + (k + 0.5f) / 8.0f},
                                         polyline)) {
                                expectedSamples++;
                                expectedMask |= (1 << k);
                            }
                        }

                        uint8_t actualMask =
                                (alphaIdx < exactMasks.size()) ? exactMasks[alphaIdx] : 0;
                        uint8_t actualAlpha = (alphaIdx < alphaBuf.size()) ? alphaBuf[alphaIdx] : 0;

                        int sampleDiff = 0;
                        int actualSamples = 0;
                        for (int k = 0; k < 8; ++k) {
                            if (actualMask & (1 << k)) actualSamples++;
                            if ((expectedMask & (1 << k)) != (actualMask & (1 << k))) sampleDiff++;
                        }

                        uint8_t expectedAlphaFromMask =
                                static_cast<uint8_t>((actualSamples * 255 + 4) / 8);
                        if (actualAlpha != expectedAlphaFromMask) {
                            REPORTER_ASSERT(
                                    reporter,
                                    false,
                                    "[%s] Alpha Reduction Mismatch at tile(%d,%d) pixel(%d,%d). "
                                    "Observer tracked %d active bits (expected alpha %d), "
                                    "but AlphaBuf output was %d.",
                                    name,
                                    currX / TileW,
                                    currY / TileH,
                                    x,
                                    y,
                                    actualSamples,
                                    expectedAlphaFromMask,
                                    actualAlpha);
                            return false;
                        }

                        if (sampleDiff > 3) {
                            DumpDiagnostics(reporter,
                                            polyline,
                                            tiler,
                                            currX,
                                            currY,
                                            exactMasks,
                                            tileStartIdx);
                            REPORTER_ASSERT(reporter,
                                            false,
                                            "[%s] Fail at tile(%d,%d). Exp %d, Got %d (alpha %d)",
                                            name,
                                            currX / TileW,
                                            currY / TileH,
                                            expectedSamples,
                                            actualSamples,
                                            actualAlpha);
                            return false;
                        } else if (sampleDiff > 0) {
                            minorErrorCount[sampleDiff - 1]++;
                        }

                        alphaIdx++;
                    }
                }
                currX += TileW;
            }
        }

        bool bufferSizeMatch = (alphaIdx == alphaBuf.size());
        REPORTER_ASSERT(reporter,
                        bufferSizeMatch,
                        "[%s] Checked %d alpha bytes but buffer size is %d",
                        name,
                        alphaIdx,
                        alphaBuf.size());
        return bufferSizeMatch;
    }
};

DEF_TEST(SparseStrips_CoverageScalar_4x4, reporter) {
    skgpu::graphite::CoverageTestRunner<4, 4> scalarRunner(
            &skgpu::graphite::CoverageTestRunner<4, 4>::RunScalarWinding, "Scalar");
    scalarRunner.runAll(reporter);
}

DEF_TEST(SparseStrips_CoverageSIMD_4x4, reporter) {
    skgpu::graphite::CoverageTestRunner<4, 4> simdRunner(
            &skgpu::graphite::CoverageTestRunner<4, 4>::RunSimdWinding, "SIMD");
    simdRunner.runAll(reporter);
}

DEF_TEST(SparseStrips_CoverageSIMD_8x8, reporter) {
    skgpu::graphite::CoverageTestRunner<8, 8> simdRunner(
            &skgpu::graphite::CoverageTestRunner<8, 8>::RunSimdWinding, "SIMD");
    simdRunner.runAll(reporter);
}

}  // namespace skgpu::graphite
