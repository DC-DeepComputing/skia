/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED
#define skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkString.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <cmath>
#include <cstdint>
#include <vector>

namespace skgpu::graphite {

// -------------------------------------------------------------------------------------------------
// 1. ScanlineOracle8x: Direct SkPath curve root-solving 8x MSAA reference oracle
// -------------------------------------------------------------------------------------------------
class ScanlineOracle8x {
public:
    struct CurveIntersection {
        skvx::float8 x;
        skvx::int8 dir;  // +1 for downward, -1 for upward, 0 for no hit in this lane
    };

    struct RowWindingInterval {
        int32_t x;
        skvx::int8 winding;  // 8-element subsample winding vector starting at x until next interval
    };

    static void IntersectLine(const SkPoint pts[2],
                              skvx::float8 targetY,
                              std::vector<CurveIntersection>* out);
    static void IntersectQuad(const SkPoint pts[3],
                              skvx::float8 targetY,
                              std::vector<CurveIntersection>* out);
    static void IntersectConic(const SkPoint pts[3],
                               float weight,
                               skvx::float8 targetY,
                               std::vector<CurveIntersection>* out);
    static void IntersectCubic(const SkPoint pts[4],
                               skvx::float8 targetY,
                               std::vector<CurveIntersection>* out);

    static void CollectPathIntersections(const SkPath& path,
                                         skvx::float8 targetY,
                                         std::vector<CurveIntersection>* out);

    static skvx::int8 EvaluatePixelWinding(float px, const std::vector<CurveIntersection>& hits);

    static std::vector<RowWindingInterval> BuildRowIntervals(int py, const SkPath& path);

