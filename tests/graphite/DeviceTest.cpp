/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkGradient.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/private/chromium/Slug.h"
#include "include/utils/SkTextUtils.h"

#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/draw/DrawListBase.h"
#include "tools/fonts/FontToolUtils.h"

#include <array>
#include <cstdlib>

namespace skgpu::graphite {

// Tests that vertex transparency will affect draw order.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(DeviceTestVertexTransparency, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    // Set up transparent vertices, in a 5px wide by 10px tall rectangle.
    static constexpr int kVertexCount = 5;
    SkPoint positions[kVertexCount];
    positions[0].set(2.5, 5);
    positions[1].set(0, 0);
    positions[2].set(5, 0);
    positions[3].set(5, 10);
    positions[4].set(0, 10);

    static constexpr int kIndexCount = 6;
    static constexpr uint16_t kIndices[kIndexCount] = {0, 1, 2, 3, 4, 1};

    SkColor colors[kVertexCount];
    for (size_t i = 0; i < kVertexCount; ++i) {
        colors[i] = 0x7F00FF00;
    }

    auto v = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode,
                                  kVertexCount,
                                  positions,
                                  nullptr,
                                  colors,
                                  kIndexCount,
                                  kIndices);

    // Draw vertices at x = 0.
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(10, 10),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkCanvas* canvas = surface->getCanvas();
    canvas->drawVertices(v, SkBlendMode::kDst, SkPaint());

    // Draw a square that will overlap both vertex draws.
    SkPaint redPaint;
    redPaint.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeXYWH(0, 0, 10, 10), redPaint);

    // Draw vertices at x = 5.
    canvas->translate(5, 0);
    canvas->drawVertices(v, SkBlendMode::kDst, SkPaint());

    // Read pixels.
    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(ii);
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    // Check that draws weren't reordered to put vertex draws together.
    // The second vertex draw should have been 50% green on top of red.
    SkColor color = pixmap.getColor(9, 5);
    SkColor expected = 0xFF807F00;
    REPORTER_ASSERT(
            reporter, color == expected, "Wrong color, expected %08x, found %08x", expected, color);
}

// Tests that large clip stacks flush appropriately when handleable and safely skip draws when a
// single draw's effective clip elements exceed DrawListBase::kMaxRenderSteps.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(DeviceTestClipRenderStepLimits,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(64, 64),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    paint.setColor(SK_ColorGREEN);

    auto makeClipTriangle = [](int index) {
        float shift = index * 0.01f;
        SkPathBuilder builder;
        builder.moveTo(shift, 0.f);
        builder.lineTo(64.f, 8.f + shift * 0.5f);
        builder.lineTo(8.f, 64.f - shift);
        builder.close();
        return builder.detach();
    };

    // 1. Handleable case: accumulate clips and draws that fit within a single pass individually
    //    but require flushing across multiple batches when total render steps approach
    //    DrawListBase::kMaxRenderSteps, while also issuing multiple draws under the same clip stack
    //    to exercise already-drawn clip tracking (`undrawnClipElements < clipElements.size()`).
    for (int batch = 0; batch < 10; ++batch) {
        canvas->save();
        for (int subBatch = 0; subBatch < 2; ++subBatch) {
            for (int i = 0; i < 150; ++i) {
                canvas->clipPath(makeClipTriangle(batch * 1000 + subBatch * 150 + i),
                                 SkClipOp::kIntersect,
                                 /*doAntiAlias=*/false);
            }
            canvas->drawRect(SkRect::MakeWH(64, 64), paint);
            canvas->drawRect(SkRect::MakeWH(64, 64), paint);
        }
        canvas->restore();
    }

    // 2. Pathological case: a single draw with more effective clip elements than can fit in one
    //    DrawList even after flushing. This should be skipped without asserting or overflowing.
    constexpr int kPathologicalClipCount =
            DrawListBase::kMaxRenderSteps / Renderer::kMaxRenderSteps + 10;
    canvas->save();
    for (int i = 0; i < kPathologicalClipCount; ++i) {
        canvas->clipPath(makeClipTriangle(20000 + i), SkClipOp::kIntersect, /*doAntiAlias=*/false);
    }
    canvas->drawRect(SkRect::MakeWH(64, 64), paint);
    canvas->restore();

    // 3. Subsequent normal draw after restoring the pathological clip stack should still succeed.
    canvas->drawRect(SkRect::MakeWH(64, 64), paint);

    std::unique_ptr<Recording> recording = recorder->snap();
    REPORTER_ASSERT(reporter, recording != nullptr);
}

