/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkTDArray.h"
#include "include/private/chromium/Slug.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/gpu/MaskFormat.h"
#include "src/text/gpu/SlugImpl.h"
#include "src/utils/SkFloatUtils.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <cstdint>
#include <cstring>

struct GrContextOptions;

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_empty,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface(SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info));
    auto canvas = surface->getCanvas();

    static const char* kText = " ";
    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface);
    size_t txtLen = strlen(kText);
    int glyphCount = font.countText(kText, txtLen, SkTextEncoding::kUTF8);

    SkTDArray<SkGlyphID> glyphs;
    glyphs.append(glyphCount);
    font.textToGlyphs(kText, txtLen, SkTextEncoding::kUTF8, glyphs);

    SkTextBlobBuilder builder;

    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setTypeface(typeface);
    font.setSize(16);

    const SkTextBlobBuilder::RunBuffer& buf = builder.allocRun(font, glyphs.size(), 0, 0);
    memcpy(buf.glyphs, glyphs.begin(), glyphs.size() * sizeof(SkGlyphID));
    auto blob = builder.make();

    SkPaint p;
    p.setAntiAlias(true);
    sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(canvas, *blob, {10, 10}, p);
    REPORTER_ASSERT(reporter, slug == nullptr);
}

static void set_sdf_options(GrContextOptions* options) {
    options->fMinDistanceFieldFontSize = 4;
    options->fGlyphsAsPathsFontSize = 256;
    options->fSupportBilerpFromGlyphAtlas = true;
}

DEF_GANESH_TEST_FOR_CONTEXTS(Slug_b520113415,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             set_sdf_options,
                             CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();

    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);
    auto canvas = surface->getCanvas();

    // This matrix is big enough such that with the font below...
    SkMatrix canvasMatrix = SkMatrix::Scale(1.2345f, 6.7890f);

    canvas->save();
    canvas->setMatrix(canvasMatrix);

    //  ... it will force SDFT (6.789 x 24 = ~162.9).
    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface);
    font.setSubpixel(true);
    font.setSize(24);

    static const char* kText = "A";
    int glyphCount = font.countText(kText, 1, SkTextEncoding::kUTF8);
    SkTDArray<SkGlyphID> glyphs;
    glyphs.append(glyphCount);
    font.textToGlyphs(kText, 1, SkTextEncoding::kUTF8, glyphs);

    SkTextBlobBuilder builder;
    const SkTextBlobBuilder::RunBuffer& buf = builder.allocRun(font, glyphs.size(), 0, 0);
    memcpy(buf.glyphs, glyphs.begin(), glyphs.size() * sizeof(SkGlyphID));
    auto blob = builder.make();

    SkPaint paint;
    paint.setAntiAlias(true);

    sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(canvas, *blob, {0, 0}, paint);
    canvas->restore();

    if (!slug) {
        return;
    }

    sk_sp<SkData> data = slug->serialize();
    if (!data || data->size() == 0) {
        return;
    }

    // Copy data to a writable buffer so we can corrupt it
    size_t size = data->size();
    std::unique_ptr<uint8_t[]> writableData(new uint8_t[size]);
    memcpy(writableData.get(), data->data(), size);


    // The creationMatrix of VertexFiller inside SDFTSubRun is located 216 bytes in.
    // We can overwrite it to have a matrix with perspective.
    if (size < 216 + 9 * sizeof(float)) {
        ERRORF(reporter, "Serialized Slug is too small to contain creationMatrix!");
        return;
    }
    float* f = reinterpret_cast<float*>(writableData.get() + 216);
    f[0] = 1.0f; f[1] = 0.0f; f[2] = 0.0f;
    f[3] = 0.0f; f[4] = 1.0f; f[5] = 0.0f;
    f[6] = 0.0078125f; f[7] = 0.0f; f[8] = 1.0f;

    // Deserialize the forged slug. This should return nullptr. Previously, when we went to draw it,
    // the perspective would cause issues.
    sk_sp<sktext::gpu::Slug> forgedSlug =
            sktext::gpu::Slug::Deserialize(writableData.get(), size, nullptr);
    REPORTER_ASSERT(reporter, forgedSlug == nullptr);
}

