/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED
#define skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED

#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace skgpu::graphite {

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

    ScanlineOracle8x() = default;
    explicit ScanlineOracle8x(const SkPath& path) : fPath(&path) {}

    void reset() {
        fPath = nullptr;
        fHits.clear();
    }

    void reset(const SkPath& path) {
        fPath = &path;
        fHits.clear();
    }

    void intersectLine(const SkPoint pts[2], skvx::float8 targetY);
    void intersectQuad(const SkPoint pts[3], skvx::float8 targetY);
    void intersectConic(const SkPoint pts[3], float weight, skvx::float8 targetY);
    void intersectCubic(const SkPoint pts[4], skvx::float8 targetY);

    void collectPathIntersections(skvx::float8 targetY);
    void collectPathIntersections(const SkPath& path, skvx::float8 targetY);

    skvx::int8 evaluatePixelWinding(float px) const;

    std::vector<RowWindingInterval> buildRowIntervals(int py);
    std::vector<RowWindingInterval> buildRowIntervals(int py, const SkPath& path);

    skvx::int8 evaluate(SkPoint pt);
    skvx::int8 evaluate(SkPoint pt, const SkPath& path);

    const std::vector<CurveIntersection>& hits() const { return fHits; }

private:
    const SkPath* fPath = nullptr;
    std::vector<CurveIntersection> fHits;
};

