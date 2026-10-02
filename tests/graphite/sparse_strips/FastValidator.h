/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_sparse_strips_FastValidator_DEFINED
#define skgpu_graphite_sparse_strips_FastValidator_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"

#include <array>
#include <cstdint>
#include <vector>

namespace skgpu::graphite {

class Recorder;

template <uint16_t kTileWidth, uint16_t kTileHeight>
class FastValidator {
public:
    static constexpr float kTileWidthF = static_cast<float>(kTileWidth);
    static constexpr float kTileHeightF = static_cast<float>(kTileHeight);

    using StripFunc = void (*)(const Tiles<kTileWidth, kTileHeight>&,
                               WideTiles* wides,
                               EndCaps* ends,
                               AlphaAtlasManager* atlasManager,
                               SkPathFillType fillType,
                               const Polyline& polyline,
                               const SparseStripConfig::LutArray& msaaLut,
                               bool isCulled,
                               const WindingHistogram& windingHistogram,
                               const SkIRect& clipRect,
                               MsaaExactMaskObserver observer);

    static void RunScalarWinding(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                                 WideTiles* wides,
                                 EndCaps* ends,
                                 AlphaAtlasManager* atlasManager,
                                 SkPathFillType fillType,
                                 const Polyline& polyline,
                                 const SparseStripConfig::LutArray& maskLut,
                                 bool isCulled,
                                 const WindingHistogram& windingHistogram,
                                 const SkIRect& clipRect,
                                 MsaaExactMaskObserver observer) {
        MakeStrips::MsaaScalar<kTileWidth, kTileHeight>(
                tileContainer, wides, ends, atlasManager,
                fillType, polyline, maskLut, isCulled, windingHistogram, clipRect, observer);
    }

    static void RunSimdWinding(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                               WideTiles* wides,
                               EndCaps* ends,
                               AlphaAtlasManager* atlasManager,
                               SkPathFillType fillType,
                               const Polyline& polyline,
                               const SparseStripConfig::LutArray& maskLut,
                               bool isCulled,
                               const WindingHistogram& windingHistogram,
                               const SkIRect& clipRect,
                               MsaaExactMaskObserver observer) {
        MakeStrips::MsaaSimd<kTileWidth, kTileHeight>(
                tileContainer, wides, ends, atlasManager,
                fillType, polyline, maskLut, isCulled, windingHistogram, clipRect, observer);
    }

    // Validates a single path end-to-end:
    // 1. Flattens using unculled Flatten::processPathsSimdTest to get ground-truth polyline.
    // 2. Runs the production pipeline under test (culled Flatten -> Tiler -> MakeStrips).
    // 3. Evaluates 8-subsample winding only across the tiles of interest (ends.caps()).
    // 4. Verifies actual masks and alpha reduction against expected ground truth.
    static bool ValidatePath(
            skiatest::Reporter* reporter,
            Recorder* recorder,
            const SkPath& path,
            const SkMatrix& ctm,
            uint16_t viewportWidth,
            uint16_t viewportHeight,
            const char* testName,
            const SparseStripConfig::LutArray& maskLut,
            StripFunc stripFunc = nullptr,
            std::array<uint32_t, 3>* minorErrorCount = nullptr);

    // Validates precomputed strip output against an unculled reference polyline.
    static bool ValidateStripOutput(
            skiatest::Reporter* reporter,
            const SkPath& path,
            const Polyline& polyline,
            const Polyline& unculledPolyline,
            const Tiles<kTileWidth, kTileHeight>& tiler,
            const WideTiles& wides,
            const EndCaps& ends,
            const AlphaAtlasManager& atlasManager,
            const SkTDArray<uint8_t>& exactMasks,
            uint16_t viewportWidth,
            uint16_t viewportHeight,
            SkPathFillType fillType,
            const char* testName,
            std::array<uint32_t, 3>* minorErrorCount = nullptr);


private:
    struct LineHit {
        skvx::float8 x;
        skvx::int8 dir;  // +1 downward, -1 upward, 0 inactive
    };

    // Intersects lines active on row py with the 8 sub-scanlines.
    static void CollectRowHits(
            int py,
            const std::vector<Line>& candidateLines,
            std::vector<LineHit>* rowHits);

    // Buckets unculled polyline lines by tile row for fast lookup.
    static std::vector<std::vector<Line>> BuildTileRowLineBuckets(
            const Polyline& polyline,
            uint16_t viewportHeight);
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_FastValidator_DEFINED
