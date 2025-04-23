/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRasterContext.h"
#include "include/core/SkRasterRecorder.h"
#include "include/core/SkSurface.h"
#include "src/core/SkRasterContextImpl.h"
#include "src/core/SkResourceCache.h"
#include "tests/Test.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#endif

#include <memory>

DEF_TEST(RasterSurface_UsesRasterContextAndRecorderToDraw_DrawsPixels, reporter) {
    SkRasterContext::Options opts;
    auto ctx = SkRasterContext::Make(opts);

    std::unique_ptr<SkRasterRecorder> recorder = ctx->makeRecorder();
    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::Raster(recorder.get(), imageInfo, imageInfo.minRowBytes(), {});

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);

    SkPixmap pmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorRED);

    REPORTER_ASSERT(reporter, surface->getCanvas()->baseRecorder() == recorder.get());
}

DEF_TEST(RasterSurface_UsesTODORecorder_DrawsPixels, reporter) {
    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface =
            SkSurfaces::Raster(SkRasterRecorder::TODO(), imageInfo, imageInfo.minRowBytes(), {});

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->clear(SK_ColorGREEN);
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);

    SkPixmap pmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorRED);

    REPORTER_ASSERT(reporter, surface->getCanvas()->baseRecorder() == SkRasterRecorder::TODO());
}

DEF_TEST(ImageMakeColorSpace_UsesRasterRecorderToMakeImage_Success, reporter) {
    auto ctx = SkRasterContext::MakeWithDefaults();

    std::unique_ptr<SkRasterRecorder> recorder = ctx->makeRecorder();

    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto img = SkImages::RasterFromBitmap(bm);
    SkASSERT(img);

    auto newImg = img->makeColorSpace(recorder.get(), SkColorSpace::MakeSRGBLinear(), {});

    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 100);
    REPORTER_ASSERT(reporter, !newImg->isTextureBacked());
}

#if defined(SK_GRAPHITE)
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(
        RasterSurface_UsesGraphiteContextAndRasterRecorderToDraw_DrawsPixels,
        reporter,
        context,
        CtsEnforcement::kNextRelease) {
    std::unique_ptr<SkRasterRecorder> recorder = context->makeRasterRecorder();

    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::Raster(recorder.get(), imageInfo, imageInfo.minRowBytes(), {});

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);

    // If this were a graphite surface, we'd have to do async read-back, but we make a raster
    // surface which is synchronous.
    SkPixmap pmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorRED);
}

#endif

#if defined(SK_GANESH)
DEF_GANESH_TEST_FOR_ALL_CONTEXTS(RasterSurface_UsesGaneshContextAndRasterRecorderToDraw_DrawsPixels,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kNextRelease) {
    std::unique_ptr<SkRasterRecorder> recorder = ctxInfo.directContext()->makeRasterRecorder();

    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::Raster(recorder.get(), imageInfo, imageInfo.minRowBytes(), {});

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);

    SkPixmap pmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorRED);
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(
        ImageMakeColorSpace_GaneshImageWithContext_Success,
        reporter,
        ctxInfo,
        CtsEnforcement::kNextRelease) {
    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto img = SkImages::RasterFromBitmap(bm);
    SkASSERT(img);
    auto grImage = SkImages::TextureFromImage(ctxInfo.directContext(), img);
    SkASSERT(grImage);

    auto newImg = grImage->makeColorSpace(ctxInfo.directContext(), SkColorSpace::MakeSRGBLinear());

    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 100);
    REPORTER_ASSERT(reporter, newImg->isTextureBacked());
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(
        ImageMakeColorSpace_GaneshImageWithRecorder_Success,
        reporter,
        ctxInfo,
        CtsEnforcement::kNextRelease) {
    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto img = SkImages::RasterFromBitmap(bm);
    SkASSERT(img);
    auto grImage = SkImages::TextureFromImage(ctxInfo.directContext(), img);
    SkASSERT(grImage);

    auto newImg = grImage->makeColorSpace(ctxInfo.directContext()->asRecorder(), SkColorSpace::MakeSRGBLinear(), {});

    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 100);
    REPORTER_ASSERT(reporter, newImg->colorSpace() == SkColorSpace::MakeSRGBLinear().get());
    REPORTER_ASSERT(reporter, newImg->isTextureBacked());
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(
        ImageMakeColorSpace_GaneshSourceGaneshDestWithRecorder_Success,
        reporter,
        ctxInfo,
        CtsEnforcement::kNextRelease) {
    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto rImg = SkImages::RasterFromBitmap(bm);
    SkASSERT(rImg);

    auto img = SkImages::TextureFromImage(ctxInfo.directContext(), rImg);
    SkASSERT(img);
    SkASSERT(img->isTextureBacked());

    auto newImg = img->makeColorSpace(ctxInfo.directContext()->asRecorder(), SkColorSpace::MakeSRGBLinear(), {});

    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 100);
    REPORTER_ASSERT(reporter, newImg->isTextureBacked());
}

#endif