DEF_GANESH_TEST_FOR_CONTEXTS(Slug_b520571816,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             set_sdf_options,
                             CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();

    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);
    auto canvas = surface->getCanvas();

    // Scale enough to force SDFT.
    SkMatrix canvasMatrix = SkMatrix::Scale(10.0f, 10.0f);
    canvas->save();
    canvas->setMatrix(canvasMatrix);

    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface);
    font.setSize(24);

    static const char* kText = "A";
    SkTextBlobBuilder builder;
    const SkTextBlobBuilder::RunBuffer& runBuf = builder.allocRun(font, 1, 0, 0);
    font.textToGlyphs(kText, 1, SkTextEncoding::kUTF8, SkSpan(runBuf.glyphs, 1));
    auto blob = builder.make();

    SkPaint paint;
    paint.setAntiAlias(true);

    sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(canvas, *blob, {0, 0}, paint);
    SkASSERT_RELEASE(slug);
    canvas->restore();

    sk_sp<SkData> data = slug->serialize();
    SkASSERT_RELEASE(data && data->size());

    size_t size = data->size();
    std::unique_ptr<uint8_t[]> writableData(new uint8_t[size]);
    memcpy(writableData.get(), data->data(), size);

    // Corrupt the maskFormat of VertexFiller inside SDFTSubRun at offset 208 from
    // kA8 (0) to kARGB (2).
    if (size < 212) {
        ERRORF(reporter, "Serialized Slug is too small to contain maskFormat!");
        return;
    }
    int32_t* formatPtr = reinterpret_cast<int32_t*>(writableData.get() + 208);
    if (*formatPtr != 0) {
        ERRORF(reporter, "maskFormat is not kA8 at expected offset 208!");
        return;
    }
    *formatPtr = static_cast<int32_t>(skgpu::MaskFormat::kARGB);

    // Since our validation fix will be active, this must fail and return nullptr. Previously, when
    // we went to draw it, the unexpected mask format would cause issues.
    sk_sp<sktext::gpu::Slug> forgedSlug =
            sktext::gpu::Slug::Deserialize(writableData.get(), size, nullptr);
    REPORTER_ASSERT(reporter, forgedSlug == nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_draw_then_serialize,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);
    auto canvas = surface->getCanvas();

    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface);
    font.setSize(16);

    static const char* kText = "Hello World";
    size_t textLen = strlen(kText);
    SkTextBlobBuilder builder;
    const SkTextBlobBuilder::RunBuffer& runBuf = builder.allocRun(font, textLen, 0, 0);
    font.textToGlyphs(kText, textLen, SkTextEncoding::kUTF8, SkSpan(runBuf.glyphs, textLen));
    auto blob = builder.make();

    SkPaint paint;
    paint.setAntiAlias(true);

    sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(canvas, *blob, {10, 10}, paint);
    REPORTER_ASSERT(reporter, slug != nullptr);

    // Draw the slug. In the past, drawing the slug nulled out the strike in fStrikePromise, making
    // the slug unusable.
    slug->draw(canvas, paint);

    // Redraw the slug to ensure drawing again works.
    slug->draw(canvas, paint);

    // Serialize the slug post-draw. In the past, this would crash because descriptor()
    // dereferenced the nulled-out strike pointer.
    sk_sp<SkData> data = slug->serialize();
    REPORTER_ASSERT(reporter, data != nullptr && data->size() > 0);

    // Ensure it can be deserialized and drawn again.
    sk_sp<sktext::gpu::Slug> deserializedSlug =
            sktext::gpu::Slug::Deserialize(data->data(), data->size(), nullptr);
    REPORTER_ASSERT(reporter, deserializedSlug != nullptr);
    deserializedSlug->draw(canvas, paint);
}

namespace {
bool has_non_white_pixels(const SkBitmap& bm) {
    const int width = bm.width();
    const int height = bm.height();
    for (int y = 0; y < height && y < 1024; ++y) {
        for (int x = 0; x < width && x < 1024; ++x) {
            if (bm.getColor(x, y) != SK_ColorWHITE) {
                return true;
            }
        }
    }
    return false;
}

class RectGlyphDrawable final : public SkDrawable {
public:
    explicit RectGlyphDrawable(const SkRect& rect) : fRect(rect) {}

protected:
    SkRect onGetBounds() override { return fRect; }
    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        canvas->drawRect(fRect, paint);
    }

private:
    const SkRect fRect;
};
}  // namespace

// In Ganesh, GlyphVector::initBackendData only runs during AtlasTextOp::onPrepareDraws on flush,
// so flushing before serialize() is required to exercise post-draw serialization of AtlasSubRuns
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_draw_flush_then_serialize_DirectMask,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 64);
    auto surface1 = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    auto surface2 = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface1 && surface2);

    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface, 16);
    auto blob = SkTextBlob::MakeFromString("Hello World", font);
    REPORTER_ASSERT(reporter, blob != nullptr);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);

    surface1->getCanvas()->clear(SK_ColorWHITE);
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface1->getCanvas(), *blob, {10, 32}, paint);
    REPORTER_ASSERT(reporter, slug != nullptr);

    sk_sp<SkData> preDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, preDrawData != nullptr && preDrawData->size() > 0);

    slug->draw(surface1->getCanvas(), paint);
    dContext->flushAndSubmit();

    SkBitmap bm1;
    bm1.allocPixels(info);
    REPORTER_ASSERT(reporter, surface1->readPixels(bm1, 0, 0));
    REPORTER_ASSERT(reporter, has_non_white_pixels(bm1));

    sk_sp<SkData> postDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, postDrawData != nullptr && postDrawData->size() > 0);
    REPORTER_ASSERT(reporter, preDrawData->equals(postDrawData.get()));

    sk_sp<sktext::gpu::Slug> deserializedSlug =
            sktext::gpu::Slug::Deserialize(postDrawData->data(), postDrawData->size(), nullptr);
    REPORTER_ASSERT(reporter, deserializedSlug != nullptr);

    surface2->getCanvas()->clear(SK_ColorWHITE);
    deserializedSlug->draw(surface2->getCanvas(), paint);

    SkBitmap bm2;
    bm2.allocPixels(info);
    REPORTER_ASSERT(reporter, surface2->readPixels(bm2, 0, 0));
    REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(bm1, bm2));
}

// PathOpSubmitter stores glyph IDs and SkPaths in a union (IDOrPath). Converting glyph IDs to
// SkPaths in-place during submitDraws must not corrupt subsequent Slug serialization
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_draw_then_serialize_PathSubRun,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(512, 420);
    auto surface1 = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    auto surface2 = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface1 && surface2);

    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    // Must exceed fGlyphsAsPathsFontSize (324 on Linux, 384 on Android) to route to PathSubRun
    SkFont font(typeface, 400);
    auto blob = SkTextBlob::MakeFromString("AB", font);
    REPORTER_ASSERT(reporter, blob != nullptr);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);

    surface1->getCanvas()->clear(SK_ColorWHITE);
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface1->getCanvas(), *blob, {10, 360}, paint);
    REPORTER_ASSERT(reporter, slug != nullptr);

    sk_sp<SkData> preDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, preDrawData != nullptr && preDrawData->size() > 0);

    slug->draw(surface1->getCanvas(), paint);
    dContext->flushAndSubmit();

    SkBitmap bm1;
    bm1.allocPixels(info);
    REPORTER_ASSERT(reporter, surface1->readPixels(bm1, 0, 0));
    REPORTER_ASSERT(reporter, has_non_white_pixels(bm1));

    sk_sp<SkData> postDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, postDrawData != nullptr && postDrawData->size() > 0);
    REPORTER_ASSERT(reporter, preDrawData->equals(postDrawData.get()));

    sk_sp<sktext::gpu::Slug> deserializedSlug =
            sktext::gpu::Slug::Deserialize(postDrawData->data(), postDrawData->size(), nullptr);
    REPORTER_ASSERT(reporter, deserializedSlug != nullptr);

    surface2->getCanvas()->clear(SK_ColorWHITE);
    deserializedSlug->draw(surface2->getCanvas(), paint);

    SkBitmap bm2;
    bm2.allocPixels(info);
    REPORTER_ASSERT(reporter, surface2->readPixels(bm2, 0, 0));
    REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(bm1, bm2));
}

// DrawableOpSubmitter stores glyph IDs and SkDrawable* in a union (IDOrDrawable). Converting
// glyph IDs to drawables during submitDraws must not corrupt subsequent Slug serialization
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_draw_then_serialize_DrawableSubRun,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(512, 420);
    auto surface1 = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    auto surface2 = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface1 && surface2);

    SkCustomTypefaceBuilder tfBuilder;
    SkFontMetrics metrics{};
    metrics.fAscent = -1.0f;
    metrics.fDescent = 0.2f;
    tfBuilder.setMetrics(metrics);
    const SkRect glyphBounds = SkRect::MakeLTRB(0.1f, -0.8f, 0.9f, 0.0f);
    tfBuilder.setGlyph(1, 1.0f, sk_make_sp<RectGlyphDrawable>(glyphBounds), glyphBounds);
    sk_sp<SkTypeface> drawableTypeface = tfBuilder.detach();
    REPORTER_ASSERT(reporter, drawableTypeface != nullptr);

    // Must exceed fGlyphsAsPathsFontSize (324 on Linux, 384 on Android) to route to DrawableSubRun
    SkFont font(drawableTypeface, 400);
    SkTextBlobBuilder blobBuilder;
    const SkTextBlobBuilder::RunBuffer& runBuf = blobBuilder.allocRun(font, 1, 0, 0);
    runBuf.glyphs[0] = 1;
    auto blob = blobBuilder.make();
    REPORTER_ASSERT(reporter, blob != nullptr);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);

    surface1->getCanvas()->clear(SK_ColorWHITE);
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface1->getCanvas(), *blob, {10, 360}, paint);
    REPORTER_ASSERT(reporter, slug != nullptr);

    sk_sp<SkData> preDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, preDrawData != nullptr && preDrawData->size() > 0);

    slug->draw(surface1->getCanvas(), paint);
    dContext->flushAndSubmit();

    SkBitmap bm1;
    bm1.allocPixels(info);
    REPORTER_ASSERT(reporter, surface1->readPixels(bm1, 0, 0));
    REPORTER_ASSERT(reporter, has_non_white_pixels(bm1));

    sk_sp<SkData> postDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, postDrawData != nullptr && postDrawData->size() > 0);
    REPORTER_ASSERT(reporter, preDrawData->equals(postDrawData.get()));

    // Drawing a deserialized DrawableSubRun when postDrawData has corrupted glyph IDs triggers an
    // out-of-bounds assertion in SkUserScalerContext::generateDrawable
    if (preDrawData->equals(postDrawData.get())) {
        sk_sp<sktext::gpu::Slug> deserializedSlug =
                sktext::gpu::Slug::Deserialize(postDrawData->data(), postDrawData->size(), nullptr);
        REPORTER_ASSERT(reporter, deserializedSlug != nullptr);

        surface2->getCanvas()->clear(SK_ColorWHITE);
        deserializedSlug->draw(surface2->getCanvas(), paint);

        SkBitmap bm2;
        bm2.allocPixels(info);
        REPORTER_ASSERT(reporter, surface2->readPixels(bm2, 0, 0));
        REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(bm1, bm2));
    }
}

// Cached sktext::gpu::TextBlob instances in TextBlobRedrawCoordinator are never serialized and
// must drop their SkStrike references after the first flushed draw so SkStrikeCache can evict them
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_TextBlob_draw_releases_strike_from_cache,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(512, 420);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);
    auto canvas = surface->getCanvas();

    SkCustomTypefaceBuilder tfBuilder;
    SkFontMetrics metrics{};
    metrics.fAscent = -1.0f;
    metrics.fDescent = 0.2f;
    tfBuilder.setMetrics(metrics);
    tfBuilder.setGlyph(1, 1.0f, SkPath::Rect(SkRect::MakeLTRB(0.1f, -0.8f, 0.9f, 0.0f)));
    sk_sp<SkTypeface> customTypeface = tfBuilder.detach();
    REPORTER_ASSERT(reporter, customTypeface != nullptr);

    // 400pt (> fGlyphsAsPathsFontSize) exercises PathSubRun; 16pt exercises DirectMaskSubRun
    SkFont pathFont(customTypeface, 400);
    SkTextBlobBuilder pathBlobBuilder;
    pathBlobBuilder.allocRun(pathFont, 1, 0, 0).glyphs[0] = 1;
    auto pathBlob = pathBlobBuilder.make();

    SkFont maskFont(customTypeface, 16);
    SkTextBlobBuilder maskBlobBuilder;
    maskBlobBuilder.allocRun(maskFont, 1, 0, 0).glyphs[0] = 1;
    auto maskBlob = maskBlobBuilder.make();

    SkPaint paint;
    paint.setAntiAlias(true);

    canvas->drawTextBlob(pathBlob, 10, 360, paint);
    canvas->drawTextBlob(maskBlob, 10, 32, paint);
    dContext->flushAndSubmit();

    auto [pathSpec, _] = SkStrikeSpec::MakePath(
            pathFont, paint, surface->props(), SkScalerContextFlags::kFakeGammaAndBoostContrast);
    sk_sp<SkStrike> pathStrike =
            SkStrikeCache::GlobalStrikeCache()->findStrike(pathSpec.descriptor());
    REPORTER_ASSERT(reporter, pathStrike != nullptr);

    SkStrikeSpec maskSpec = SkStrikeSpec::MakeMask(
            maskFont,
            paint,
            surface->props(),
            SkScalerContextFlags::kFakeGammaAndBoostContrast,
            SkMatrix::I());
    sk_sp<SkStrike> maskStrike =
            SkStrikeCache::GlobalStrikeCache()->findStrike(maskSpec.descriptor());
    REPORTER_ASSERT(reporter, maskStrike != nullptr);

    SkStrikeCache::GlobalStrikeCache()->purgeAll();

    REPORTER_ASSERT(reporter, pathStrike->unique());
    REPORTER_ASSERT(reporter, maskStrike->unique());
}