    static skvx::int8 Evaluate(SkPoint pt, const SkPath& path);
};

template <uint16_t kTileWidth, uint16_t kTileHeight> class StripStreamValidator {
public:
    StripStreamValidator(const SkPath& path,
                         const SkTDArray<Strip>& stripBuf,
                         const SkTDArray<uint8_t>& alphaBuf,
                         const SkTDArray<skvx::int8>& exactWindings,
                         const Polyline* polyline = nullptr,
                         const Tiles<kTileWidth, kTileHeight>* tiler = nullptr,
                         const char* testName = "StripStreamValidator")
            : fPath(path)
            , fStrips(stripBuf)
            , fAlphaBuf(alphaBuf)
            , fExactWindings(exactWindings)
            , fPolyline(polyline)
            , fTiler(tiler)
            , fTestName(testName) {}

    static skvx::int8 GetOracleWinding(
            int px, const std::vector<ScanlineOracle8x::RowWindingInterval>& intervals) {
        if (intervals.empty() || px < intervals.front().x || px >= intervals.back().x) {
            return skvx::int8(0);
        }
        auto it = std::upper_bound(
                intervals.begin(),
                intervals.end(),
                px,
                [](int val, const ScanlineOracle8x::RowWindingInterval& interval) {
                    return val < interval.x;
                });
        if (it == intervals.begin()) {
            return skvx::int8(0);
        }
        return std::prev(it)->winding;
    }

    bool validate(skiatest::Reporter* reporter,
                  uint16_t viewportWidth,
                  uint16_t viewportHeight,
                  uint32_t* maxSampleDiffOut = nullptr) const {
        if (!this->validateStreamInvariants(reporter, viewportWidth, viewportHeight)) {
            return false;
        }

        constexpr size_t kBytesPerTile = kTileWidth * kTileHeight;
        bool allPass = true;
        int stripIdx = 0;
        const int numStrips = fStrips.size();

        // Zero-copy streaming walk row-by-row directly from fStrips array
        while (stripIdx < numStrips) {
            if (fStrips[stripIdx].fX == Strip::kCapCoord) {
                stripIdx++;
                continue;
            }

            uint16_t rowY = fStrips[stripIdx].fY;
            int rowStartIdx = stripIdx;

            // Find range [rowStartIdx, rowEndIdx) for the current tile row
            int rowEndIdx = rowStartIdx;
            while (rowEndIdx < numStrips && fStrips[rowEndIdx].fY == rowY) {
                if (fStrips[rowEndIdx].fX == Strip::kCapCoord) {
                    rowEndIdx++;
                    break;
                }
                rowEndIdx++;
            }

            // Advance main stream index to next row
            stripIdx = rowEndIdx;

            // Process each pixel row within this tile row [rowY, rowY + kTileHeight)
            for (uint16_t py = rowY; py < rowY + kTileHeight && py < viewportHeight; ++py) {
                // 1. Invoke reference oracle to compute ground-truth winding intervals
                std::vector<ScanlineOracle8x::RowWindingInterval> oracleIntervals =
                        ScanlineOracle8x::BuildRowIntervals(py, fPath);

                int prevEndTileX = 0;
                uint16_t alphaEndTileX = 0;

                // 2. Stream-walk strips in current tile row
                for (int i = rowStartIdx; i < rowEndIdx; ++i) {
                    const Strip& curr = fStrips[i];
                    if (curr.fX == Strip::kCapCoord) {
                        // Cap strip ends the row: validate trailing gap to viewport edge
                        if (alphaEndTileX < viewportWidth) {
                            allPass &= this->validateGapSpan(
                                    reporter, alphaEndTileX, viewportWidth, py, oracleIntervals, curr.shouldFill());
                        }
                        break;
                    }

                    uint32_t startIdx = curr.alphaIndex();
                    uint32_t endIdx = (i + 1 < numStrips) ? fStrips[i + 1].alphaIndex() : startIdx;
                    uint16_t spannedTiles = (endIdx - startIdx) / kBytesPerTile;
                    uint16_t alphaStartTileX = curr.fX;
                    alphaEndTileX = curr.fX + spannedTiles * kTileWidth;

                    // A. Validate gap preceding this strip [prevEndTileX, alphaStartTileX)
                    if (alphaStartTileX > prevEndTileX) {
                        allPass &= this->validateGapSpan(reporter,
                                                         prevEndTileX,
                                                         alphaStartTileX,
                                                         py,
                                                         oracleIntervals,
                                                         curr.shouldFill());
                    }

                    // B. Validate boundary column left of the first tile in a run
                    if (alphaStartTileX > 0) {
                        uint16_t leftColX = alphaStartTileX - 1;
                        allPass &= this->validateBoundaryColumn(
                                reporter, leftColX, py, oracleIntervals, curr.shouldFill());
                    }

                    // C. Validate inside the alpha tiles span
                    allPass &= this->validateAlphaTileSpan(reporter,
                                                          curr,
                                                          startIdx,
                                                          spannedTiles,
                                                          py,
                                                          oracleIntervals,
                                                          maxSampleDiffOut);

                    // D. Validate boundary column right of the last tile in a run
                    uint16_t rightColX = alphaEndTileX;
                    if (rightColX < viewportWidth && i + 1 < numStrips) {
                        bool nextFill = fStrips[i + 1].shouldFill();
                        allPass &= this->validateBoundaryColumn(
                                reporter, rightColX, py, oracleIntervals, nextFill);
                    }

                    prevEndTileX = alphaEndTileX;
                }
            }
        }

        return allPass;
    }

private:
    bool validateStreamInvariants(skiatest::Reporter* reporter,
                                  uint16_t viewportWidth,
                                  uint16_t viewportHeight) const {
        if (fStrips.empty()) {
            if (!fAlphaBuf.empty()) {
                ERRORF(reporter,
                       "[%s] Empty strip buffer but non-empty alpha buffer (%d bytes)",
                       fTestName,
                       fAlphaBuf.size());
                return false;
            }
            return true;
        }

        uint16_t lastY = 0;
        bool hasLastY = false;

        for (int i = 0; i < fStrips.size(); ++i) {
            const Strip& s = fStrips[i];
            if (s.fX != Strip::kCapCoord) {
                if (s.fX % kTileWidth != 0) {
                    ERRORF(reporter,
                           "[%s] Strip[%d] fX (%u) is not aligned to kTileWidth (%u)",
                           fTestName,
                           i,
                           s.fX,
                           kTileWidth);
                    return false;
                }
                if (s.fY % kTileHeight != 0) {
                    ERRORF(reporter,
                           "[%s] Strip[%d] fY (%u) is not aligned to kTileHeight (%u)",
                           fTestName,
                           i,
                           s.fY,
                           kTileHeight);
                    return false;
                }
                if (hasLastY && s.fY < lastY) {
                    ERRORF(reporter,
                           "[%s] Strip[%d] fY (%u) < previous row fY (%u)",
                           fTestName,
                           i,
                           s.fY,
                           lastY);
                    return false;
                }
                lastY = s.fY;
                hasLastY = true;
            }

            if (s.alphaIndex() % (kTileWidth * kTileHeight) != 0) {
                ERRORF(reporter,
                       "[%s] Strip[%d] alphaIndex (%u) is not a multiple of tile size (%d)",
                       fTestName,
                       i,
                       s.alphaIndex(),
                       static_cast<int>(kTileWidth * kTileHeight));
                return false;
            }
        }
        return true;
    }

    bool validateBoundaryColumn(
            skiatest::Reporter* reporter,
            uint16_t px,
            uint16_t py,
            const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
            bool expectedFill) const {
        skvx::int8 w = GetOracleWinding(px, oracleIntervals);
        int activeCount = 0;
        for (int k = 0; k < 8; ++k) {
            if (w[k] != 0) activeCount++;
        }

        // Allow 1-sample tolerance on boundary columns for curved paths due to flattening chord error
        bool matches = expectedFill ? (activeCount >= 7) : (activeCount <= 1);
        if (!matches) {
            ERRORF(reporter,
                   "[%s] Boundary Column Winding Mismatch at (px=%u, py=%u): Expected %s, "
                   "but Oracle has winding [%d,%d,%d,%d,%d,%d,%d,%d] (%d non-zero)",
                   fTestName,
                   px,
                   py,
                   expectedFill ? "Solid Fill" : "Empty Space",
                   w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7],
                   activeCount);
            return false;
        }
        return true;
    }