namespace {

void enable_bilerp_glyph_atlas(ContextOptions* options) {
    options->fSupportBilerpFromGlyphAtlas = true;
}

// Checks that solid glyph interior pixels in maskBm sample the same shader colors in textBm as
// refBm, spanning both repeating gradient bands while excluding 1px AA/LCD fringes
void verify_perspective_shader_pixels(skiatest::Reporter* reporter,
                                      const char* label,
                                      const SkBitmap& maskBm,
                                      const SkBitmap& textBm,
                                      const SkBitmap& refBm) {
    int interiorPixels = 0;
    int bluePixels = 0;
    int redPixels = 0;
    int mismatchedPixels = 0;
    for (int y = 1; y < maskBm.height() - 1; ++y) {
        for (int x = 1; x < maskBm.width() - 1; ++x) {
            bool solid = true;
            for (int dy = -1; dy <= 1 && solid; ++dy) {
                for (int dx = -1; dx <= 1 && solid; ++dx) {
                    solid = maskBm.getColor(x + dx, y + dy) == SK_ColorWHITE;
                }
            }
            if (!solid) {
                continue;
            }
            ++interiorPixels;
            const SkColor got = textBm.getColor(x, y);
            const SkColor want = refBm.getColor(x, y);
            if (SkColorGetB(want) > 200 && SkColorGetR(want) < 50) {
                ++bluePixels;
            } else if (SkColorGetR(want) > 200 && SkColorGetB(want) < 50) {
                ++redPixels;
            }
            if (std::abs(int(SkColorGetR(got)) - int(SkColorGetR(want))) > 8 ||
                std::abs(int(SkColorGetG(got)) - int(SkColorGetG(want))) > 8 ||
                std::abs(int(SkColorGetB(got)) - int(SkColorGetB(want))) > 8) {
                ++mismatchedPixels;
            }
        }
    }
    REPORTER_ASSERT(reporter, interiorPixels > 20, "%s interior=%d", label, interiorPixels);
    REPORTER_ASSERT(reporter,
                    bluePixels > 5 && redPixels > 5,
                    "%s blue=%d red=%d",
                    label,
                    bluePixels,
                    redPixels);
    REPORTER_ASSERT(reporter,
                    mismatchedPixels == 0,
                    "%s mismatches=%d/%d",
                    label,
                    mismatchedPixels,
                    interiorPixels);
}

}  // namespace

