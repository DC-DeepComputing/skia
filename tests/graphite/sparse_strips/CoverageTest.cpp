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
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "tests/Test.h"
#include "tests/graphite/sparse_strips/FastValidator.h"
#include "tests/graphite/sparse_strips/SkpValidator.h"

#include <array>
#include <vector>

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight> class CoverageTestRunner {
public:
    static constexpr float kTileWidthF = static_cast<float>(kTileWidth);
    static constexpr float kTileHeightF = static_cast<float>(kTileHeight);
    static constexpr uint32_t kViewportWidth = 400;
    static constexpr uint32_t kViewportHeight = 400;
    static constexpr float kViewportWidthF = static_cast<float>(kViewportWidth);
    static constexpr float kViewportHeightF = static_cast<float>(kViewportHeight);

    using StripFunc = typename FastValidator<kTileWidth, kTileHeight>::StripFunc;

    static void RunScalarWinding(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                                 WideTiles<kTileHeight>& wides,
                                 EndCaps<kTileHeight>& ends,
                                 SkTDArray<uint8_t>* alphaBuf,
                                 SkPathFillType fillType,
                                 const Polyline& polyline,
                                 const SkTDArray<uint8_t>& maskLut,
                                 MsaaExactMaskObserver observer,
                                 uint16_t viewportWidth,
                                 uint16_t viewportHeight) {
        FastValidator<kTileWidth, kTileHeight>::RunScalarWinding(
                tileContainer, wides, ends, alphaBuf, fillType, polyline, maskLut, observer,
                viewportWidth, viewportHeight);
    }

    static void RunSimdWinding(const Tiles<kTileWidth, kTileHeight>& tileContainer,
                               WideTiles<kTileHeight>& wides,
                               EndCaps<kTileHeight>& ends,
                               SkTDArray<uint8_t>* alphaBuf,
                               SkPathFillType fillType,
                               const Polyline& polyline,
                               const SkTDArray<uint8_t>& maskLut,
                               MsaaExactMaskObserver observer,
                               uint16_t viewportWidth,
                               uint16_t viewportHeight) {
        FastValidator<kTileWidth, kTileHeight>::RunSimdWinding(
                tileContainer, wides, ends, alphaBuf, fillType, polyline, maskLut, observer,
                viewportWidth, viewportHeight);
    }

    CoverageTestRunner(StripFunc func, const char* implName) : fFunc(func), fImplName(implName) {}

    void runAll(skiatest::Reporter* reporter) {
        const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();
        constexpr int kErrorLimit = 3;
        std::array<uint32_t, kErrorLimit> minorErrorCount = {0, 0, 0};
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

        addRect(kTileWidthF * 0.5f, kTileHeightF * 0.5f, "Rect(Small)");
        addRect(kTileWidthF, kTileHeightF, "Rect(ExactTile)");
        addRect(kTileWidthF * 2.5f, kTileHeightF * 1.5f, "Rect(MultiTile)");
        addRect(kTileWidthF * 4.0f, 0.2f, "Rect(HorizSliver)");
        addRect(0.2f, kTileHeightF * 4.0f, "Rect(VertSliver)");

        baseGeometries.push_back(
                {SkPathBuilder()
                         .addCircle(kTileWidthF * 1.5f, kTileHeightF * 1.5f, kTileWidthF * 1.2f)
                         .detach(),
                 "Circle"});

        baseGeometries.push_back(
                {SkPathBuilder().addOval(SkRect::MakeWH(kTileWidth * 4.0f, 0.5f)).detach(),
                 "ThinOval"});

        SkPathBuilder inset;
        inset.addRect(SkRect::MakeWH(kTileWidthF * 3.0f, kTileHeightF * 3.0f),
                      SkPathDirection::kCW);
        inset.addRect(SkRect::MakeXYWH(kTileWidthF, kTileHeightF, kTileWidthF, kTileHeightF),
                      SkPathDirection::kCCW);
        baseGeometries.push_back({inset.detach(), "InsetRect"});

        // Tile-relative alignments (dx, dy)
        const SkPoint alignments[] = {
                {0.0f, 0.0f},                                 // Top & Left aligned
                {0.0f, kTileHeightF * 0.5f},                  // Left aligned, offset top
                {kTileWidthF * 0.5f, 0.0f},                   // Top aligned, offset left
                {kTileWidthF * 0.33f, kTileHeightF * 0.33f},  // Strictly inside
                {kTileWidthF - 0.01f, kTileHeightF - 0.01f}   // Right on a tile boundary edge
        };

        for (const TestCase& geom : baseGeometries) {
            // Progressively rotate the geometry
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

                    SkString testName;
                    testName.printf("%s - %s Rot(%.1f) Align(%.2f,%.2f)",
                                    fImplName,
                                    geom.name,
                                    angle,
                                    alignment.fX,
                                    alignment.fY);

                    if (!FastValidator<kTileWidth, kTileHeight>::ValidatePath(
                                reporter,
                                deviceSpacePath,
                                SkMatrix::I(),
                                kViewportWidth,
                                kViewportHeight,
                                testName.c_str(),
                                lut,
                                fFunc,
                                &minorErrorCount)) {
                        return;
                    }
                    totalTestsRun++;
                }
            }
        }

        INFOF(reporter,
              "[%s (%dx%d)] Coverage LUT Test Complete. Ran %d variants. "
              "Minor Error Summary: 1-sample: %u, 2-sample: %u, 3-sample: %u\n",
              fImplName,
              kTileWidth,
              kTileHeight,
              totalTestsRun,
              minorErrorCount[0],
              minorErrorCount[1],
              minorErrorCount[2]);
    }

private:
    StripFunc fFunc;
    const char* fImplName;
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

DEF_TEST(SparseStrips_Coverage_SKP_SIMD_4x4, reporter) {
    const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();
    SkpValidator::ValidateSkp<4, 4>(reporter, "skps/desk_tiger8svg.skp", lut);
}

DEF_TEST(SparseStrips_Coverage_SKP_SIMD_8x8, reporter) {
    const SkTDArray<uint8_t> lut = GenerateMSAALUT<uint8_t>();
    SkpValidator::ValidateSkp<8, 8>(reporter, "skps/desk_tiger8svg.skp", lut);
}

}  // namespace skgpu::graphite
