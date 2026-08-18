/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPicture.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/graphite/sparse_strips/CoverageTestUtils.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/skui/InputState.h"
#include "tools/skui/ModifierKey.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <map>
#include <unordered_map>
#include <vector>

namespace skiagm {

class SparseStripsOverlayGM : public GM {
public:
    enum class ErrorMetric {
        kWinding,   // Exact subsample winding vector equality (actualW == oracleW)
        kCoverage,  // In/out coverage decision equality ((actualW != 0) == (oracleW != 0))
    };

    enum class StrictnessMode {
        kStrict,     // Exact sample matching against oracle
        kNonStrict,  // Forgive winding errors on subsamples within 0.25px of any flattened chord in the tile
    };

    SparseStripsOverlayGM(const char* skpName = "skps/desk_mapsvg.skp")
            : fSkpPath(skpName) {}

protected:
    SkString getName() const override { return SkString("SparseStrips_ErrorOverlay_8x8"); }

    SkISize getISize() override { return SkISize::Make(1200, 900); }

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

    void onOnceBeforeDraw() override {
        constexpr uint16_t kTileWidth = 8;
        constexpr uint16_t kTileHeight = 8;
        constexpr size_t kBytesPerTile = kTileWidth * kTileHeight;
        constexpr float kToleranceSq = 0.25f * 0.25f;

        // 1. Locate and load the SKP file
        std::unique_ptr<SkStreamAsset> stream;
        auto fileData = SkData::MakeFromFileName(fSkpPath.c_str());
        if (!fileData) {
            SkString altPath = SkStringPrintf("../%s", fSkpPath.c_str());
            fileData = SkData::MakeFromFileName(altPath.c_str());
        }
        if (fileData) {
            stream = SkMemoryStream::Make(fileData);
        } else {
            stream = GetResourceAsStream(fSkpPath.c_str());
        }

        if (!stream) {
            fLoadError.printf("Failed to load SKP: %s", fSkpPath.c_str());
            return;
        }

        fPicture = SkPicture::MakeFromStream(stream.get());
        if (!fPicture) {
            fLoadError.printf("Failed to parse picture: %s", fSkpPath.c_str());
            return;
        }

        SkRect cullRect = fPicture->cullRect();
        fVpWidth = (static_cast<uint16_t>(std::ceil(cullRect.width())) + kTileWidth - 1) & ~(kTileWidth - 1);
        fVpHeight = (static_cast<uint16_t>(std::ceil(cullRect.height())) + kTileHeight - 1) & ~(kTileHeight - 1);
        fVpWidth = std::max<uint16_t>(fVpWidth, kTileWidth);
        fVpHeight = std::max<uint16_t>(fVpHeight, kTileHeight);

        // 2. Extract paths from the SKP with their exact transform matrices
        auto extractedPaths = skgpu::graphite::SkpPathExtractor::ExtractPaths(fPicture.get());
        const SkTDArray<uint8_t> lut = skgpu::graphite::GenerateMSAALUT<uint8_t>();

        fTotalPathsProcessed = 0;
        fTotalLineSegments = 0;
        fPixelDiagnostics.clear();
        fPathRecords.clear();
        fTileToPathIndices.clear();
        fErrorTilesStrict.clear();
        fErrorTilesNonStrict.clear();

        for (size_t itemIdx = 0; itemIdx < extractedPaths.size(); ++itemIdx) {
            const auto& item = extractedPaths[itemIdx];
            const SkPath& devPath = item.devicePath;
            if (devPath.isEmpty() || !devPath.isFinite()) {
                continue;
            }

            SkRect bounds = devPath.getBounds();
            if (bounds.width() <= 0.0f || bounds.height() <= 0.0f) {
                continue;
            }

            uint16_t pathVpWidth =
                    (static_cast<uint16_t>(std::ceil(std::max(bounds.fRight, cullRect.fRight))) + kTileWidth - 1) &
                    ~(kTileWidth - 1);
            uint16_t pathVpHeight =
                    (static_cast<uint16_t>(std::ceil(std::max(bounds.fBottom, cullRect.fBottom))) + kTileHeight - 1) &
                    ~(kTileHeight - 1);
            pathVpWidth = std::max<uint16_t>(pathVpWidth, kTileWidth);
            pathVpHeight = std::max<uint16_t>(pathVpHeight, kTileHeight);

            skgpu::graphite::Flatten flattener;
            skgpu::graphite::Polyline polyline;
            // Flatten in device space using the exact transformed path at 1x resolution
            flattener.processPaths<skgpu::graphite::FlattenMode::kSimd>(
                    devPath, SkMatrix::I(), static_cast<float>(pathVpWidth), static_cast<float>(pathVpHeight), &polyline);

            if (polyline.count() == 0) {
                continue;
            }

            size_t pathRecordIdx = fPathRecords.size();
            PathRecord rec;
            rec.pathIndex = pathRecordIdx;
            rec.originalPath = item.path;
            rec.devicePath = item.devicePath;
            rec.matrix = item.matrix;
            rec.paint = item.paint;
            rec.fillType = devPath.getFillType();
            fPathRecords.push_back(rec);

            // Build closed SkPath representing the flattened polygon geometry
            SkPathBuilder pathBuilder;
            bool inContour = false;
            for (const SkPoint& pt : polyline.points()) {
                if (std::isnan(pt.fX) || std::isnan(pt.fY)) {
                    if (inContour) {
                        pathBuilder.close();
                        inContour = false;
                    }
                } else {
                    if (!inContour) {
                        pathBuilder.moveTo(pt);
                        inContour = true;
                    } else {
                        pathBuilder.lineTo(pt);
                    }
                }
            }
            if (inContour) {
                pathBuilder.close();
            }
            SkPath flattenedPolyPath = pathBuilder.detach();
            flattenedPolyPath.setFillType(devPath.getFillType());

            fFlattenedPaths.push_back({flattenedPolyPath, item.paint});

            // Extract all individual flattened line segments for wireframe rendering
            for (auto it = polyline.begin(); it != polyline.end(); ++it) {
                auto [line, idx] = *it;
                fWireframeLines.push_back(line);
                fTotalLineSegments++;
            }

            // Tile using 8x8 tiles
            skgpu::graphite::Tiles<kTileWidth, kTileHeight> tiler;
            tiler.makeTilesMSAA(polyline, pathVpWidth, pathVpHeight);
            tiler.sortTiles();

            if (tiler.getTiles().empty()) {
                continue;
            }

            // Associate this path with all its generated edge tiles (converting tile units to pixel coords)
            std::unordered_map<uint32_t, std::vector<skgpu::graphite::Line>> pathTileLineMap;
            for (const auto& tile : tiler.getTiles()) {
                fTileToPathIndices[{static_cast<uint16_t>(tile.x * kTileWidth),
                                    static_cast<uint16_t>(tile.y * kTileHeight)}].push_back(pathRecordIdx);

                uint32_t lineIdx = tile.fPackedLineIdxIntersectionMask >> skgpu::graphite::IntersectionBits::INT_MASK_SHIFT;
                if (lineIdx + 1 < static_cast<uint32_t>(polyline.points().size())) {
                    skgpu::graphite::Line line = polyline.getLine(lineIdx);
                    uint32_t tileKey = (static_cast<uint32_t>(tile.y) << 16) | tile.x;
                    auto& lines = pathTileLineMap[tileKey];
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

            auto isNearTileLine = [&](SkPoint subPt, uint16_t tileGridX, uint16_t tileGridY) -> bool {
                for (int dy = -1; dy <= 1; ++dy) {
                    int ny = static_cast<int>(tileGridY) + dy;
                    if (ny < 0) continue;
                    for (int dx = -1; dx <= 1; ++dx) {
                        int nx = static_cast<int>(tileGridX) + dx;
                        if (nx < 0) continue;
                        uint32_t key = (static_cast<uint32_t>(ny) << 16) | static_cast<uint32_t>(nx);
                        auto it = pathTileLineMap.find(key);
                        if (it != pathTileLineMap.end()) {
                            for (const auto& line : it->second) {
                                if (DistanceSqToSegment(subPt, line.p0, line.p1) <= kToleranceSq) {
                                    return true;
                                }
                            }
                        }
                    }
                }
                return false;
            };

            SkTDArray<skgpu::graphite::Strip> stripBuf;
            SkTDArray<uint8_t> alphaBuf;
            SkTDArray<skvx::int8> exactWindings;

            auto observer = [&](uint8_t /*exactMask*/, skvx::int8 winding) {
                exactWindings.push_back(winding);
            };

            // Run 8x8 SIMD strip generation
            skgpu::graphite::MakeStrips::MsaaSimd(
                    tiler, &stripBuf, &alphaBuf, devPath.getFillType(), polyline, lut, observer);

            // Ground truth ScanlineOracle8x evaluated on the exact transformed geometry
            skgpu::graphite::ScanlineOracle8x oracle(devPath);

            fTotalPathsProcessed++;

            int stripIdx = 0;
            const int numStrips = stripBuf.size();
            const bool isEvenOdd = devPath.getFillType() == SkPathFillType::kEvenOdd ||
                                   devPath.getFillType() == SkPathFillType::kInverseEvenOdd;

            while (stripIdx < numStrips) {
                if (stripBuf[stripIdx].fX == skgpu::graphite::Strip::kCapCoord) {
                    stripIdx++;
                    continue;
                }

                uint16_t rowY = stripBuf[stripIdx].fY;
                int rowStartIdx = stripIdx;
                int rowEndIdx = rowStartIdx;
                while (rowEndIdx < numStrips && stripBuf[rowEndIdx].fY == rowY) {
                    if (stripBuf[rowEndIdx].fX == skgpu::graphite::Strip::kCapCoord) {
                        rowEndIdx++;
                        break;
                    }
                    rowEndIdx++;
                }
                stripIdx = rowEndIdx;

                for (uint16_t py = rowY; py < rowY + kTileHeight && py < pathVpHeight; ++py) {
                    auto oracleIntervals = oracle.buildRowIntervals(py);

                    for (int i = rowStartIdx; i < rowEndIdx; ++i) {
                        const auto& curr = stripBuf[i];
                        if (curr.fX == skgpu::graphite::Strip::kCapCoord) {
                            break;
                        }

                        uint32_t startIdx = curr.alphaIndex();
                        uint32_t endIdx = (i + 1 < numStrips) ? stripBuf[i + 1].alphaIndex() : startIdx;
                        uint16_t spannedTiles = (endIdx - startIdx) / kBytesPerTile;

                        for (uint16_t t = 0; t < spannedTiles; ++t) {
                            uint16_t tileStartX = curr.fX + t * kTileWidth;
                            uint32_t tileStartAlphaIdx = startIdx + t * kBytesPerTile;
                            uint16_t localY = py - rowY;
                            uint16_t tileGridX = tileStartX / kTileWidth;
                            uint16_t tileGridY = rowY / kTileHeight;

                            // Also associate path with every tile spanned by solid strips
                            fTileToPathIndices[{tileStartX, rowY}].push_back(pathRecordIdx);

                            for (uint16_t localX = 0; localX < kTileWidth; ++localX) {
                                uint16_t px = tileStartX + localX;
                                uint32_t alphaIdx = tileStartAlphaIdx + localY * kTileWidth + localX;

                                if (alphaIdx < static_cast<uint32_t>(exactWindings.size())) {
                                    skvx::int8 actualW = exactWindings[alphaIdx];
                                    skvx::int8 oracleW = skgpu::graphite::StripStreamValidator<kTileWidth, kTileHeight>::GetOracleWinding(
                                            px, oracleIntervals);

                                    PixelDiagnostic diag;
                                    diag.px = px;
                                    diag.py = py;
                                    diag.actualW = actualW;
                                    diag.oracleW = oracleW;
                                    diag.actualAlpha = (alphaIdx < static_cast<uint32_t>(alphaBuf.size())) ? alphaBuf[alphaIdx] : 0;
                                    diag.windingDiffCountStrict = 0;
                                    diag.windingDiffCountNonStrict = 0;
                                    diag.coverageDiffCountStrict = 0;
                                    diag.coverageDiffCountNonStrict = 0;
                                    diag.maxWindingDelta = 0;

                                    int oracleActiveCount = 0;
                                    for (int k = 0; k < 8; ++k) {
                                        float subX = (static_cast<float>(skgpu::graphite::kMsaaPattern<uint8_t>[k]) + 0.5f) / 8.0f;
                                        float subY = (static_cast<float>(k) + 0.5f) / 8.0f;
                                        SkPoint subPt = SkPoint::Make(px + subX, py + subY);
                                        diag.withinTolerance[k] = isNearTileLine(subPt, tileGridX, tileGridY);

                                        bool windingDiff = (actualW[k] != oracleW[k]);
                                        if (windingDiff) {
                                            diag.windingDiffCountStrict++;
                                            if (!diag.withinTolerance[k]) {
                                                diag.windingDiffCountNonStrict++;
                                            }
                                            diag.maxWindingDelta = std::max(
                                                    diag.maxWindingDelta, std::abs(static_cast<int>(actualW[k]) - static_cast<int>(oracleW[k])));
                                        }

                                        bool actualCovered = isEvenOdd ? ((actualW[k] & 1) != 0) : (actualW[k] != 0);
                                        bool oracleCovered = isEvenOdd ? ((oracleW[k] & 1) != 0) : (oracleW[k] != 0);
                                        if (oracleCovered) {
                                            oracleActiveCount++;
                                        }
                                        if (actualCovered != oracleCovered) {
                                            diag.coverageDiffCountStrict++;
                                            if (!diag.withinTolerance[k]) {
                                                diag.coverageDiffCountNonStrict++;
                                            }
                                        }
                                    }
                                    diag.oracleAlpha = static_cast<uint8_t>((oracleActiveCount * 255 + 4) / 8);

                                    if (diag.windingDiffCountStrict > 0 || diag.coverageDiffCountStrict > 0) {
                                        fPixelDiagnostics.push_back(diag);

                                        uint16_t errTileX = (px / kTileWidth) * kTileWidth;
                                        uint16_t errTileY = (py / kTileHeight) * kTileHeight;
                                        fErrorTilesStrict.push_back({errTileX, errTileY});
                                        if (diag.windingDiffCountNonStrict > 0 || diag.coverageDiffCountNonStrict > 0) {
                                            fErrorTilesNonStrict.push_back({errTileX, errTileY});
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Deduplicate path records per tile in spatial map
        for (auto& entry : fTileToPathIndices) {
            std::sort(entry.second.begin(), entry.second.end());
            entry.second.erase(std::unique(entry.second.begin(), entry.second.end()), entry.second.end());
        }

        // Deduplicate error tiles list for keyboard navigation
        std::sort(fErrorTilesStrict.begin(), fErrorTilesStrict.end());
        fErrorTilesStrict.erase(std::unique(fErrorTilesStrict.begin(), fErrorTilesStrict.end()), fErrorTilesStrict.end());

        std::sort(fErrorTilesNonStrict.begin(), fErrorTilesNonStrict.end());
        fErrorTilesNonStrict.erase(std::unique(fErrorTilesNonStrict.begin(), fErrorTilesNonStrict.end()), fErrorTilesNonStrict.end());

        // Default selection to first error tile if available
        if (!fErrorTilesStrict.empty()) {
            fSelectedTileX = fErrorTilesStrict[0].first;
            fSelectedTileY = fErrorTilesStrict[0].second;
            fHasSelectedTile = true;
        }
    }

    void dumpSelectedTilePaths() const {
        if (!fHasSelectedTile) {
            printf("\n[SparseStrips] No tile selected. Click a tile in Viewer or use '[' / ']' to select.\n\n");
            fflush(stdout);
            return;
        }

        uint16_t tx = fSelectedTileX;
        uint16_t ty = fSelectedTileY;
        auto it = fTileToPathIndices.find({tx, ty});

        printf("\n================================================================================\n");
        printf("[SparseStrips] Tile (x=%u, y=%u) [Bounds: (%u, %u) to (%u, %u)]\n",
               tx, ty, tx, ty, (unsigned)(tx + 8), (unsigned)(ty + 8));

        if (it == fTileToPathIndices.end() || it->second.empty()) {
            printf("No unflattened paths registered for this tile.\n");
            printf("================================================================================\n\n");
            fflush(stdout);
            return;
        }

        const auto& uniquePathIndices = it->second;
        printf("Found %zu intersecting unflattened path(s)\n", uniquePathIndices.size());
        printf("================================================================================\n\n");

        for (size_t pathIdx : uniquePathIndices) {
            if (pathIdx >= fPathRecords.size()) {
                continue;
            }
            const auto& rec = fPathRecords[pathIdx];
            const SkPath& path = rec.devicePath;

            const char* fillStr = (rec.fillType == SkPathFillType::kWinding) ? "kWinding" :
                                  (rec.fillType == SkPathFillType::kEvenOdd ? "kEvenOdd" :
                                  (rec.fillType == SkPathFillType::kInverseWinding ? "kInverseWinding" : "kInverseEvenOdd"));

            printf("--------------------------------------------------------------------------------\n");
            printf("--- Path #%zu | FillType: %s | Verbs: %d | Points: %d ---\n",
                   pathIdx, fillStr, path.countVerbs(), path.countPoints());
            printf("--------------------------------------------------------------------------------\n");

            // Print CTM Matrix
            const SkMatrix& m = rec.matrix;
            printf("// Transform Matrix (from SKP):\n");
            printf("SkMatrix matrix = SkMatrix::MakeAll(\n"
                   "    %+11.6ff, %+11.6ff, %+11.6ff,\n"
                   "    %+11.6ff, %+11.6ff, %+11.6ff,\n"
                   "    %+11.6ff, %+11.6ff, %+11.6ff);\n\n",
                   m.getScaleX(), m.getSkewX(), m.getTranslateX(),
                   m.getSkewY(), m.getScaleY(), m.getTranslateY(),
                   m.getPerspX(), m.getPerspY(), m[8]);

            // Print copy-pasteable C++ SkPath definition
            printf("// Copy-pasteable C++ SkPath construction (in device space):\n");
            printf("SkPath path;\n");
            printf("path.setFillType(SkPathFillType::%s);\n", fillStr);

            SkPath::Iter iter(path, true);
            SkPoint pts[4];
            SkPath::Verb verb;

            while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
                switch (verb) {
                    case SkPath::kMove_Verb:
                        printf("path.moveTo(%10.4ff, %10.4ff);\n", pts[0].fX, pts[0].fY);
                        break;
                    case SkPath::kLine_Verb:
                        printf("path.lineTo(%10.4ff, %10.4ff);\n", pts[1].fX, pts[1].fY);
                        break;
                    case SkPath::kQuad_Verb:
                        printf("path.quadTo(%10.4ff, %10.4ff, %10.4ff, %10.4ff);\n",
                               pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                        break;
                    case SkPath::kConic_Verb:
                        printf("path.conicTo(%10.4ff, %10.4ff, %10.4ff, %10.4ff, %10.4ff);\n",
                               pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY, iter.conicWeight());
                        break;
                    case SkPath::kCubic_Verb:
                        printf("path.cubicTo(%10.4ff, %10.4ff, %10.4ff, %10.4ff, %10.4ff, %10.4ff);\n",
                               pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY, pts[3].fX, pts[3].fY);
                        break;
                    case SkPath::kClose_Verb:
                        printf("path.close();\n");
                        break;
                    default:
                        break;
                }
            }
            printf("\n");
        }
        printf("================================================================================\n\n");
        fflush(stdout);
    }

    void dumpSelectedTileErrors() const {
        if (!fHasSelectedTile) {
            printf("\n[SparseStrips] No tile selected. Click a tile in Viewer or use '[' / ']' to select.\n\n");
            fflush(stdout);
            return;
        }

        uint16_t tx = fSelectedTileX;
        uint16_t ty = fSelectedTileY;

        std::vector<const PixelDiagnostic*> tileDiags;
        for (const auto& diag : fPixelDiagnostics) {
            if (diag.px >= tx && diag.px < tx + 8 && diag.py >= ty && diag.py < ty + 8) {
                tileDiags.push_back(&diag);
            }
        }

        const char* metricStr = (fErrorMetric == ErrorMetric::kWinding) ? "WINDING" : "COVERAGE";
        const char* strictStr = (fStrictness == StrictnessMode::kStrict) ? "STRICT" : "NON-STRICT (0.25px tol)";

        printf("\n================================================================================\n");
        printf("[SparseStrips] ERROR DIAGNOSTICS: Tile (x=%u, y=%u) [Bounds: (%u, %u) to (%u, %u)]\n",
               tx, ty, tx, ty, (unsigned)(tx + 8), (unsigned)(ty + 8));
        printf("Active Settings: Metric = %s | Strictness = %s\n", metricStr, strictStr);

        if (tileDiags.empty()) {
            printf("No pixel discrepancies recorded for this tile (all 64 pixels match oracle).\n");
            printf("================================================================================\n\n");
            fflush(stdout);
            return;
        }

        size_t strictWindingErrors = 0;
        size_t nonStrictWindingErrors = 0;
        size_t strictCoverageErrors = 0;
        size_t nonStrictCoverageErrors = 0;
        size_t totalActiveErrors = 0;

        for (const auto* diag : tileDiags) {
            strictWindingErrors += diag->windingDiffCountStrict;
            nonStrictWindingErrors += diag->windingDiffCountNonStrict;
            strictCoverageErrors += diag->coverageDiffCountStrict;
            nonStrictCoverageErrors += diag->coverageDiffCountNonStrict;

            int diffCount = (fStrictness == StrictnessMode::kStrict) ?
                    ((fErrorMetric == ErrorMetric::kWinding) ? diag->windingDiffCountStrict : diag->coverageDiffCountStrict) :
                    ((fErrorMetric == ErrorMetric::kWinding) ? diag->windingDiffCountNonStrict : diag->coverageDiffCountNonStrict);
            if (diffCount > 0) {
                totalActiveErrors++;
            }
        }

        printf("Tile Status: %zu / 64 pixel(s) have discrepancies (%zu active error(s) under %s / %s)\n",
               tileDiags.size(), totalActiveErrors, metricStr, strictStr);
        printf("Discrepancy Summary:\n");
        printf("  - Winding Errors:  Strict = %zu subsamples | Non-Strict = %zu subsamples\n",
               strictWindingErrors, nonStrictWindingErrors);
        printf("  - Coverage Errors: Strict = %zu subsamples | Non-Strict = %zu subsamples\n",
               strictCoverageErrors, nonStrictCoverageErrors);
        printf("================================================================================\n\n");

        std::sort(tileDiags.begin(), tileDiags.end(), [](const PixelDiagnostic* a, const PixelDiagnostic* b) {
            if (a->py != b->py) return a->py < b->py;
            return a->px < b->px;
        });

        for (size_t i = 0; i < tileDiags.size(); ++i) {
            const auto& diag = *tileDiags[i];
            uint16_t localX = diag.px - tx;
            uint16_t localY = diag.py - ty;

            int activeDiff = (fStrictness == StrictnessMode::kStrict) ?
                    ((fErrorMetric == ErrorMetric::kWinding) ? diag.windingDiffCountStrict : diag.coverageDiffCountStrict) :
                    ((fErrorMetric == ErrorMetric::kWinding) ? diag.windingDiffCountNonStrict : diag.coverageDiffCountNonStrict);

            printf("--------------------------------------------------------------------------------\n");
            printf("--- Pixel #%zu: Global (%u, %u) | Local Tile (%u, %u) | Status: %s (%d active error(s)) ---\n",
                   i + 1, diag.px, diag.py, localX, localY,
                   (activeDiff > 0 ? "ERROR" : "PASS (tolerated)"), activeDiff);
            printf("--------------------------------------------------------------------------------\n");
            printf("Alpha:  Actual = %3u (%5.1f%%)  |  Oracle = %3u (%5.1f%%)\n",
                   diag.actualAlpha, (diag.actualAlpha / 255.0f) * 100.0f,
                   diag.oracleAlpha, (diag.oracleAlpha / 255.0f) * 100.0f);
            printf("Windings:\n");
            printf("  Actual: [%+2d, %+2d, %+2d, %+2d, %+2d, %+2d, %+2d, %+2d]\n",
                   diag.actualW[0], diag.actualW[1], diag.actualW[2], diag.actualW[3],
                   diag.actualW[4], diag.actualW[5], diag.actualW[6], diag.actualW[7]);
            printf("  Oracle: [%+2d, %+2d, %+2d, %+2d, %+2d, %+2d, %+2d, %+2d]\n",
                   diag.oracleW[0], diag.oracleW[1], diag.oracleW[2], diag.oracleW[3],
                   diag.oracleW[4], diag.oracleW[5], diag.oracleW[6], diag.oracleW[7]);

            skvx::int8 deltas = diag.actualW - diag.oracleW;
            printf("  Delta:  [%+2d, %+2d, %+2d, %+2d, %+2d, %+2d, %+2d, %+2d]  (Max |Delta| = %d)\n\n",
                   deltas[0], deltas[1], deltas[2], deltas[3],
                   deltas[4], deltas[5], deltas[6], deltas[7],
                   diag.maxWindingDelta);

            printf("Subsample Details (8x MSAA):\n");
            printf("  Idx | Subpixel (X, Y)      | Act W | Orc W | Delta | Act Cov | Orc Cov | Within Tol | Subsample Status\n");
            printf("  ----+----------------------+-------+-------+-------+---------+---------+------------+-----------------\n");

            for (int k = 0; k < 8; ++k) {
                float subX = (static_cast<float>(skgpu::graphite::kMsaaPattern<uint8_t>[k]) + 0.5f) / 8.0f;
                float subY = (static_cast<float>(k) + 0.5f) / 8.0f;
                float globalSubX = diag.px + subX;
                float globalSubY = diag.py + subY;

                int delta = static_cast<int>(diag.actualW[k]) - static_cast<int>(diag.oracleW[k]);
                bool actCov = (diag.actualW[k] != 0);
                bool orcCov = (diag.oracleW[k] != 0);

                bool isExactMatch = (fErrorMetric == ErrorMetric::kWinding) ? (delta == 0) : (actCov == orcCov);
                const char* statusStr = isExactMatch ? "MATCH" :
                        (diag.withinTolerance[k] ? (fStrictness == StrictnessMode::kNonStrict ? "TOLERATED (OK)" : "ERROR (Near chord)") : "ERROR");

                printf("   %d  | (%9.4f, %9.4f) |  %+2d   |  %+2d   |  %+2d   |    %d    |    %d    |    %s     | %s\n",
                       k, globalSubX, globalSubY,
                       diag.actualW[k], diag.oracleW[k], delta,
                       actCov ? 1 : 0, orcCov ? 1 : 0,
                       diag.withinTolerance[k] ? "YES" : " no",
                       statusStr);
            }
            printf("\n");
        }

        // List nearby flattened chords intersecting or adjacent to this tile
        std::vector<skgpu::graphite::Line> nearbyLines;
        for (const auto& line : fWireframeLines) {
            float minX = std::min(line.p0.fX, line.p1.fX);
            float maxX = std::max(line.p0.fX, line.p1.fX);
            float minY = std::min(line.p0.fY, line.p1.fY);
            float maxY = std::max(line.p0.fY, line.p1.fY);
            if (maxX >= tx - 1.0f && minX <= tx + 9.0f && maxY >= ty - 1.0f && minY <= ty + 9.0f) {
                nearbyLines.push_back(line);
            }
        }

        if (!nearbyLines.empty()) {
            printf("--------------------------------------------------------------------------------\n");
            printf("Flattened Chords in/near Tile (%zu chord(s)):\n", nearbyLines.size());
            printf("--------------------------------------------------------------------------------\n");
            for (size_t i = 0; i < nearbyLines.size(); ++i) {
                const auto& line = nearbyLines[i];
                printf("  Chord #%zu: (%8.4f, %8.4f) -> (%8.4f, %8.4f) | dx=%+8.4f, dy=%+8.4f\n",
                       i + 1, line.p0.fX, line.p0.fY, line.p1.fX, line.p1.fY,
                       line.p1.fX - line.p0.fX, line.p1.fY - line.p0.fY);
            }
            printf("\n");
        }

        printf("================================================================================\n\n");
        fflush(stdout);
    }

    bool onMouse(SkScalar x, SkScalar y, skui::InputState state, skui::ModifierKey /*modifiers*/) override {
        if (state == skui::InputState::kDown) {
            fMouseDownPt = SkPoint::Make(x, y);
        } else if (state == skui::InputState::kUp) {
            float dx = x - fMouseDownPt.fX;
            float dy = y - fMouseDownPt.fY;
            // If it's a quick click without dragging, update selected tile
            if (std::sqrt(dx * dx + dy * dy) < 4.0f) {
                if (x >= 0.0f && y >= 0.0f && x < fVpWidth && y < fVpHeight) {
                    fSelectedTileX = (static_cast<uint16_t>(std::floor(x)) / 8) * 8;
                    fSelectedTileY = (static_cast<uint16_t>(std::floor(y)) / 8) * 8;
                    fHasSelectedTile = true;
                }
            }
        }
        return false;
    }

    bool onChar(SkUnichar uni) override {
        const auto& activeErrorTiles = (fStrictness == StrictnessMode::kStrict) ? fErrorTilesStrict : fErrorTilesNonStrict;
        switch (uni) {
            case 'p':
            case 'P':
            case '\r':
            case '\n':
                this->dumpSelectedTilePaths();
                return true;
            case 'e':
            case 'E':
                this->dumpSelectedTileErrors();
                return true;
            case 's':
            case 'S':
                fStrictness = (fStrictness == StrictnessMode::kStrict) ? StrictnessMode::kNonStrict : StrictnessMode::kStrict;
                return true;
            case ']':
            case '}':
                if (!activeErrorTiles.empty()) {
                    fErrorTileIndex = (fErrorTileIndex + 1) % activeErrorTiles.size();
                    fSelectedTileX = activeErrorTiles[fErrorTileIndex].first;
                    fSelectedTileY = activeErrorTiles[fErrorTileIndex].second;
                    fHasSelectedTile = true;
                    return true;
                }
                break;
            case '[':
            case '{':
                if (!activeErrorTiles.empty()) {
                    fErrorTileIndex = (fErrorTileIndex == 0) ? activeErrorTiles.size() - 1 : fErrorTileIndex - 1;
                    fSelectedTileX = activeErrorTiles[fErrorTileIndex].first;
                    fSelectedTileY = activeErrorTiles[fErrorTileIndex].second;
                    fHasSelectedTile = true;
                    return true;
                }
                break;
            case 'm':
            case 'M':
                fErrorMetric = (fErrorMetric == ErrorMetric::kWinding) ? ErrorMetric::kCoverage : ErrorMetric::kWinding;
                return true;
            case 'n':
            case 'N':
                fShowSubsampleDeltas = !fShowSubsampleDeltas;
                return true;
            case 'w':
            case 'W':
                fShowWireframe = !fShowWireframe;
                return true;
            case 'f':
            case 'F':
                fShowFlattenedFill = !fShowFlattenedFill;
                return true;
            case 'b':
            case 'B':
                fBackgroundMode = (fBackgroundMode + 1) % 3; // 0: Off, 1: Faint, 2: Full
                return true;
            case 'g':
            case 'G':
                fShowPixelGrid = !fShowPixelGrid;
                return true;
            case 't':
            case 'T':
                fShowTileGrid = !fShowTileGrid;
                return true;
            case 'h':
            case 'H':
                fShowHUD = !fShowHUD;
                return true;
            default:
                break;
        }
        return false;
    }

    void onDraw(SkCanvas* canvas) override {
        if (!fLoadError.isEmpty()) {
            SkPaint textPaint;
            textPaint.setColor(SK_ColorRED);
            SkFont font(ToolUtils::DefaultTypeface(), 20.0f);
            canvas->drawString(fLoadError, 30, 50, font, textPaint);
            return;
        }

        // Layer 0: Original SKP Background
        if (fPicture && fBackgroundMode > 0) {
            canvas->save();
            if (fBackgroundMode == 1) {
                SkPaint faintPaint;
                faintPaint.setAlphaf(0.30f);
                canvas->saveLayer(nullptr, &faintPaint);
                canvas->drawPicture(fPicture);
                canvas->restore();
            } else {
                canvas->drawPicture(fPicture);
            }
            canvas->restore();
        }

        // Layer 1: Flattened Path Fill
        if (fShowFlattenedFill) {
            for (const auto& item : fFlattenedPaths) {
                SkPaint fillPaint = item.paint;
                fillPaint.setStyle(SkPaint::kFill_Style);
                fillPaint.setAlphaf(fBackgroundMode > 0 ? 0.35f : 0.85f);
                canvas->drawPath(item.path, fillPaint);
            }
        }

        // Layer 2: Flattened Wireframe Lines
        if (fShowWireframe) {
            SkPaint wirePaint;
            wirePaint.setAntiAlias(true);
            wirePaint.setStyle(SkPaint::kStroke_Style);
            wirePaint.setStrokeWidth(0.0f); // Hairline
            wirePaint.setColor(SkColorSetARGB(230, 0, 229, 255)); // Electric Cyan

            for (const auto& line : fWireframeLines) {
                canvas->drawLine(line.p0.fX, line.p0.fY, line.p1.fX, line.p1.fY, wirePaint);
            }
        }

        // Layer 3: Hairline 1x1 Pixel Grid
        if (fShowPixelGrid) {
            SkPaint pixelGridPaint;
            pixelGridPaint.setAntiAlias(false);
            pixelGridPaint.setStyle(SkPaint::kStroke_Style);
            pixelGridPaint.setStrokeWidth(0.0f); // Hairline
            pixelGridPaint.setColor(SkColorSetARGB(38, 200, 220, 240));

            for (int x = 0; x <= fVpWidth; ++x) {
                canvas->drawLine(static_cast<float>(x), 0.0f, static_cast<float>(x), static_cast<float>(fVpHeight), pixelGridPaint);
            }
            for (int y = 0; y <= fVpHeight; ++y) {
                canvas->drawLine(0.0f, static_cast<float>(y), static_cast<float>(fVpWidth), static_cast<float>(y), pixelGridPaint);
            }
        }

        // Layer 4: Hairline 8x8 Tile Boundaries
        if (fShowTileGrid) {
            constexpr uint16_t kTileWidth = 8;
            constexpr uint16_t kTileHeight = 8;
            SkPaint tileGridPaint;
            tileGridPaint.setAntiAlias(false);
            tileGridPaint.setStyle(SkPaint::kStroke_Style);
            tileGridPaint.setStrokeWidth(0.0f); // Hairline
            tileGridPaint.setColor(SkColorSetARGB(95, 255, 170, 0)); // Warm Amber

            for (int x = 0; x <= fVpWidth; x += kTileWidth) {
                canvas->drawLine(static_cast<float>(x), 0.0f, static_cast<float>(x), static_cast<float>(fVpHeight), tileGridPaint);
            }
            for (int y = 0; y <= fVpHeight; y += kTileHeight) {
                canvas->drawLine(0.0f, static_cast<float>(y), static_cast<float>(fVpWidth), static_cast<float>(y), tileGridPaint);
            }
        }

        // Layer 5: Selected 8x8 Tile Highlight Box
        if (fHasSelectedTile) {
            SkRect selRect = SkRect::MakeXYWH(fSelectedTileX, fSelectedTileY, 8.0f, 8.0f);

            SkPaint selFill;
            selFill.setColor(SkColorSetARGB(55, 255, 235, 0)); // Soft gold translucent fill
            canvas->drawRect(selRect, selFill);

            SkPaint selBox;
            selBox.setAntiAlias(false);
            selBox.setStyle(SkPaint::kStroke_Style);
            selBox.setStrokeWidth(0.0f); // Hairline
            selBox.setColor(SkColorSetARGB(255, 255, 235, 0)); // Bright Neon Gold
            canvas->drawRect(selRect, selBox);
        }

        // Evaluate counts based on active error metric & strictness mode
        size_t erroneousPixels = 0;
        size_t totalSubsampleErrors = 0;
        size_t totalSubsampleMatches = 0;
        size_t toleranceForgivenSubsamples = 0;

        for (const auto& diag : fPixelDiagnostics) {
            int diffCount = (fStrictness == StrictnessMode::kStrict) ?
                    ((fErrorMetric == ErrorMetric::kWinding) ? diag.windingDiffCountStrict : diag.coverageDiffCountStrict) :
                    ((fErrorMetric == ErrorMetric::kWinding) ? diag.windingDiffCountNonStrict : diag.coverageDiffCountNonStrict);

            if (diffCount > 0) {
                erroneousPixels++;
            }

            for (int k = 0; k < 8; ++k) {
                bool exactMatch = (fErrorMetric == ErrorMetric::kWinding) ?
                        (diag.actualW[k] == diag.oracleW[k]) :
                        ((diag.actualW[k] != 0) == (diag.oracleW[k] != 0));

                if (exactMatch) {
                    totalSubsampleMatches++;
                } else if (fStrictness == StrictnessMode::kNonStrict && diag.withinTolerance[k]) {
                    totalSubsampleMatches++;
                    toleranceForgivenSubsamples++;
                } else {
                    totalSubsampleErrors++;
                }
            }
        }

        // Layer 6: Error 1x1 Pixel Outlines (only drawn for pixels with active errors)
        SkPaint errBoxPaint;
        errBoxPaint.setAntiAlias(false);
        errBoxPaint.setStyle(SkPaint::kStroke_Style);
        errBoxPaint.setStrokeWidth(0.0f); // Hairline
        errBoxPaint.setColor(SkColorSetARGB(180, 255, 60, 60));

        for (const auto& diag : fPixelDiagnostics) {
            int diffCount = (fStrictness == StrictnessMode::kStrict) ?
                    ((fErrorMetric == ErrorMetric::kWinding) ? diag.windingDiffCountStrict : diag.coverageDiffCountStrict) :
                    ((fErrorMetric == ErrorMetric::kWinding) ? diag.windingDiffCountNonStrict : diag.coverageDiffCountNonStrict);

            if (diffCount > 0) {
                canvas->drawRect(SkRect::MakeXYWH(diag.px, diag.py, 1.0f, 1.0f), errBoxPaint);
            }
        }

        // Layer 7: 8x MSAA Subsample Points (only drawn for pixels with active errors)
        constexpr float kSubsampleRadius = 0.038f;

        SkPaint greenPaint;
        greenPaint.setAntiAlias(true);
        greenPaint.setStyle(SkPaint::kFill_Style);
        greenPaint.setColor(SkColorSetARGB(250, 0, 230, 118)); // Correct / Forgiven (Green)

        SkPaint redPaint;
        redPaint.setAntiAlias(true);
        redPaint.setStyle(SkPaint::kFill_Style);
        redPaint.setColor(SkColorSetARGB(255, 255, 23, 68));   // True Error (Red)

        for (const auto& diag : fPixelDiagnostics) {
            int diffCount = (fStrictness == StrictnessMode::kStrict) ?
                    ((fErrorMetric == ErrorMetric::kWinding) ? diag.windingDiffCountStrict : diag.coverageDiffCountStrict) :
                    ((fErrorMetric == ErrorMetric::kWinding) ? diag.windingDiffCountNonStrict : diag.coverageDiffCountNonStrict);

            if (diffCount > 0) {
                for (int k = 0; k < 8; ++k) {
                    float subX = (static_cast<float>(skgpu::graphite::kMsaaPattern<uint8_t>[k]) + 0.5f) / 8.0f;
                    float subY = (static_cast<float>(k) + 0.5f) / 8.0f;
                    SkPoint pt = SkPoint::Make(diag.px + subX, diag.py + subY);

                    bool exactMatch = (fErrorMetric == ErrorMetric::kWinding) ?
                            (diag.actualW[k] == diag.oracleW[k]) :
                            ((diag.actualW[k] != 0) == (diag.oracleW[k] != 0));

                    bool match = exactMatch;
                    if (!match && fStrictness == StrictnessMode::kNonStrict && diag.withinTolerance[k]) {
                        match = true;
                    }

                    if (match) {
                        canvas->drawCircle(pt.fX, pt.fY, kSubsampleRadius, greenPaint);
                    } else {
                        canvas->drawCircle(pt.fX, pt.fY, kSubsampleRadius, redPaint);
                    }
                }
            }
        }

        // Layer 8: Subsample Signed Deltas on Zoom
        if (fShowSubsampleDeltas) {
            SkFont subNumFont(ToolUtils::DefaultTypeface(), 0.16f);

            SkPaint textNumPaint;
            textNumPaint.setAntiAlias(true);

            SkPaint shadowNumPaint;
            shadowNumPaint.setAntiAlias(true);
            shadowNumPaint.setColor(SkColorSetARGB(220, 0, 0, 0));

            for (const auto& diag : fPixelDiagnostics) {
                int diffCount = (fStrictness == StrictnessMode::kStrict) ?
                        ((fErrorMetric == ErrorMetric::kWinding) ? diag.windingDiffCountStrict : diag.coverageDiffCountStrict) :
                        ((fErrorMetric == ErrorMetric::kWinding) ? diag.windingDiffCountNonStrict : diag.coverageDiffCountNonStrict);

                if (diffCount > 0) {
                    for (int k = 0; k < 8; ++k) {
                        bool isErr = false;
                        if (fErrorMetric == ErrorMetric::kWinding) {
                            isErr = (diag.actualW[k] != diag.oracleW[k]);
                            if (isErr && fStrictness == StrictnessMode::kNonStrict && diag.withinTolerance[k]) {
                                isErr = false;
                            }
                            if (isErr) {
                                int delta = static_cast<int>(diag.actualW[k]) - static_cast<int>(diag.oracleW[k]);
                                float subX = (static_cast<float>(skgpu::graphite::kMsaaPattern<uint8_t>[k]) + 0.5f) / 8.0f;
                                float subY = (static_cast<float>(k) + 0.5f) / 8.0f;

                                SkString deltaStr;
                                deltaStr.printf("%+d", delta);
                                textNumPaint.setColor(delta > 0 ? SkColorSetRGB(255, 80, 80) : SkColorSetRGB(80, 180, 255));

                                canvas->drawString(deltaStr, diag.px + subX + 0.035f, diag.py + subY + 0.055f, subNumFont, shadowNumPaint);
                                canvas->drawString(deltaStr, diag.px + subX + 0.030f, diag.py + subY + 0.050f, subNumFont, textNumPaint);
                            }
                        } else {
                            bool actualCovered = (diag.actualW[k] != 0);
                            bool oracleCovered = (diag.oracleW[k] != 0);
                            isErr = (actualCovered != oracleCovered);
                            if (isErr && fStrictness == StrictnessMode::kNonStrict && diag.withinTolerance[k]) {
                                isErr = false;
                            }
                            if (isErr) {
                                float subX = (static_cast<float>(skgpu::graphite::kMsaaPattern<uint8_t>[k]) + 0.5f) / 8.0f;
                                float subY = (static_cast<float>(k) + 0.5f) / 8.0f;

                                const char* covStr = actualCovered ? "+C" : "-C";
                                textNumPaint.setColor(actualCovered ? SkColorSetRGB(255, 100, 100) : SkColorSetRGB(100, 200, 255));

                                canvas->drawString(covStr, diag.px + subX + 0.035f, diag.py + subY + 0.055f, subNumFont, shadowNumPaint);
                                canvas->drawString(covStr, diag.px + subX + 0.030f, diag.py + subY + 0.050f, subNumFont, textNumPaint);
                            }
                        }
                    }
                }
            }
        }

        // Layer 9: High-Visibility Screen-Space HUD & Controls Banner
        if (fShowHUD) {
            canvas->save();
            canvas->resetMatrix(); // Lock HUD coordinates to screen viewport

            SkRect hudRect = SkRect::MakeXYWH(20, 20, 680, 220);
            SkPaint hudBg;
            hudBg.setColor(SkColorSetARGB(238, 12, 16, 22));
            canvas->drawRoundRect(hudRect, 10, 10, hudBg);

            SkPaint hudBorder;
            hudBorder.setStyle(SkPaint::kStroke_Style);
            hudBorder.setColor(SkColorSetARGB(190, 80, 140, 220));
            hudBorder.setStrokeWidth(1.5f);
            canvas->drawRoundRect(hudRect, 10, 10, hudBorder);

            SkFont titleFont(ToolUtils::DefaultTypeface(), 17.0f);
            SkFont bodyFont(ToolUtils::DefaultTypeface(), 13.5f);
            SkPaint textPaint;
            textPaint.setColor(SK_ColorWHITE);

            canvas->drawString("SparseStrips 8x8 Diagnostic & Oracle Overlay", 38, 46, titleFont, textPaint);

            SkString pathsText;
            pathsText.printf("Source: %s | Evaluated Paths: %zu | Chords: %zu",
                             fSkpPath.c_str(), fTotalPathsProcessed, fTotalLineSegments);
            textPaint.setColor(SkColorSetRGB(180, 205, 230));
            canvas->drawString(pathsText, 38, 70, bodyFont, textPaint);

            const char* metricStr = (fErrorMetric == ErrorMetric::kWinding) ? "WINDING" : "COVERAGE";
            const char* strictStr = (fStrictness == StrictnessMode::kStrict) ? "STRICT" : "NON-STRICT (0.25px)";

            SkString statsText;
            if (fStrictness == StrictnessMode::kStrict) {
                statsText.printf("Metric: %s  |  Mode: %s  |  %zu Matches  |  %zu Errors in %zu Pixels",
                                 metricStr, strictStr, totalSubsampleMatches, totalSubsampleErrors, erroneousPixels);
            } else {
                statsText.printf("Metric: %s  |  Mode: %s  |  %zu Matches (%zu within tol)  |  %zu True Errors",
                                 metricStr, strictStr, totalSubsampleMatches, toleranceForgivenSubsamples, totalSubsampleErrors);
            }
            textPaint.setColor(totalSubsampleErrors == 0 ? SkColorSetRGB(100, 230, 120) : SkColorSetRGB(255, 120, 90));
            canvas->drawString(statsText, 38, 94, bodyFont, textPaint);

            // Selected tile inspection status
            SkString selText;
            if (fHasSelectedTile) {
                auto it = fTileToPathIndices.find({fSelectedTileX, fSelectedTileY});
                size_t numTilePaths = (it != fTileToPathIndices.end()) ? it->second.size() : 0;
                selText.printf("Selected Tile: (%u, %u)  |  %zu Parent Path(s)  |  [P]: Dump Paths  |  [E]: Dump Errors",
                               fSelectedTileX, fSelectedTileY, numTilePaths);
                textPaint.setColor(SkColorSetRGB(255, 235, 50)); // Bright Gold
            } else {
                selText.printf("Selected Tile: None (Click any tile or use '[' / ']' to select)");
                textPaint.setColor(SkColorSetRGB(180, 180, 180));
            }
            canvas->drawString(selText, 38, 120, bodyFont, textPaint);

            const char* bgModeStr = (fBackgroundMode == 0) ? "OFF" : (fBackgroundMode == 1 ? "FAINT" : "FULL");
            SkString keysText1;
            keysText1.printf("[S]trictness: %s   [M]etric: %s   [N]umbers: %s   [B]ackground: %s",
                             (fStrictness == StrictnessMode::kStrict) ? "STRICT" : "NON-STRICT",
                             (fErrorMetric == ErrorMetric::kWinding) ? "WINDING" : "COVERAGE",
                             fShowSubsampleDeltas ? "ON" : "OFF",
                             bgModeStr);
            textPaint.setColor(SkColorSetRGB(190, 210, 230));
            canvas->drawString(keysText1, 38, 146, bodyFont, textPaint);

            SkString keysText2;
            keysText2.printf("[W]ireframe: %s   [F]ill: %s   [G]rid: %s   [T]iles: %s",
                             fShowWireframe ? "ON" : "OFF",
                             fShowFlattenedFill ? "ON" : "OFF",
                             fShowPixelGrid ? "ON" : "OFF",
                             fShowTileGrid ? "ON" : "OFF");
            textPaint.setColor(SkColorSetRGB(0, 229, 255)); // Cyan
            canvas->drawString(keysText2, 38, 172, bodyFont, textPaint);

            SkString keysText3;
            keysText3.printf("Controls: Click: Select | [ / ]: Step | [P]: Paths | [E]: Errors | [S]: Strict");
            textPaint.setColor(SkColorSetRGB(255, 170, 80)); // Warm Orange
            canvas->drawString(keysText3, 38, 196, bodyFont, textPaint);

            canvas->restore();
        }
    }

private:
    struct FlattenedPathItem {
        SkPath path;
        SkPaint paint;
    };

    struct PathRecord {
        size_t pathIndex;
        SkPath originalPath;
        SkPath devicePath;
        SkMatrix matrix;
        SkPaint paint;
        SkPathFillType fillType;
    };

    struct PixelDiagnostic {
        uint16_t px;
        uint16_t py;
        skvx::int8 actualW;
        skvx::int8 oracleW;
        uint8_t actualAlpha;
        uint8_t oracleAlpha;
        int windingDiffCountStrict;
        int windingDiffCountNonStrict;
        int coverageDiffCountStrict;
        int coverageDiffCountNonStrict;
        int maxWindingDelta;
        std::array<bool, 8> withinTolerance;
    };

    SkString fSkpPath;
    SkString fLoadError;
    sk_sp<SkPicture> fPicture;
    uint16_t fVpWidth = 0;
    uint16_t fVpHeight = 0;

    std::vector<FlattenedPathItem> fFlattenedPaths;
    std::vector<skgpu::graphite::Line> fWireframeLines;
    std::vector<PixelDiagnostic> fPixelDiagnostics;
    std::vector<PathRecord> fPathRecords;
    std::map<std::pair<uint16_t, uint16_t>, std::vector<size_t>> fTileToPathIndices;
    std::vector<std::pair<uint16_t, uint16_t>> fErrorTilesStrict;
    std::vector<std::pair<uint16_t, uint16_t>> fErrorTilesNonStrict;

    size_t fTotalPathsProcessed = 0;
    size_t fTotalLineSegments = 0;

    // Tile selection state
    uint16_t fSelectedTileX = 0;
    uint16_t fSelectedTileY = 0;
    bool fHasSelectedTile = false;
    size_t fErrorTileIndex = 0;
    SkPoint fMouseDownPt = {0.0f, 0.0f};

    // Interactive toggle states
    StrictnessMode fStrictness = StrictnessMode::kNonStrict; // 's': Strict vs Non-Strict (0.25px tol)
    ErrorMetric fErrorMetric = ErrorMetric::kWinding;         // 'm': Winding vs Coverage
    bool fShowSubsampleDeltas = false;                        // 'n': Toggle subsample delta numbers adjacent to dots
    bool fShowWireframe = true;
    bool fShowFlattenedFill = true;
    int  fBackgroundMode = 1;     // 0: Off, 1: Faint (30%), 2: Full (100%)
    bool fShowPixelGrid = true;
    bool fShowTileGrid = true;
    bool fShowHUD = true;
};

DEF_GM(return new SparseStripsOverlayGM("skps/desk_mapsvg.skp");)

}  // namespace skiagm