// Verifies that SDFTextRenderStep, SDFTextLCDRenderStep, and BitmapTextRenderStep (via a 2D Slug
// replayed with atlas bilerp) sample paint shaders at perspective-correct local coordinates
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(DeviceTestPerspectiveTextShaders,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           /*testContext=*/,
                                           /*options=*/,
                                           enable_bilerp_glyph_atlas,
                                           /*cond=*/true,
                                           CtsEnforcement::kNextRelease) {
    if (context->supportsProtectedContent()) {
        return;
    }
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const SkImageInfo ii =
            SkImageInfo::Make({320, 140}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkM44 persp;
    persp.setRC(3, 2, -1.0f / 280.0f);
    const SkM44 xform = SkM44::Translate(160.0f, 70.0f) * persp *
                        SkM44::Rotate({0, 1, 0}, -SK_ScalarPI / 5) *
                        SkM44::Rotate({1, 0, 0}, SK_ScalarPI / 6);

    constexpr SkPoint kPts[2] = {{0.0f, 0.0f}, {20.0f, 16.0f}};
    constexpr SkColor4f kColors[4] = {
            SkColors::kBlue, SkColors::kBlue, SkColors::kRed, SkColors::kRed};
    constexpr float kPos[4] = {0.0f, 0.5f, 0.5f, 1.0f};
    sk_sp<SkShader> shader =
            SkShaders::LinearGradient(kPts, {{kColors, kPos, SkTileMode::kRepeat}, {}})
                    ->makeWithLocalMatrix(SkMatrix::Scale(1.25f, 0.8f));

    auto renderToBitmap = [&](const SkSurfaceProps& props, auto drawFn) {
        sk_sp<SkSurface> surf =
                SkSurfaces::RenderTarget(recorder.get(), ii, skgpu::Mipmapped::kNo, &props);
        SkASSERT(surf);
        drawFn(surf->getCanvas());
        SkBitmap bm;
        bm.allocPixels(ii);
        SkAssertResult(surf->readPixels(bm, 0, 0));
        return bm;
    };

    const SkSurfaceProps defaultProps;
    const SkBitmap refBm = renderToBitmap(defaultProps, [&](SkCanvas* c) {
        c->clear(SK_ColorGREEN);
        c->concat(xform);
        SkPaint p;
        p.setShader(shader);
        c->drawRect({-150, -55, 150, 55}, p);
    });

    sk_sp<SkTypeface> tf = ToolUtils::CreatePortableTypeface("serif", SkFontStyle::Bold());
    // Direct perspective outline draws promote to SDFT regardless of SkFont::Edging; capturing a
    // Slug at 2D identity records a DirectMaskSubRun so replaying it under xform exercises
    // BitmapTextRenderStep
    const SkFont baseFont(tf, 42.0f);
    const float halfWidth = baseFont.measureText("PERSP", 5, SkTextEncoding::kUTF8) * 0.5f;
    sk_sp<SkTextBlob> slugBlob = SkTextBlob::MakeFromString("PERSP", baseFont);
    SkASSERT(slugBlob);
    sk_sp<SkSurface> captureSurf = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkASSERT(captureSurf);
    sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(
            captureSurf->getCanvas(), *slugBlob, {-halfWidth, 14.0f}, SkPaint());
    REPORTER_ASSERT(reporter, slug != nullptr);

    struct Case {
        const char* label;
        SkFont::Edging edging;
        SkPixelGeometry geometry;
        bool useSlug;
    };
    constexpr std::array<Case, 3> kCases = {{
            {"sdf", SkFont::Edging::kAntiAlias, kUnknown_SkPixelGeometry, false},
            {"sdf_lcd", SkFont::Edging::kSubpixelAntiAlias, kRGB_H_SkPixelGeometry, false},
            {"bitmap_slug", SkFont::Edging::kAntiAlias, kUnknown_SkPixelGeometry, true},
    }};

    for (const Case& tc : kCases) {
        const SkSurfaceProps props(0, tc.geometry);
        SkFont font(tf, 42.0f);
        font.setEdging(tc.edging);
        auto drawGlyphs = [&](SkCanvas* c, const SkPaint& p) {
            if (tc.useSlug) {
                slug->draw(c, p);
            } else {
                SkTextUtils::DrawString(
                        c, "PERSP", 0.0f, 14.0f, font, p, SkTextUtils::kCenter_Align);
            }
        };

        const SkBitmap maskBm = renderToBitmap(props, [&](SkCanvas* c) {
            c->clear(SK_ColorBLACK);
            c->concat(xform);
            SkPaint white;
            white.setColor(SK_ColorWHITE);
            drawGlyphs(c, white);
        });
        const SkBitmap textBm = renderToBitmap(props, [&](SkCanvas* c) {
            c->clear(SK_ColorGREEN);
            c->concat(xform);
            SkPaint p;
            p.setShader(shader);
            drawGlyphs(c, p);
        });
        verify_perspective_shader_pixels(reporter, tc.label, maskBm, textBm, refBm);
    }

    // Edge-on 3D rotation has an invertible 4x4 Transform whose 2.5D asM33() projection is
    // singular; drawing must be culled cleanly without shading stray pixels
    const SkBitmap edgeOnBm = renderToBitmap(defaultProps, [&](SkCanvas* c) {
        c->clear(SK_ColorGREEN);
        c->translate(160.0f, 70.0f);
        c->concat(SkM44::Rotate({0, 1, 0}, SK_ScalarPI / 2));
        SkFont font(tf, 42.0f);
        SkPaint p;
        p.setShader(shader);
        SkTextUtils::DrawString(c, "PERSP", 0.0f, 14.0f, font, p, SkTextUtils::kCenter_Align);
    });
    REPORTER_ASSERT(reporter, edgeOnBm.getColor(160, 70) == SK_ColorGREEN);
}

}  // namespace skgpu::graphite