    bool validateGapSpan(
            skiatest::Reporter* reporter,
            int startPx,
            int endPx,
            uint16_t py,
            const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
            bool expectedFill) const {
        bool allPass = true;
        for (int px = startPx; px < endPx; ++px) {
            skvx::int8 w = GetOracleWinding(px, oracleIntervals);
            int activeCount = 0;
            for (int k = 0; k < 8; ++k) {
                if (w[k] != 0) activeCount++;
            }

            // Allow 1-sample tolerance on gap edges for curved paths due to flattening chord error
            bool matches = expectedFill ? (activeCount >= 7) : (activeCount <= 1);
            if (!matches) {
                ERRORF(reporter,
                       "[%s] Gap Span Winding Mismatch at (px=%d, py=%u): Expected %s, "
                       "but Oracle has winding [%d,%d,%d,%d,%d,%d,%d,%d] (%d non-zero)",
                       fTestName,
                       px,
                       py,
                       expectedFill ? "Solid Fill" : "Empty Space",
                       w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7],
                       activeCount);
                allPass = false;
                break;
            }
        }
        return allPass;
    }

    bool validateAlphaTileSpan(
            skiatest::Reporter* reporter,
            const Strip& strip,
            uint32_t startAlphaIdx,
            uint16_t spannedTiles,
            uint16_t py,
            const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
            uint32_t* maxSampleDiffOut) const {
        uint16_t rowY = strip.fY;
        uint16_t localY = py - rowY;
        bool allPass = true;

        for (uint16_t t = 0; t < spannedTiles; ++t) {
            uint16_t tileStartX = strip.fX + t * kTileWidth;
            uint32_t tileStartAlphaIdx = startAlphaIdx + t * (kTileWidth * kTileHeight);

            for (uint16_t localX = 0; localX < kTileWidth; ++localX) {
                uint16_t px = tileStartX + localX;
                uint32_t alphaIdx = tileStartAlphaIdx + localY * kTileWidth + localX;

                if (alphaIdx >= static_cast<uint32_t>(fExactWindings.size())) {
                    ERRORF(reporter,
                           "[%s] Alpha index %u out of bounds (exactWindings size %d) at tile(%d,%d) pixel(%d,%d)",
                           fTestName,
                           alphaIdx,
                           fExactWindings.size(),
                           static_cast<int>(tileStartX / kTileWidth),
                           static_cast<int>(rowY / kTileHeight),
                           static_cast<int>(localX),
                           static_cast<int>(localY));
                    return false;
                }

                skvx::int8 actualWinding = fExactWindings[alphaIdx];
                skvx::int8 oracleWinding = GetOracleWinding(px, oracleIntervals);

                int sampleDiff = 0;
                for (int k = 0; k < 8; ++k) {
                    if (actualWinding[k] != oracleWinding[k]) {
                        sampleDiff++;
                    }
                }

                if (maxSampleDiffOut) {
                    *maxSampleDiffOut =
                            std::max(*maxSampleDiffOut, static_cast<uint32_t>(sampleDiff));
                }

                if (sampleDiff > 4) {
                    this->printMismatchDiagnostics(reporter,
                                                   tileStartX,
                                                   rowY,
                                                   px,
                                                   py,
                                                   actualWinding,
                                                   oracleWinding);
                    ERRORF(reporter,
                           "[%s] Direct Winding Mismatch at tile(%d,%d) pixel(%d,%d) (px=%u, py=%u): "
                           "Diff %d samples. Actual Winding [%d,%d,%d,%d,%d,%d,%d,%d], Oracle Winding [%d,%d,%d,%d,%d,%d,%d,%d]",
                           fTestName,
                           static_cast<int>(tileStartX / kTileWidth),
                           static_cast<int>(rowY / kTileHeight),
                           static_cast<int>(localX),
                           static_cast<int>(localY),
                           px,
                           py,
                           sampleDiff,
                           actualWinding[0], actualWinding[1], actualWinding[2], actualWinding[3],
                           actualWinding[4], actualWinding[5], actualWinding[6], actualWinding[7],
                           oracleWinding[0], oracleWinding[1], oracleWinding[2], oracleWinding[3],
                           oracleWinding[4], oracleWinding[5], oracleWinding[6], oracleWinding[7]);
                    allPass = false;
                }
            }
        }
        return allPass;
    }

    void printMismatchDiagnostics(skiatest::Reporter* reporter,
                                  uint16_t tileStartX,
                                  uint16_t tileStartY,
                                  uint16_t px,
                                  uint16_t py,
                                  skvx::int8 actualWinding,
                                  skvx::int8 oracleWinding) const {
        SkString out;
        out.appendf("\n=== MISMATCH DIAGNOSTIC [%s] ===\n", fTestName);
        out.appendf("Location: Tile(%d,%d), Pixel(%d,%d) [Global px=%u, py=%u]\n",
                    static_cast<int>(tileStartX / kTileWidth),
                    static_cast<int>(tileStartY / kTileHeight),
                    static_cast<int>(px - tileStartX),
                    static_cast<int>(py - tileStartY),
                    px,
                    py);
        out.appendf("Actual Winding: [%d, %d, %d, %d, %d, %d, %d, %d]\n",
                    actualWinding[0],
                    actualWinding[1],
                    actualWinding[2],
                    actualWinding[3],
                    actualWinding[4],
                    actualWinding[5],
                    actualWinding[6],
                    actualWinding[7]);
        out.appendf("Oracle Winding: [%d, %d, %d, %d, %d, %d, %d, %d]\n",
                    oracleWinding[0],
                    oracleWinding[1],
                    oracleWinding[2],
                    oracleWinding[3],
                    oracleWinding[4],
                    oracleWinding[5],
                    oracleWinding[6],
                    oracleWinding[7]);

        if (fPolyline) {
            out.append("Polyline Lines near pixel:\n");
            for (auto it = fPolyline->begin(); it != fPolyline->end(); ++it) {
                auto [l, idx] = *it;
                float minX = std::min(l.p0.fX, l.p1.fX);
                float maxX = std::max(l.p0.fX, l.p1.fX);
                float minY = std::min(l.p0.fY, l.p1.fY);
                float maxY = std::max(l.p0.fY, l.p1.fY);
                if (maxX >= px - 1 && minX <= px + 2 && maxY >= py - 1 && minY <= py + 2) {
                    out.appendf("  Line %d: (%.2f, %.2f) -> (%.2f, %.2f)\n",
                                idx,
                                l.p0.fX,
                                l.p0.fY,
                                l.p1.fX,
                                l.p1.fY);
                }
            }
        }
        INFOF(reporter, "%s", out.c_str());
    }

    const SkPath& fPath;
    const SkTDArray<Strip>& fStrips;
    const SkTDArray<uint8_t>& fAlphaBuf;
    const SkTDArray<skvx::int8>& fExactWindings;
    const Polyline* fPolyline;
    const Tiles<kTileWidth, kTileHeight>* fTiler;
    const char* fTestName;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED
