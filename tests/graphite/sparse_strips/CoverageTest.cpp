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
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace skgpu::graphite {

namespace {

const std::array<uint8_t, 8> samplePattern8 = {0, 5, 3, 7, 1, 4, 6, 2};

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

template <uint16_t TileW, uint16_t TileH> class CoverageTestRunner {
public:
    static void runAll(skiatest::Reporter* reporter) {
        const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();
        constexpr int errorLimit = 3;
        std::array<uint32_t, errorLimit> minorErrorCount = {0, 0, 0};
        int totalTestsRun = 0;

        // 1. Base geometries to torture test
        const SkRect baseRects[] = {
                SkRect::MakeWH(TileW * 0.5f, TileH * 0.5f),  // Small
                SkRect::MakeWH(TileW, TileH),                // Exactly one tile
                SkRect::MakeWH(TileW * 2.5f, TileH * 1.5f),  // Multi-tile fractional
                SkRect::MakeWH(TileW * 4.0f, 0.2f),          // Extremely thin horizontal sliver
                SkRect::MakeWH(0.2f, TileH * 4.0f),          // Extremely thin vertical sliver
        };

        // 2. Tile-relative alignments (dx, dy)
        const SkPoint alignments[] = {
                {0.0f, 0.0f},                    // Top & Left aligned
                {0.0f, TileH * 0.5f},            // Left aligned, offset top
                {TileW * 0.5f, 0.0f},            // Top aligned, offset left
                {TileW * 0.33f, TileH * 0.33f},  // Strictly inside
                {TileW - 0.01f, TileH - 0.01f}   // Right on a tile boundary edge
        };

        for (const SkRect& baseRect : baseRects) {
            // Sweep through every single degree
            for (int angleDeg = 0; angleDeg < 360; ++angleDeg) {
                float angle = static_cast<float>(angleDeg);

                for (const SkPoint& alignment : alignments) {
                    // Build base path (local space geometry)
                    SkPathBuilder pb;
                    pb.addRect(baseRect);
                    SkPath localPath = pb.detach();

                    // Create rotation matrix
                    SkMatrix rotMatrix;
                    rotMatrix.setRotate(angle);

                    // We need to know the bounds of the rotated geometry to calculate
                    // the exact translation required to satisfy the positive coordinate rule
                    // and our alignment offset.
                    SkRect rotatedBounds = localPath.makeTransform(rotMatrix).getBounds();

                    // Create translation matrix
                    SkMatrix transMatrix;
                    transMatrix.setTranslate(alignment.fX - rotatedBounds.fLeft,
                                             alignment.fY - rotatedBounds.fTop);

                    // Combine into a single CTM (Rotation followed by Translation)
                    SkMatrix ctm = SkMatrix::Concat(transMatrix, rotMatrix);

                    // Apply the transform exactly as the actual pipeline does
                    SkPath deviceSpacePath = localPath.makeTransform(ctm);

                    // Re-calculate bounds for Tiler sizing using the final device space path
                    SkRect bounds = deviceSpacePath.getBounds();
                    uint16_t reqWidth = static_cast<uint16_t>(std::ceil(bounds.fRight));
                    uint16_t reqHeight = static_cast<uint16_t>(std::ceil(bounds.fBottom));

                    // Naming for debug output
                    SkString testName;
                    testName.printf("Rect(%.1fx%.1f) Rot(%.1f) Align(%.2f,%.2f)",
                                    baseRect.width(),
                                    baseRect.height(),
                                    angle,
                                    alignment.fX,
                                    alignment.fY);

                    if (!RunSingleTest(reporter,
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
              "Coverage LUT Torture Test Complete. Ran %d variants.\n"
              "Minor Error Summary: 1-sample: %u, 2-sample: %u, 3-sample: %u",
              totalTestsRun,
              minorErrorCount[0],
              minorErrorCount[1],
              minorErrorCount[2]);
    }

private:
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

    static void DumpTileASCII(skiatest::Reporter* reporter,
                              const Polyline& polyline,
                              int tx,
                              int ty,
                              const std::vector<uint8_t>& exactMasks,
                              size_t tileAlphaStartIdx) {
        SkString out;
        out.appendf("\n--- ASCII Map for Tile(%d, %d) ---\n", tx, ty);
        out.append("Left: Expected Subpixels & Geometry (* = line, # = IN, o = OUT)\n");
        out.append("Right: Actual GPU Buffer (* = line, # = IN, o = OUT)\n");

        // Subpixel grid border: "+----------------+" (16 chars per pixel)
        SkString subPixelBorder("+");
        for (int x = 0; x < TileW; ++x) subPixelBorder.append("----------------+");

        // Draw the visualization row by row
        for (int y = 0; y < TileH; ++y) {
            out.appendf("%s   %s\n", subPixelBorder.c_str(), subPixelBorder.c_str());

            for (int sy = 0; sy < 8; ++sy) {
                // LEFT SIDE: Expected Subpixel Grid with Geometry
                out.append("|");
                for (int x = 0; x < TileW; ++x) {
                    for (int sx = 0; sx < 8; ++sx) {
                        float cellCenterPx = tx * TileW + x + (sx + 0.5f) / 8.0f;
                        float cellCenterPy = ty * TileH + y + (sy + 0.5f) / 8.0f;

                        bool onLine =
                                IsNearLine({cellCenterPx, cellCenterPy}, polyline, 0.6f / 8.0f);

                        if (samplePattern8[sy] == sx) {
                            if (IsInside({cellCenterPx, cellCenterPy}, polyline)) {
                                out.append(onLine ? "*#" : " #");
                            } else {
                                out.append(onLine ? "*o" : " o");
                            }
                        } else {
                            out.append(onLine ? "**" : "  ");
                        }
                    }
                    out.append("|");
                }

                // RIGHT SIDE: Actual Subpixel Grid from Observer
                out.append("   |");
                for (int x = 0; x < TileW; ++x) {
                    size_t bufIdx = tileAlphaStartIdx + (x * TileH + y);
                    uint8_t actualMask = (bufIdx < exactMasks.size()) ? exactMasks[bufIdx] : 0;

                    for (int sx = 0; sx < 8; ++sx) {
                        float cellCenterPx = tx * TileW + x + (sx + 0.5f) / 8.0f;
                        float cellCenterPy = ty * TileH + y + (sy + 0.5f) / 8.0f;

                        bool onLine =
                                IsNearLine({cellCenterPx, cellCenterPy}, polyline, 0.6f / 8.0f);

                        if (samplePattern8[sy] == sx) {
                            if (actualMask & (1 << sy)) {
                                out.append(onLine ? "*#" : " #");
                            } else {
                                out.append(onLine ? "*o" : " o");
                            }
                        } else {
                            out.append(onLine ? "**" : "  ");
                        }
                    }
                    out.append("|");
                }
                out.append("\n");
            }
        }
        out.appendf("%s   %s\n", subPixelBorder.c_str(), subPixelBorder.c_str());

        INFOF(reporter, "%s", out.c_str());
    }

    static bool RunSingleTest(skiatest::Reporter* reporter,
                              const SkPath& path,
                              uint16_t width,
                              uint16_t height,
                              const char* name,
                              const SkTDArray<uint8_t>& lut,
                              std::array<uint32_t, 3>& minorErrorCount) {
        Flatten flattener;
        Polyline polyline;
        flattener.processPaths<FlattenMode::kSimd>(
                path, static_cast<float>(width), static_cast<float>(height), polyline);

        Tiles<TileW, TileH> tiler;
        tiler.makeTilesMSAA(polyline, width, height);
        tiler.sortTiles();

        std::vector<Strip> stripBuf;
        std::vector<uint8_t> alphaBuf;
        std::vector<uint8_t> exactMasks;

        auto observer = [&](uint8_t exactMask) { exactMasks.push_back(exactMask); };

        Strip::MakeStripsMsaa(
                tiler, stripBuf, alphaBuf, SkPathFillType::kWinding, polyline, &lut, observer);

        const auto& tiles = tiler.getTiles();
        size_t alphaIdx = 0;

        int prevTx = -1;
        int prevTy = -1;

        for (size_t i = 0; i <= tiles.size(); ++i) {
            bool isSentinel = (i == tiles.size());
            uint16_t tx = isSentinel ? 0xFFFF : tiles[i].x;
            uint16_t ty = isSentinel ? 0xFFFF : tiles[i].y;

            if (i == 0) {
                prevTx = tx;
                prevTy = ty;
            } else if (tx != prevTx || ty != prevTy) {
                size_t tileAlphaStartIdx = alphaIdx;
                for (int x = 0; x < TileW; ++x) {
                    for (int y = 0; y < TileH; ++y) {
                        int expectedSamples = 0;
                        uint8_t expectedMask = 0;

                        for (int k = 0; k < Strip::kNumSubSamples; ++k) {
                            float sx = (samplePattern8[k] + 0.5f) / 8.0f;
                            float sy = (k + 0.5f) / 8.0f;
                            float px = prevTx * TileW + x + sx;
                            float py = prevTy * TileH + y + sy;

                            if (IsInside({px, py}, polyline)) {
                                expectedSamples++;
                                expectedMask |= (1 << k);
                            }
                        }

                        if (alphaIdx < alphaBuf.size()) {
                            uint8_t actualAlpha = alphaBuf[alphaIdx];
                            int sampleDiff = 0;
                            int actualSamples = 0;

                            uint8_t actualMask =
                                    (alphaIdx < exactMasks.size()) ? exactMasks[alphaIdx] : 0;

                            for (int k = 0; k < Strip::kNumSubSamples; ++k) {
                                bool expected = (expectedMask & (1 << k));
                                bool actual = (actualMask & (1 << k));
                                if (actual) actualSamples++;
                                if (expected != actual) {
                                    sampleDiff++;
                                }
                            }

                            if (sampleDiff > 3) {
                                DumpTileASCII(reporter,
                                              polyline,
                                              prevTx,
                                              prevTy,
                                              exactMasks,
                                              tileAlphaStartIdx);
                                REPORTER_ASSERT(reporter,
                                                false,
                                                "[%s] CRITICAL Fail at tile(%d,%d) local(%d,%d). "
                                                "Expected %d samples, got %d samples (alpha %d)",
                                                name,
                                                prevTx,
                                                prevTy,
                                                x,
                                                y,
                                                expectedSamples,
                                                actualSamples,
                                                actualAlpha);
                                return false;
                            } else if (sampleDiff > 0) {
                                minorErrorCount[sampleDiff - 1]++;
                            }
                        } else {
                            REPORTER_ASSERT(reporter,
                                            false,
                                            "[%s] alphaBuf out of bounds at alphaIdx %zu",
                                            name,
                                            alphaIdx);
                            return false;
                        }
                        alphaIdx++;
                    }
                }
                prevTx = tx;
                prevTy = ty;
            }
        }

        bool bufferSizeMatch = (alphaIdx == alphaBuf.size());
        REPORTER_ASSERT(reporter,
                        bufferSizeMatch,
                        "[%s] Checked %zu alpha bytes but buffer size is %zu",
                        name,
                        alphaIdx,
                        alphaBuf.size());
        return bufferSizeMatch;
    }
};

DEF_TEST(GraphiteCoverage_4x4, reporter) {
    skgpu::graphite::CoverageTestRunner<4, 4>::runAll(reporter);
}

}  // namespace skgpu::graphite
