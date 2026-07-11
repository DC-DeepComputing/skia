/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include "include/core/SkSurfaceProps.h"
#include "include/utils/SkCustomTypeface.h"
#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#endif

static void run_direct_mask_limit_test(skiatest::Reporter* reporter, SkCanvas* canvas) {
    canvas->clear(SK_ColorBLACK);

    SkCustomTypefaceBuilder builder;
    SkPath path = SkPath::Rect(SkRect::MakeLTRB(0.f, -1.f, 1.f, 0.f));
    builder.setGlyph(1, 1.0f, path);

    SkFont font(builder.detach(), 253);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);

    SkGlyphID glyph = 1;
    SkPoint position = {0, 0};
    canvas->drawGlyphs({&glyph, 1}, {&position, 1}, {100, 300}, font, paint);
}

static void disable_sdft_options(GrContextOptions* options) {
    options->fMinDistanceFieldFontSize = 384.f;
    options->fGlyphsAsPathsFontSize = 384.f;
    options->fSupportBilerpFromGlyphAtlas = true;
    options->fMaxTextureSizeOverride = 1024;
}

#if defined(SK_GANESH)
DEF_GANESH_TEST_FOR_CONTEXTS(DirectMaskLimitTest_Ganesh,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             disable_sdft_options,
                             CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(512, 512);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    if (!surface) return;
    auto canvas = surface->getCanvas();

    run_direct_mask_limit_test(reporter, canvas);

    dContext->flushAndSubmit();

    SkBitmap bitmap;
    bitmap.allocN32Pixels(512, 512);
    REPORTER_ASSERT(reporter, surface->readPixels(bitmap, 0, 0));

    // Verify that we actually rendered some white pixels (not all black),
    // proving the draw didn't bail out.
    bool has_white_pixel = false;
    for (int y = 0; y < 512; ++y) {
        for (int x = 0; x < 512; ++x) {
            if (bitmap.getColor(x, y) != SK_ColorBLACK) {
                has_white_pixel = true;
                break;
            }
        }
        if (has_white_pixel) break;
    }
    REPORTER_ASSERT(reporter, has_white_pixel, "Draw failed (all pixels are black)");
}
#endif // defined(SK_GANESH)
