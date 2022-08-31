/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file
*/

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/SkGr.h"
#include "tests/Test.h"

static const int kSurfaceSize = 10;

// Exercise the public API of SkSpecialSurface (e.g., getCanvas, newImageSnapshot)
static void test_surface(const sk_sp<SkSpecialSurface>& surf,
                         skiatest::Reporter* reporter,
                         int offset) {

    const SkIRect surfSubset = surf->subset();
    REPORTER_ASSERT(reporter, offset == surfSubset.fLeft);
    REPORTER_ASSERT(reporter, offset == surfSubset.fTop);
    REPORTER_ASSERT(reporter, kSurfaceSize == surfSubset.width());
    REPORTER_ASSERT(reporter, kSurfaceSize == surfSubset.height());

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT_RELEASE(canvas);

    canvas->clear(SK_ColorRED);

    sk_sp<SkSpecialImage> img(surf->makeImageSnapshot());
    REPORTER_ASSERT(reporter, img);

    const SkIRect imgSubset = img->subset();
    REPORTER_ASSERT(reporter, surfSubset == imgSubset);

    // the canvas was invalidated by the newImageSnapshot call
    REPORTER_ASSERT(reporter, !surf->getCanvas());
}

DEF_TEST(SpecialSurface_Raster, reporter) {

    SkImageInfo info = SkImageInfo::MakeN32(kSurfaceSize, kSurfaceSize, kOpaque_SkAlphaType);
    sk_sp<SkSpecialSurface> surf(SkSpecialSurface::MakeRaster1(info, SkSurfaceProps()));

    test_surface(surf, reporter, 0);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SpecialSurface_Gpu1,
                                   reporter,
                                   ctxInfo,
                                   CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();

    for (auto colorType : { kRGBA_8888_SkColorType, kRGBA_1010102_SkColorType }) {
        if (!dContext->colorTypeSupportedAsSurface(colorType)) {
            continue;
        }

        SkImageInfo ii = SkImageInfo::Make({ kSurfaceSize, kSurfaceSize }, colorType,
                                           kPremul_SkAlphaType);

        auto surf(SkSpecialSurface::MakeRenderTarget(dContext, ii, SkSurfaceProps(),
                                                     kTopLeft_GrSurfaceOrigin));
        test_surface(surf, reporter, 0);
    }
}
