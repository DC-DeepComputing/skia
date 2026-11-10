/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/tessellate/MidpointContourParser.h"
#include "tests/Test.h"

#include <memory>

using namespace skgpu::graphite;

// Verifies MidpointContourParser does not round 3*X*(1/3.f) up by 1 ULP outside the contour's
// bounds at large coordinates, nor overflow float32 when summing large coordinates.
DEF_TEST(Tessellation_MidpointContourParserLargeCoords, reporter) {
    constexpr float kHugeX = 4294978560.0f;  // 2^32 + 22 * 512, ULP = 512
    {
        SkPathBuilder pb;
        pb.moveTo(kHugeX, 0.0f);
        pb.lineTo(kHugeX, 256.0f);
        pb.conicTo({kHugeX, 384.0f}, {kHugeX, 512.0f}, 0.7406733f);
        pb.close();
        SkPath path = pb.detach();

        skgpu::tess::MidpointContourParser parser(path);
        REPORTER_ASSERT(reporter, parser.parseNextContour());
        SkPoint mid = parser.currentMidpoint();
        REPORTER_ASSERT(reporter,
                        mid.fX == kHugeX,
                        "Expected midpoint.fX == %.1f, got %.1f",
                        kHugeX,
                        mid.fX);
    }

    {
        constexpr float kNearMax = 1e38f;
        SkPathBuilder pb;
        pb.moveTo(kNearMax, kNearMax);
        pb.lineTo(kNearMax, kNearMax);
        pb.lineTo(kNearMax, kNearMax);
        pb.lineTo(kNearMax, kNearMax);
        pb.close();
        SkPath path = pb.detach();

        skgpu::tess::MidpointContourParser parser(path);
        REPORTER_ASSERT(reporter, parser.parseNextContour());
        SkPoint mid = parser.currentMidpoint();
        REPORTER_ASSERT(reporter, SkIsFinite(mid.fX) && SkIsFinite(mid.fY));
    }
}

// Verifies filled (convex wedge, 3-verb fan, and CPU-chopped) and stroked (regular and hairline)
// conics at large local coordinates translated back on-screen do not overhang their device-space
// path bounds due to fp32 cancellation.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(Tessellation_LargeCoordConicBoundsTest,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    if (context->supportsProtectedContent()) {
        return;
    }

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);

    constexpr int kW = 1600;
    constexpr int kH = 1600;
    const SkImageInfo ii = SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    REPORTER_ASSERT(reporter, surface);

    SkCanvas* canvas = surface->getCanvas();
    SkBitmap bm;
    bm.allocPixels(ii);

    constexpr float kHugeX = 4294978560.0f;  // 2^32 + 22 * 512, ULP = 512

    auto countNonBlueRightOf = [&](int minX, int maxH) {
        int count = 0;
        for (int y = 0; y < maxH; ++y) {
            for (int x = minX; x < kW; ++x) {
                if (bm.getColor(x, y) != SK_ColorBLUE) {
                    ++count;
                }
            }
        }
        return count;
    };

    // 1. Unchopped 4-verb convex conic fill (lands at device x == 1024).
    {
        skiatest::ReporterContext subtest(reporter, "ConvexConicFill");
        canvas->clear(SK_ColorBLUE);

        SkPathBuilder pb;
        pb.moveTo(kHugeX - 512.0f, 0.0f);
        pb.lineTo(kHugeX, 0.0f);
        pb.conicTo({kHugeX, 256.0f}, {kHugeX, 512.0f}, 0.7406733f);
        pb.lineTo(kHugeX - 512.0f, 512.0f);
        pb.close();

        canvas->save();
        canvas->translate(-kHugeX + 1024.0f, 0.0f);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorRED);
        canvas->drawPath(pb.detach(), paint);
        canvas->restore();

        std::unique_ptr<Recording> rec = recorder->snap();
        REPORTER_ASSERT(reporter, rec && context->insertRecording({rec.get()}));
        REPORTER_ASSERT(reporter, surface->readPixels(bm, 0, 0));

        // Interior at x = 800 must be painted red; nothing right of AA fringe (x >= 1026) may be.
        REPORTER_ASSERT(reporter, bm.getColor(800, 256) == SK_ColorRED);
        int overhang = countNonBlueRightOf(1026, 512);
        REPORTER_ASSERT(reporter, overhang == 0, "Overhanging fill pixels: %d", overhang);
    }

    // 2. 3-verb arcTo-style convex conic fill (exercises MidpointContourParser weight == 3).
    {
        skiatest::ReporterContext subtest(reporter, "ThreeVerbConicFan");
        canvas->clear(SK_ColorBLUE);

        SkPathBuilder pb;
        pb.moveTo(kHugeX, 0.0f);
        pb.conicTo({kHugeX, 256.0f}, {kHugeX, 512.0f}, 0.7406733f);
        pb.lineTo(kHugeX - 512.0f, 256.0f);
        pb.close();

        canvas->save();
        canvas->translate(-kHugeX + 1024.0f, 0.0f);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorRED);
        canvas->drawPath(pb.detach(), paint);
        canvas->restore();

        std::unique_ptr<Recording> rec = recorder->snap();
        REPORTER_ASSERT(reporter, rec && context->insertRecording({rec.get()}));
        REPORTER_ASSERT(reporter, surface->readPixels(bm, 0, 0));

        REPORTER_ASSERT(reporter, bm.getColor(900, 256) == SK_ColorRED);
        int overhang = countNonBlueRightOf(1026, 512);
        REPORTER_ASSERT(reporter, overhang == 0, "Overhanging 3-verb fan pixels: %d", overhang);
    }

    // 3. Unchopped curved circular-arc conic fill (with 0.25x x-scale so a 1-ULP = 512 local
    // control point delta projects to 128px on-screen and stays a single GPU patch). Its local
    // apex offset (212.13) is < 0.5 * ULP(kHugeX) (256), verifying fast-math compilers do not
    // reassociate curveOffset back into baseCoord before applying the translation.
    {
        skiatest::ReporterContext subtest(reporter, "CurvedConicFill");
        canvas->clear(SK_ColorBLUE);

        SkPathBuilder pb;
        pb.moveTo(kHugeX - 512.0f, 0.0f);
        pb.conicTo({kHugeX, 256.0f}, {kHugeX - 512.0f, 512.0f}, SK_ScalarRoot2Over2);
        pb.close();

        canvas->save();
        canvas->translate(-0.25f * kHugeX + 1024.0f, 0.0f);
        canvas->scale(0.25f, 1.0f);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorRED);
        canvas->drawPath(pb.detach(), paint);
        canvas->restore();

        std::unique_ptr<Recording> rec = recorder->snap();
        REPORTER_ASSERT(reporter, rec && context->insertRecording({rec.get()}));
        REPORTER_ASSERT(reporter, surface->readPixels(bm, 0, 0));

        // Chord sits at x = 896 and apex at y = 256 sits at device x ~= 896 + 53.03 = 949.03.
        REPORTER_ASSERT(reporter, bm.getColor(940, 256) == SK_ColorRED);
        int overhang = countNonBlueRightOf(952, 512);
        REPORTER_ASSERT(reporter, overhang == 0, "Overhanging curved conic pixels: %d", overhang);
    }

    // 4. Non-convex conic fill with >= 50 verbs and area > 256x256 to exercise
    // TessellateCurvesRenderStep (stencilTessellatedCurvesAndTris).
    {
        skiatest::ReporterContext subtest(reporter, "NonConvexCurvesConicFill");
        canvas->clear(SK_ColorBLUE);

        SkPathBuilder pb;
        pb.moveTo(kHugeX - 512.0f, 0.0f);
        pb.lineTo(kHugeX, 0.0f);
        pb.conicTo({kHugeX, 300.0f}, {kHugeX, 600.0f}, 0.7406733f);
        // Zig-zag back up the left side so the path is non-convex and has >= 50 verbs.
        for (int i = 50; i >= 1; --i) {
            float xOff = (i % 2 == 0) ? -512.0f : -1024.0f;
            pb.lineTo(kHugeX + xOff, i * 12.0f);
        }
        pb.close();

        canvas->save();
        canvas->translate(-kHugeX + 1024.0f, 0.0f);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorRED);
        canvas->drawPath(pb.detach(), paint);
        canvas->restore();

        std::unique_ptr<Recording> rec = recorder->snap();
        REPORTER_ASSERT(reporter, rec && context->insertRecording({rec.get()}));
        REPORTER_ASSERT(reporter, surface->readPixels(bm, 0, 0));

        REPORTER_ASSERT(reporter, bm.getColor(800, 300) == SK_ColorRED);
        int overhang = countNonBlueRightOf(1026, 612);
        REPORTER_ASSERT(reporter,
                        overhang == 0,
                        "Overhanging non-convex curves conic pixels: %d",
                        overhang);
    }

    // 5. Tall conic fill requiring CPU chopping in PatchWriter::chopAndWriteConics.
    {
        skiatest::ReporterContext subtest(reporter, "ChoppedTallConicFill");
        canvas->clear(SK_ColorBLUE);

        SkPathBuilder pb;
        pb.moveTo(kHugeX - 512.0f, 0.0f);
        pb.lineTo(kHugeX, 0.0f);
        pb.conicTo({kHugeX, 800.0f}, {kHugeX, 1600.0f}, 0.25f);
        pb.lineTo(kHugeX - 512.0f, 1600.0f);
        pb.close();

        canvas->save();
        canvas->translate(-kHugeX + 1024.0f, 0.0f);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorRED);
        canvas->drawPath(pb.detach(), paint);
        canvas->restore();

        std::unique_ptr<Recording> rec = recorder->snap();
        REPORTER_ASSERT(reporter, rec && context->insertRecording({rec.get()}));
        REPORTER_ASSERT(reporter, surface->readPixels(bm, 0, 0));

        REPORTER_ASSERT(reporter, bm.getColor(800, 800) == SK_ColorRED);
        int overhang = countNonBlueRightOf(1026, 1600);
        REPORTER_ASSERT(reporter, overhang == 0, "Overhanging chopped conic pixels: %d", overhang);
    }

    // 6. Stroked conic (regular width = 10 -> [1019, 1029], and hairline width = 0).
    {
        skiatest::ReporterContext subtest(reporter, "StrokedConic");
        canvas->clear(SK_ColorBLUE);

        SkPathBuilder pb;
        pb.moveTo(kHugeX - 512.0f, 50.0f);
        pb.lineTo(kHugeX, 50.0f);
        pb.conicTo({kHugeX, 256.0f}, {kHugeX, 462.0f}, 0.7406733f);
        SkPath conicStrokePath = pb.detach();

        canvas->save();
        canvas->translate(-kHugeX + 1024.0f, 0.0f);
        SkPaint strokePaint;
        strokePaint.setAntiAlias(true);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        strokePaint.setStrokeWidth(10.0f);
        strokePaint.setStrokeCap(SkPaint::kSquare_Cap);
        strokePaint.setColor(SK_ColorRED);
        canvas->drawPath(conicStrokePath, strokePaint);
        canvas->restore();

        std::unique_ptr<Recording> rec = recorder->snap();
        REPORTER_ASSERT(reporter, rec && context->insertRecording({rec.get()}));
        REPORTER_ASSERT(reporter, surface->readPixels(bm, 0, 0));

        // Verify both halves of the 10px stroke [1019, 1029] and the square endcap [462, 467].
        REPORTER_ASSERT(reporter, bm.getColor(1020, 256) == SK_ColorRED);
        REPORTER_ASSERT(reporter, bm.getColor(1028, 256) == SK_ColorRED);
        REPORTER_ASSERT(reporter, bm.getColor(1024, 465) == SK_ColorRED);
        int overhang = countNonBlueRightOf(1031, 512);
        REPORTER_ASSERT(reporter, overhang == 0, "Overhanging stroked conic pixels: %d", overhang);

        // Hairline stroke at huge X with scale = 2 (verifies hairline neither overhangs nor
        // vanishes when 2*kHugeX has ULP = 1024 > 0.5px hairline radius).
        canvas->clear(SK_ColorBLUE);
        canvas->save();
        canvas->translate(-2.0f * kHugeX + 1024.0f, 0.0f);
        canvas->scale(2.0f, 1.0f);
        strokePaint.setStrokeWidth(0.0f);
        canvas->drawPath(conicStrokePath, strokePaint);
        canvas->restore();

        rec = recorder->snap();
        REPORTER_ASSERT(reporter, rec && context->insertRecording({rec.get()}));
        REPORTER_ASSERT(reporter, surface->readPixels(bm, 0, 0));

        bool drewHairline = (bm.getColor(1023, 256) != SK_ColorBLUE) ||
                            (bm.getColor(1024, 256) != SK_ColorBLUE);
        REPORTER_ASSERT(reporter, drewHairline, "Hairline conic vanished at large coordinates");
        overhang = countNonBlueRightOf(1026, 512);
        REPORTER_ASSERT(reporter, overhang == 0, "Overhanging hairline conic pixels: %d", overhang);
    }
}
