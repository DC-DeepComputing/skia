/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED
#define skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

class SkPicture;
class SkStream;

namespace skgpu::graphite {

// Used as the source of ground truth for testing the entire SparseStrips pipeline end to end.
// Builds the subsample windings for a single pixel row by raycasting at each of the 8 subsample
// locations in the SparseStrips MSAA pattern.
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
    explicit ScanlineOracle8x(const SkPath& path) : fPath(path) {}

    void reset() {
        fPath.reset();
        fBuilt = false;
        fCurrentRow = -1;
        fHits.clear();
        fIntervals.clear();
    }

    void reset(const SkPath& path) {
        fPath = path;
        fBuilt = false;
        fCurrentRow = -1;
        fHits.clear();
        fIntervals.clear();
    }

    void buildRow(int py);
    void buildRow(int py, const SkPath& path);

    skvx::int8 evaluatePixelWinding(float px) const;
    skvx::int8 evaluate(float px) const { return this->evaluatePixelWinding(px); }

    skvx::int8 evaluate(SkPoint pt);
    skvx::int8 evaluate(SkPoint pt, const SkPath& path);

    bool isBuilt() const { return fBuilt; }
    int currentRow() const { return fCurrentRow; }
    const std::vector<CurveIntersection>& hits() const {
        SkASSERT(fBuilt);
        return fHits;
    }
    const std::vector<RowWindingInterval>& rowIntervals() const {
        SkASSERT(fBuilt);
        return fIntervals;
    }

    std::vector<RowWindingInterval> buildRowIntervals(int py) {
        this->buildRow(py);
        return fIntervals;
    }
    std::vector<RowWindingInterval> buildRowIntervals(int py, const SkPath& path) {
        this->buildRow(py, path);
        return fIntervals;
    }

private:
    void intersectLine(const SkPoint pts[2], const skvx::float8& targetY);
    void intersectQuad(const SkPoint pts[3], const skvx::float8& targetY);
    void intersectConic(const SkPoint pts[3], float weight, const skvx::float8& targetY);
    void intersectCubic(const SkPoint pts[4], const skvx::float8& targetY);
    void collectPathIntersections(const SkPath& path, const skvx::float8& targetY);

    SkPath fPath;
    int fCurrentRow = -1;
    bool fBuilt = false;
    std::vector<CurveIntersection> fHits;
    std::vector<RowWindingInterval> fIntervals;
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
                         float flatteningTolerance = 0.25f);

    void setStrictness(Strictness s) { fStrictness = s; }
    Strictness strictness() const { return fStrictness; }
    void setTolerance(float tol) { fToleranceSq = tol * tol; }

    static skvx::int8 GetOracleWinding(
            int px, const std::vector<ScanlineOracle8x::RowWindingInterval>& intervals);

    bool validate(skiatest::Reporter* reporter,
                  uint16_t viewportWidth,
                  uint16_t viewportHeight,
                  uint32_t* maxSampleDiffOut = nullptr) const;

private:
    bool validateStreamInvariants(skiatest::Reporter* reporter,
                                  uint16_t viewportWidth,
                                  uint16_t viewportHeight) const;

    bool validateBoundaryColumn(
            skiatest::Reporter* reporter,
            uint16_t px,
            uint16_t py,
            const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
            bool expectedFill) const;

    bool validateGapSpan(
            skiatest::Reporter* reporter,
            int startPx,
            int endPx,
            uint16_t py,
            const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
            bool expectedFill) const;

    bool validateAlphaTileSpan(
            skiatest::Reporter* reporter,
            const Strip& strip,
            uint32_t startAlphaIdx,
            uint16_t spannedTiles,
            uint16_t py,
            const std::vector<ScanlineOracle8x::RowWindingInterval>& oracleIntervals,
            uint32_t* maxSampleDiffOut) const;

    void printMismatchDiagnostics(skiatest::Reporter* reporter,
                                  uint16_t tileStartX,
                                  uint16_t tileStartY,
                                  uint16_t px,
                                  uint16_t py,
                                  skvx::int8 actualWinding,
                                  skvx::int8 oracleWinding) const;

    static float DistanceSqToSegment(SkPoint p, SkPoint a, SkPoint b);
    void buildTileLineMap();
    bool isWithinTolerance(SkPoint subPt, uint16_t tileX, uint16_t tileY) const;

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

    // We only explicitly instantiate 4 and 8
    static_assert(kTileWidth == kTileHeight && (kTileWidth == 4 || kTileWidth == 8));
};

class SkpValidator {
public:
    struct ExtractedPath {
        SkPath path;
        SkPaint paint;
        SkMatrix matrix;
        SkPath devicePath;
    };

    static std::vector<ExtractedPath> ExtractPaths(const char* filepath);
    static std::vector<ExtractedPath> ExtractPaths(SkStream* stream);
    static std::vector<ExtractedPath> ExtractPaths(const SkPicture* picture);

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static bool ValidatePath(
            skiatest::Reporter* reporter,
            const SkPath& path,
            const char* testName,
            const SkTDArray<uint8_t>& maskLut,
            bool useSimd = true,
            std::array<uint32_t, 3>* minorErrorCount = nullptr);

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static bool ValidateSkp(
            skiatest::Reporter* reporter,
            const char* filepath,
            const SkTDArray<uint8_t>& maskLut,
            bool useSimd = true);
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_CoverageTestUtils_DEFINED
