/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "src/core/SkPathEffectBase.h"
#include "tests/Test.h"

#include <array>

#if defined(SK_GANESH)
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#endif

// crbug.com/348821 was rooted in SkDashPathEffect refusing to flatten and unflatten itself when
// the effect is nonsense.  Here we test that it fails when passed nonsense parameters.

DEF_TEST(DashPathEffectTest_crbug_348821, r) {
    SkScalar intervals[] = { 1.76934361e+36f, 2.80259693e-45f };  // Values from bug.
    const int count = 2;
    SkScalar phase = SK_ScalarInfinity;  // Used to force a nonsense effect.
    sk_sp<SkPathEffect> dash(SkDashPathEffect::Make(intervals, count, phase));

    REPORTER_ASSERT(r, dash == nullptr);
}

// Test out the asPoint culling behavior.
DEF_TEST(DashPathEffectTest_asPoints, r) {

    const SkScalar intervals[] = { 1.0f, 1.0f };
    const int count = 2;
    sk_sp<SkPathEffect> dash(SkDashPathEffect::Make(intervals, count, 0.0f));

    SkRect cull = SkRect::MakeWH(1.0f, 1.0f);

    const struct {
        SkPoint fPts[2];
        bool    fExpectedResult;
    } testCases[] = {
        { { { -5.0f,  0.5f }, { -4.0f,  0.5f } }, false },   // off to the left
        { { {  4.0f,  0.5f }, {  5.0f,  0.5f } }, false },   // off to the right
        { { {  0.5f,  4.0f }, {  0.5f,  5.0f } }, false },   // off the bottom
        { { {  0.5f, -5.0f }, {  0.5f, -4.0f } }, false },   // off the top
        { { {  0.5f,  0.2f }, {  0.5f,  0.8f } }, true  },   // entirely inside vertical
        { { {  0.2f,  0.5f }, {  0.8f,  0.5f } }, true  },   // entirely inside horizontal
        { { {  0.5f, -5.0f }, {  0.5f,  5.0f } }, true  },   // straddles both sides vertically
        { { { -5.0f,  0.5f }, {  5.0f,  0.5f } }, true  },   // straddles both sides horizontally
        { { {  0.5f, -5.0f }, {  0.5f,  0.5f } }, true  },   // straddles top
        { { {  0.5f,  5.0f }, {  0.5f,  0.5f } }, true  },   // straddles bottom
        { { { -5.0f,  0.5f }, {  0.5f,  0.5f } }, true  },   // straddles left
        { { {  5.0f,  0.5f }, {  0.5f,  0.5f } }, true  },   // straddles right
        { { {  0.5f,  0.5f }, {  0.5f,  0.5f } }, false },   // zero length
    };

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1.0f);
    SkStrokeRec rec(paint);

    static const int kNumMats = 3;
    SkMatrix mats[kNumMats];
    mats[0].reset();
    mats[1].setRotate(90, 0.5f, 0.5f);
    mats[2].setTranslate(10.0f, 10.0f);

    for (int i = 0; i < kNumMats; ++i) {
        for (int j = 0; j < (int)std::size(testCases); ++j) {
            for (int k = 0; k < 2; ++k) {  // exercise alternating endpoints
                SkPathEffectBase::PointData results;
                SkPath src;

                src.moveTo(testCases[j].fPts[k]);
                src.lineTo(testCases[j].fPts[(k+1)%2]);

                bool actualResult = as_PEB(dash)->asPoints(&results, src, rec, mats[i], &cull);
                if (i < 2) {
                    REPORTER_ASSERT(r, actualResult == testCases[j].fExpectedResult);
                } else {
                    // On the third pass all the lines should be outside the translated cull rect
                    REPORTER_ASSERT(r, !actualResult);
                }
            }
        }
    }
}

DEF_TEST(DashPath_bug4871, r) {
    SkPath path;
    path.moveTo(30, 24);
    path.cubicTo(30.002f, 24, 30, 24, 30, 24);
    path.close();

    SkScalar intervals[2] = { 1, 1 };
    sk_sp<SkPathEffect> dash(SkDashPathEffect::Make(intervals, 2, 0));

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setPathEffect(dash);

    SkPath fill;
    skpathutils::FillPathWithPaint(path, paint, &fill);
}

// Verify that long lines with many dashes don't cause overflows/OOMs.
DEF_TEST(DashPathEffectTest_asPoints_limit, r) {
    sk_sp<SkSurface> surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(256, 256)));
    SkCanvas* canvas = surface->getCanvas();

    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    // force the bounds to outset by a large amount
    p.setStrokeWidth(5.0e10f);
    const SkScalar intervals[] = { 1, 1 };
    p.setPathEffect(SkDashPathEffect::Make(intervals, std::size(intervals), 0));
    canvas->drawLine(1, 1, 1, 5.0e10f, p);
}

// This used to cause SkDashImpl to walk off the end of the intervals array, due to underflow
// trying to substract a smal value from a large one in floats.
DEF_TEST(DashCrazy_crbug_875494, r) {
    SkScalar vals[] = { 98, 94, 2888458849.f, 227, 0, 197 };
    const int N = std::size(vals);

    SkRect cull = SkRect::MakeXYWH(43,236,57,149);
    SkPath path;
    path.addRect(cull);

    SkPath path2;
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setPathEffect(SkDashPathEffect::Make(vals, N, 222));
    skpathutils::FillPathWithPaint(path, paint, &path2, &cull);
}

#if defined(SK_GANESH)

// Draws a dashed circle with circumference 100, with an on-interval of 90 and an off-interval of
// 10, offset into the intervals by 25. This should draw a dash clockwise from 3:00 ending around
// 11:00, then the start of a second dash from 12:00 back to 3:00. In https://crbug.com/1495670, the
// math for intervals ending very near 2*pi introduced floating point error that prevented drawing
// the second dash.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DashPathEffectTest_2PiInterval,
                                       reporter,
                                       contextInfo,
                                       CtsEnforcement::kNextRelease) {
    const float r = 50.0f / M_PI;
    const float centerX = ceilf(0.5f * r);
    const float centerY = ceilf(r);
    const float dashWidth = 10.0f;

    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(16, 16),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    GrDirectContext* context = contextInfo.directContext();
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, ii);
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(dashWidth);

    constexpr float intervals[2] = {90.0f, 10.0f};
    paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 25.0f));
    canvas->drawCircle(centerX, centerY, r, paint);

    // Check that we drew the second dash, which starts at the top of the circle.
    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(surface->imageInfo());
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }
    SkColor topColor = pixmap.getColor(centerX + 1.0f, centerY - r);
    REPORTER_ASSERT(reporter, topColor == SK_ColorRED);
}

#endif  // defined(SK_GANESH)