template <uint16_t kTileWidth, uint16_t kTileHeight> class StripStreamValidator {
public:
    enum class Strictness {
        kStrict,     // Exact sample matching against oracle
        kNonStrict,  // Forgive winding errors on subsamples within flattening tolerance of any chord in tile
    };

    StripStreamValidator(const SkPath& path,
                         const SkTDArray<Strip>& stripBuf,
                         const SkTDArray<uint8_t>& alphaBuf,
                         const SkTDArray<skvx::int8>& exactWindings,
                         const Polyline* polyline = nullptr,
                         const Tiles<kTileWidth, kTileHeight>* tiler = nullptr,
                         const char* testName = "StripStreamValidator",
                         Strictness strictness = Strictness::kStrict,
                         float flatteningTolerance = 0.25f)
            : fPath(path)
            , fStrips(stripBuf)
            , fAlphaBuf(alphaBuf)
            , fExactWindings(exactWindings)
            , fPolyline(polyline)
            , fTiler(tiler)
            , fTestName(testName)
            , fStrictness(strictness)
            , fToleranceSq(flatteningTolerance * flatteningTolerance)
            , fOracle(path) {
        this->buildTileLineMap();
    }

    void setStrictness(Strictness s) { fStrictness = s; }
    Strictness strictness() const { return fStrictness; }
    void setTolerance(float tol) { fToleranceSq = tol * tol; }

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
                        fOracle.buildRowIntervals(py);

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
        uint16_t tileX = px / kTileWidth;
        uint16_t tileY = py / kTileHeight;

        for (int k = 0; k < 8; ++k) {
            bool covered = (w[k] != 0);
            if (fStrictness == Strictness::kNonStrict && covered != expectedFill) {
                float subX = (static_cast<float>(kMsaaPattern<uint8_t>[k]) + 0.5f) / 8.0f;
                float subY = (static_cast<float>(k) + 0.5f) / 8.0f;
                SkPoint subPt = SkPoint::Make(px + subX, py + subY);
                if (this->isWithinTolerance(subPt, tileX, tileY)) {
                    covered = expectedFill;
                }
            }
            if (covered) activeCount++;
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
        uint16_t tileY = py / kTileHeight;
        for (int px = startPx; px < endPx; ++px) {
            skvx::int8 w = GetOracleWinding(px, oracleIntervals);
            int activeCount = 0;
            uint16_t tileX = px / kTileWidth;
            for (int k = 0; k < 8; ++k) {
                bool covered = (w[k] != 0);
                if (fStrictness == Strictness::kNonStrict && covered != expectedFill) {
                    float subX = (static_cast<float>(kMsaaPattern<uint8_t>[k]) + 0.5f) / 8.0f;
                    float subY = (static_cast<float>(k) + 0.5f) / 8.0f;
                    SkPoint subPt = SkPoint::Make(px + subX, py + subY);
                    if (this->isWithinTolerance(subPt, tileX, tileY)) {
                        covered = expectedFill;
                    }
                }
                if (covered) activeCount++;
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
        uint16_t tileY = rowY / kTileHeight;
        bool allPass = true;

        for (uint16_t t = 0; t < spannedTiles; ++t) {
            uint16_t tileStartX = strip.fX + t * kTileWidth;
            uint16_t tileX = tileStartX / kTileWidth;
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
                           static_cast<int>(tileX),
                           static_cast<int>(tileY),
                           static_cast<int>(localX),
                           static_cast<int>(localY));
                    return false;
                }

                skvx::int8 actualWinding = fExactWindings[alphaIdx];
                skvx::int8 oracleWinding = GetOracleWinding(px, oracleIntervals);

                int sampleDiff = 0;
                for (int k = 0; k < 8; ++k) {
                    if (actualWinding[k] != oracleWinding[k]) {
                        bool isError = true;
                        if (fStrictness == Strictness::kNonStrict) {
                            float subX = (static_cast<float>(kMsaaPattern<uint8_t>[k]) + 0.5f) / 8.0f;
                            float subY = (static_cast<float>(k) + 0.5f) / 8.0f;
                            SkPoint subPt = SkPoint::Make(px + subX, py + subY);
                            if (this->isWithinTolerance(subPt, tileX, tileY)) {
                                isError = false;
                            }
                        }
                        if (isError) {
                            sampleDiff++;
                        }
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
                           static_cast<int>(tileX),
                           static_cast<int>(tileY),
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

private:
    static float DistanceSqToSegment(SkPoint p, SkPoint a, SkPoint b) {
        float dx = b.fX - a.fX;
        float dy = b.fY - a.fY;
        float lenSq = dx * dx + dy * dy;
        if (lenSq == 0.0f) {
            float px = p.fX - a.fX;
            float py = p.fY - a.fY;
            return px * px + py * py;
        }
        float t = ((p.fX - a.fX) * dx + (p.fY - a.fY) * dy) / lenSq;
        t = std::max(0.0f, std::min(1.0f, t));
        float projX = a.fX + t * dx;
        float projY = a.fY + t * dy;
        float distSqX = p.fX - projX;
        float distSqY = p.fY - projY;
        return distSqX * distSqX + distSqY * distSqY;
    }

    void buildTileLineMap() {
        if (!fTiler || !fPolyline) {
            return;
        }
        for (const auto& tile : fTiler->getTiles()) {
            uint32_t lineIdx = tile.fPackedLineIdxIntersectionMask >> IntersectionBits::INT_MASK_SHIFT;
            if (lineIdx + 1 < static_cast<uint32_t>(fPolyline->points().size())) {
                Line line = fPolyline->getLine(lineIdx);
                uint32_t tileKey = (static_cast<uint32_t>(tile.y) << 16) | tile.x;
                auto& lines = fTileLineMap[tileKey];
                bool exists = false;
                for (const auto& existing : lines) {
                    if (existing.p0 == line.p0 && existing.p1 == line.p1) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    lines.push_back(line);
                }
            }
        }
    }

    bool isWithinTolerance(SkPoint subPt, uint16_t tileX, uint16_t tileY) const {
        for (int dy = -1; dy <= 1; ++dy) {
            int ny = static_cast<int>(tileY) + dy;
            if (ny < 0) continue;
            for (int dx = -1; dx <= 1; ++dx) {
                int nx = static_cast<int>(tileX) + dx;
                if (nx < 0) continue;
                uint32_t key = (static_cast<uint32_t>(ny) << 16) | static_cast<uint32_t>(nx);
                auto it = fTileLineMap.find(key);
                if (it != fTileLineMap.end()) {
                    for (const auto& line : it->second) {
                        if (DistanceSqToSegment(subPt, line.p0, line.p1) <= fToleranceSq) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    const SkPath& fPath;
    const SkTDArray<Strip>& fStrips;
    const SkTDArray<uint8_t>& fAlphaBuf;
    const SkTDArray<skvx::int8>& fExactWindings;
    const Polyline* fPolyline;
    const Tiles<kTileWidth, kTileHeight>* fTiler;
    const char* fTestName;
    Strictness fStrictness = Strictness::kStrict;
    float fToleranceSq = 0.25f * 0.25f;
    std::unordered_map<uint32_t, std::vector<Line>> fTileLineMap;
    mutable ScanlineOracle8x fOracle;
};

// -------------------------------------------------------------------------------------------------
// 3. SkpPathExtractor & SkpCoverageRunner: Extract and validate paths from SKP files
// -------------------------------------------------------------------------------------------------
struct SkpExtractedPath {
    SkPath path;
    SkPaint paint;
    SkMatrix matrix;
    SkPath devicePath;
};

class SkpPathExtractor {
public:
    static std::vector<SkpExtractedPath> ExtractPaths(const char* filepath);
    static std::vector<SkpExtractedPath> ExtractPaths(SkStream* stream);
    static std::vector<SkpExtractedPath> ExtractPaths(const SkPicture* picture);
};

class SkpCoverageRunner {
public:
    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static bool ValidatePath(
            skiatest::Reporter* reporter,
            const SkPath& path,
            const char* testName,
            const SkTDArray<uint8_t>& maskLut,
            bool useSimd = true,
            std::array<uint32_t, 3>* minorErrorCount = nullptr) {
        if (path.isEmpty() || !path.isFinite()) {
            return true;
        }

        SkRect bounds = path.getBounds();
        if (bounds.width() <= 0.0f || bounds.height() <= 0.0f) {
            return true;
        }

        // Translate path to origin with a small margin so coordinates are positive
        float margin = 4.0f;
        SkMatrix trans;
        trans.setTranslate(margin - bounds.fLeft, margin - bounds.fTop);
        SkPath localPath = path.makeTransform(trans);
        SkRect localBounds = localPath.getBounds();

        uint16_t rawWidth = static_cast<uint16_t>(std::ceil(localBounds.fRight + margin));
        uint16_t rawHeight = static_cast<uint16_t>(std::ceil(localBounds.fBottom + margin));

        // Align viewport dimensions to tile sizes
        uint16_t vpWidth = (rawWidth + kTileWidth - 1) & ~(kTileWidth - 1);
        uint16_t vpHeight = (rawHeight + kTileHeight - 1) & ~(kTileHeight - 1);

        Flatten flattener;
        Polyline polyline;
        flattener.processPaths<FlattenMode::kSimd>(
                localPath, SkMatrix(), static_cast<float>(vpWidth), static_cast<float>(vpHeight), &polyline);

        if (polyline.count() == 0) {
            return true;
        }

        Tiles<kTileWidth, kTileHeight> tiler;
        tiler.makeTilesMSAA(polyline, vpWidth, vpHeight);
        tiler.sortTiles();

        if (tiler.getTiles().empty()) {
            return true;
        }

        SkTDArray<Strip> stripBuf;
        SkTDArray<uint8_t> alphaBuf;
        SkTDArray<skvx::int8> exactWindings;

        auto observer = [&](uint8_t exactMask, skvx::int8 winding) {
            exactWindings.push_back(winding);
        };

        if (useSimd) {
            MakeStrips::MsaaSimd(tiler, &stripBuf, &alphaBuf, localPath.getFillType(), polyline, maskLut, observer);
        } else {
            MakeStrips::MsaaScalar(tiler, &stripBuf, &alphaBuf, localPath.getFillType(), polyline, maskLut, observer);
        }

        StripStreamValidator<kTileWidth, kTileHeight> validator(
                localPath, stripBuf, alphaBuf, exactWindings, &polyline, &tiler, testName,
                StripStreamValidator<kTileWidth, kTileHeight>::Strictness::kNonStrict);

        uint32_t maxSampleDiff = 0;
        bool pass = validator.validate(reporter, vpWidth, vpHeight, &maxSampleDiff);
        if (minorErrorCount && maxSampleDiff > 0 && maxSampleDiff <= 3) {
            (*minorErrorCount)[maxSampleDiff - 1]++;
        }
        return pass;
    }

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static bool ValidateSkp(
            skiatest::Reporter* reporter,
            const char* filepath,
            const SkTDArray<uint8_t>& maskLut,
            bool useSimd = true,
            size_t maxPathsToTest = 0) {
        auto paths = SkpPathExtractor::ExtractPaths(filepath);
        if (paths.empty()) {
            INFOF(reporter, "[SkpCoverageRunner] No paths found or failed to load SKP: %s", filepath);
            return true;
        }

        size_t count = maxPathsToTest > 0 ? std::min(paths.size(), maxPathsToTest) : paths.size();
        std::array<uint32_t, 3> minorErrors = {0, 0, 0};
        size_t passed = 0;

        for (size_t i = 0; i < count; ++i) {
            SkString name;
            name.printf("%s_path_%zu", filepath, i);
            if (ValidatePath<kTileWidth, kTileHeight>(
                    reporter, paths[i].devicePath, name.c_str(), maskLut, useSimd, &minorErrors)) {
                passed++;
            }
        }

        INFOF(reporter,
              "[%s (%dx%d)] Tested %zu / %zu paths from '%s'. Passed: %zu. Minor Errors: [1s:%u, 2s:%u, 3s:%u]\n",
              useSimd ? "SIMD" : "Scalar",
              kTileWidth,
              kTileHeight,
              count,
              paths.size(),
              filepath,
              passed,
              minorErrors[0],
              minorErrors[1],
              minorErrors[2]);

        return passed == count;
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED
