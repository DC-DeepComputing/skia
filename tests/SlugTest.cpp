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
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkTDArray.h"
#include "include/private/chromium/Slug.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/gpu/MaskFormat.h"
#include "src/text/gpu/SlugImpl.h"
#include "src/text/gpu/VertexFiller.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#endif

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

    // First draw and flush initializes backend data.
    slug->draw(canvas, paint);
    dContext->flushAndSubmit();

    // Verify the slug can be redrawn safely. Note: This does not test whether the Slug
    // works when the atlas is full and evicted glyphs need to be re-fetched from the strike.
    // That will be covered in a separate test.
    slug->draw(canvas, paint);
    dContext->flushAndSubmit();

    // Verify the slug can be serialized post-draw without losing strike descriptor info.
    sk_sp<SkData> data = slug->serialize();
    REPORTER_ASSERT(reporter, data != nullptr && data->size() > 0);

    // Verify the serialized slug can be deserialized and drawn.
    sk_sp<sktext::gpu::Slug> deserializedSlug =
            sktext::gpu::Slug::Deserialize(data->data(), data->size(), nullptr);
    REPORTER_ASSERT(reporter, deserializedSlug != nullptr);
    deserializedSlug->draw(canvas, paint);
    dContext->flushAndSubmit();
}

namespace {
constexpr SkRect kGlyphRect = SkRect::MakeLTRB(0, -1, 1, 0);

class RectGlyphDrawable final : public SkDrawable {
protected:
    SkRect onGetBounds() override { return kGlyphRect; }
    void onDraw(SkCanvas* canvas) override {
        canvas->drawRect(kGlyphRect, SkPaint());
    }
};

sk_sp<SkTypeface> make_path_typeface() {
    SkCustomTypefaceBuilder builder;
    builder.setGlyph(1, 1.0f, SkPath::Rect(kGlyphRect));
    return builder.detach();
}

sk_sp<SkTypeface> make_drawable_typeface() {
    SkCustomTypefaceBuilder builder;
    builder.setGlyph(1, 1.0f, sk_make_sp<RectGlyphDrawable>(), kGlyphRect);
    return builder.detach();
}

sk_sp<SkTextBlob> make_blob(sk_sp<SkTypeface> tf, SkScalar size) {
    SkTextBlobBuilder builder;
    builder.allocRun(SkFont(std::move(tf), size), 1, 0, 0).glyphs[0] = 1;
    return builder.make();
}

SkBitmap render_slug(SkSurface* surface, const sktext::gpu::Slug& slug, const SkPaint& paint) {
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    slug.draw(canvas, paint);
    SkBitmap bm;
    bm.allocPixels(surface->imageInfo());
    SkAssertResult(surface->readPixels(bm, 0, 0));
    return bm;
}
}  // namespace

// PathOpSubmitter stores each glyph's SkGlyphID and its lazily resolved SkPath in IDOrPath.
// Converting glyph IDs to SkPaths during submitDraws must not corrupt the glyph IDs written by
// subsequent Slug::serialize() calls.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_draw_then_serialize_PathSubRun,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    auto surface = SkSurfaces::RenderTarget(
            dContext, skgpu::Budgeted::kNo, SkImageInfo::MakeN32Premul(64, 64));
    REPORTER_ASSERT(reporter, surface);

    // 400pt exceeds fGlyphsAsPathsFontSize (324 on desktop, 384 on Android), forcing PathSubRun
    auto blob = make_blob(make_path_typeface(), /*size=*/400);
    REPORTER_ASSERT(reporter, blob != nullptr);

    SkPaint paint;
    paint.setAntiAlias(true);

    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {0, 64}, paint);
    REPORTER_ASSERT(reporter, slug != nullptr);

    // Before the first draw, PathOpSubmitter::fIDsOrPaths holds the original SkGlyphIDs
    sk_sp<SkData> preDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, preDrawData != nullptr && preDrawData->size() > 0);

    // Drawing the Slug triggers PathOpSubmitter::submitDraws, which resolves each glyph's SkPath.
    SkBitmap expectedPixels = render_slug(surface.get(), *slug, paint);
    REPORTER_ASSERT(reporter, expectedPixels.getColor(32, 32) == SK_ColorBLACK);

    // Verify that serializing after a draw still preserves the original SkGlyphIDs and data.
    sk_sp<SkData> postDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, postDrawData != nullptr && postDrawData->size() > 0);
    REPORTER_ASSERT(reporter, preDrawData->equals(postDrawData.get()));

    // Verify that deserializing postDrawData produces a Slug that renders matching pixels.
    sk_sp<sktext::gpu::Slug> deserializedSlug =
            sktext::gpu::Slug::Deserialize(postDrawData->data(), postDrawData->size(), nullptr);
    REPORTER_ASSERT(reporter, deserializedSlug != nullptr);
    SkBitmap actualPixels = render_slug(surface.get(), *deserializedSlug, paint);
    REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(expectedPixels, actualPixels));
}

// DrawableOpSubmitter stores each glyph's SkGlyphID and its resolved SkDrawable* in IDOrDrawable.
// Converting glyph IDs to drawables during submitDraws must not corrupt the glyph IDs written by
// subsequent Slug::serialize() calls.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_draw_then_serialize_DrawableSubRun,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    auto surface = SkSurfaces::RenderTarget(
            dContext, skgpu::Budgeted::kNo, SkImageInfo::MakeN32Premul(64, 64));
    REPORTER_ASSERT(reporter, surface);

    // 400pt exceeds fGlyphsAsPathsFontSize, forcing a drawable glyph through DrawableSubRun
    auto blob = make_blob(make_drawable_typeface(), /*size=*/400);
    SkPaint paint;
    paint.setAntiAlias(true);

    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {0, 64}, paint);
    REPORTER_ASSERT(reporter, slug != nullptr);

    // Before the first draw, DrawableOpSubmitter::fIDsOrDrawables holds the original SkGlyphID (1)
    sk_sp<SkData> preDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, preDrawData != nullptr && preDrawData->size() > 0);

    // Drawing the Slug triggers DrawableOpSubmitter::submitDraws, which resolves each glyph's
    // SkDrawable* pointer.
    SkBitmap expectedPixels = render_slug(surface.get(), *slug, paint);
    REPORTER_ASSERT(reporter, expectedPixels.getColor(32, 32) == SK_ColorBLACK);

    // Verify that serializing after a draw preserves the original SkGlyphID and data.
    sk_sp<SkData> postDrawData = slug->serialize();
    REPORTER_ASSERT(reporter, postDrawData != nullptr && postDrawData->size() > 0);
    REPORTER_ASSERT(reporter, preDrawData->equals(postDrawData.get()));

    // Verify that deserializing postDrawData produces a Slug that renders matching pixels.
    sk_sp<sktext::gpu::Slug> deserializedSlug =
            sktext::gpu::Slug::Deserialize(postDrawData->data(), postDrawData->size(), nullptr);
    REPORTER_ASSERT(reporter, deserializedSlug != nullptr);
    SkBitmap actualPixels = render_slug(surface.get(), *deserializedSlug, paint);
    REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(expectedPixels, actualPixels));
}

DEF_TEST(VertexFiller_boundsAndDeviceMatrix, reporter) {
    const SkRect bounds = SkRect::MakeXYWH(10, 10, 20, 20);
    const SkPoint positions[] = {{10, 10}};
    sktext::gpu::VertexFiller directFiller(skgpu::MaskFormat::kA8,
                                           SkMatrix::I(),
                                           bounds,
                                           SkSpan(positions, 1),
                                           /*canDrawDirect=*/true);

    // 1. Integer translation with supportBilerpAtlas = false: succeeds.
    {
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::I(), {5.0f, 5.0f},
                                                         /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, !b.isEmpty());
        REPORTER_ASSERT(reporter, m == SkMatrix::Translate(5.0f, 5.0f));
    }

    // 2. Fractional translation with supportBilerpAtlas = false: dropped.
    {
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::I(), {0.5f, 0.0f},
                                                         /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, b.isEmpty());
    }

    // 3. Matrix scale with supportBilerpAtlas = false: dropped.
    {
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::Scale(2.0f, 2.0f), {0.0f, 0.0f},
                                                         /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, b.isEmpty());
    }

    // 4. Fractional translation with supportBilerpAtlas = true: allowed.
    {
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::I(), {0.5f, 0.0f},
                                                         /*supportBilerpAtlas=*/true);
        REPORTER_ASSERT(reporter, !b.isEmpty());
        REPORTER_ASSERT(reporter, m == SkMatrix::Translate(0.5f, 0.0f));
    }

    // 5. Non-direct filler (e.g. transformed/SDFT) with supportBilerpAtlas = false: allowed.
    sktext::gpu::VertexFiller transformedFiller(skgpu::MaskFormat::kA8,
                                                SkMatrix::I(),
                                                bounds,
                                                SkSpan(positions, 1),
                                                /*canDrawDirect=*/false);
    {
        auto [b, m] = transformedFiller.boundsAndDeviceMatrix(SkMatrix::I(), {0.5f, 0.0f},
                                                              /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, !b.isEmpty());
        REPORTER_ASSERT(reporter, m == SkMatrix::Translate(0.5f, 0.0f));
    }

    // 6. Infinity translation bypass: must be dropped and canUseDirect returns false.
    {
        sktext::gpu::VertexFiller infFiller(skgpu::MaskFormat::kA8,
                                            SkMatrix::Translate(-2e38f, 0.0f),
                                            bounds,
                                            SkSpan(positions, 1),
                                            /*canDrawDirect=*/true);
        auto [b, m] = infFiller.boundsAndDeviceMatrix(SkMatrix::I(), {2e38f, 0.0f},
                                                      /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, b.isEmpty());
        auto [directOk, vec] = infFiller.canUseDirect(SkMatrix::Translate(2e38f, 0.0f));
        REPORTER_ASSERT(reporter, !directOk);
    }

    // 7. Magnitude translation bypass: must be dropped and canUseDirect returns false.
    {
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::I(), {1e25f, 0.0f},
                                                         /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, b.isEmpty());
        auto [directOk, vec] = directFiller.canUseDirect(SkMatrix::Translate(1e25f, 0.0f));
        REPORTER_ASSERT(reporter, !directOk);
    }

    // 8. Float rounding absorption bypass: fractional canvas translate must not be absorbed.
    {
        sktext::gpu::VertexFiller largeFiller(skgpu::MaskFormat::kA8,
                                              SkMatrix::Translate(8388608.0f, 0.0f),
                                              bounds,
                                              SkSpan(positions, 1),
                                              /*canDrawDirect=*/true);
        auto [b, m] = largeFiller.boundsAndDeviceMatrix(SkMatrix::Translate(0.5f, 0.0f),
                                                        {8388608.0f, 0.0f},
                                                        /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, b.isEmpty());
        auto [directOk, vec] = largeFiller.canUseDirect(SkMatrix::Translate(8388608.5f, 0.0f));
        REPORTER_ASSERT(reporter, !directOk);
    }

    // 9. Opposing large translations: drawOrigin = {3e6f, 0} and drawMatrix = Translate(-3e6f, 0)
    // cancel exactly in double precision to integer offset {0, 0}.
    {
        auto [b, m] = directFiller.boundsAndDeviceMatrix(SkMatrix::Translate(-3e6f, 0.0f),
                                                         {3e6f, 0.0f},
                                                         /*supportBilerpAtlas=*/false);
        REPORTER_ASSERT(reporter, !b.isEmpty());
        REPORTER_ASSERT(reporter, m == SkMatrix::I());
    }
}

namespace {
sk_sp<SkTextBlob> make_test_blob() {
    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface);
    font.setSize(16);

    static const char* kText = "A";
    SkTextBlobBuilder builder;
    const SkTextBlobBuilder::RunBuffer& runBuf = builder.allocRun(font, 1, 0, 0);
    font.textToGlyphs(kText, 1, SkTextEncoding::kUTF8, SkSpan(runBuf.glyphs, 1));
    return builder.make();
}

bool surface_has_non_white_pixels(SkSurface* surface) {
    SkBitmap bm;
    bm.allocPixels(surface->imageInfo());
    if (!surface->readPixels(bm, 0, 0)) {
        return false;
    }
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            if (bm.getColor(x, y) != SK_ColorWHITE) {
                return true;
            }
        }
    }
    return false;
}

template <typename FlushFn>
void run_slug_fractional_replay_test(SkSurface* surface,
                                     const sktext::gpu::Slug* slug,
                                     FlushFn flushFn,
                                     skiatest::Reporter* reporter) {
    SkCanvas* canvas = surface->getCanvas();
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    // 1. Integer translation: should draw.
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->translate(5.0f, 5.0f);
    slug->draw(canvas, paint);
    canvas->restore();
    flushFn();
    REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface));

    // 2. Fractional translation: must be dropped.
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->translate(0.5f, 0.0f);
    slug->draw(canvas, paint);
    canvas->restore();
    flushFn();
    REPORTER_ASSERT(reporter, !surface_has_non_white_pixels(surface));

    // 3. Matrix scale: must be dropped.
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->scale(1.5f, 1.5f);
    slug->draw(canvas, paint);
    canvas->restore();
    flushFn();
    REPORTER_ASSERT(reporter, !surface_has_non_white_pixels(surface));
}
}  // namespace

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_fractional_replay_ganesh,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(64, 64);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);

    sk_sp<SkTextBlob> blob = make_test_blob();
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {10, 30}, SkPaint());
    REPORTER_ASSERT(reporter, slug != nullptr);

    run_slug_fractional_replay_test(surface.get(), slug.get(), [&]() {
        dContext->flushAndSubmit();
    }, reporter);
}

DEF_GANESH_TEST_FOR_CONTEXTS(Slug_fractional_replay_bilerp_ganesh,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             [](GrContextOptions* options) {
                                 options->fSupportBilerpFromGlyphAtlas = true;
                             },
                             CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(64, 64);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);

    sk_sp<SkTextBlob> blob = make_test_blob();
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {10, 30}, SkPaint());
    REPORTER_ASSERT(reporter, slug != nullptr);

    SkCanvas* canvas = surface->getCanvas();
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    // 1. Identity draw: renders text.
    canvas->clear(SK_ColorWHITE);
    slug->draw(canvas, paint);
    dContext->flushAndSubmit();
    REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface.get()));

    // 2. Fractional translation: when fSupportBilerpFromGlyphAtlas is true, text must draw.
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->translate(0.5f, 0.0f);
    slug->draw(canvas, paint);
    canvas->restore();
    dContext->flushAndSubmit();
    REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface.get()));

    // 3. Matrix scale: when fSupportBilerpFromGlyphAtlas is true, text must draw.
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->scale(1.5f, 1.5f);
    slug->draw(canvas, paint);
    canvas->restore();
    dContext->flushAndSubmit();
    REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface.get()));
}

#if defined(SK_GRAPHITE)
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(Slug_fractional_replay_graphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    using namespace skgpu::graphite;
    if (context->supportsProtectedContent()) {
        return;
    }
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo info = SkImageInfo::MakeN32Premul(64, 64);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface);

    sk_sp<SkTextBlob> blob = make_test_blob();
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {10, 30}, SkPaint());
    REPORTER_ASSERT(reporter, slug != nullptr);

    run_slug_fractional_replay_test(surface.get(), slug.get(), [&]() {
        std::unique_ptr<Recording> recording = recorder->snap();
        context->insertRecording({recording.get()});
        context->submit(SyncToCpu::kYes);
    }, reporter);
}
#endif

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(Slug_deserialize_rejects_fractional_direct_positions,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(64, 64);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);

    sk_sp<SkTextBlob> blob = make_test_blob();
    sk_sp<sktext::gpu::Slug> slug =
            sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {10, 30}, SkPaint());
    REPORTER_ASSERT(reporter, slug != nullptr);

    sk_sp<SkData> data = slug->serialize();
    REPORTER_ASSERT(reporter, data != nullptr && data->size() >= 52);
    if (!data || data->size() < 52) {
        return;
    }

    // 1. Verify that the original untampered Slug deserializes successfully.
    sk_sp<sktext::gpu::Slug> validSlug =
            sktext::gpu::Slug::Deserialize(data->data(), data->size(), nullptr);
    REPORTER_ASSERT(reporter, validSlug != nullptr);

    // In a single-glyph direct mask SubRun, the last 12 bytes of the serialized buffer
    // are the point array: uint32_t count (1), float x, float y.
    const size_t countOffset = data->size() - 12;
    const size_t xOffset = data->size() - 8;
    const size_t yOffset = data->size() - 4;

    uint32_t count = 0;
    float origX = 0.0f;
    float origY = 0.0f;
    memcpy(&count, (const uint8_t*)data->data() + countOffset, sizeof(count));
    memcpy(&origX, (const uint8_t*)data->data() + xOffset, sizeof(origX));
    memcpy(&origY, (const uint8_t*)data->data() + yOffset, sizeof(origY));

    REPORTER_ASSERT(reporter, count == 1);
    REPORTER_ASSERT(reporter, SkScalarIsInt(origX) && SkScalarIsInt(origY));

    auto try_deserialize_mutated = [](const SkData* src, size_t offset, auto mutatedValue) -> bool {
        SkTDArray<uint8_t> mutatedData;
        mutatedData.append(src->size(), (const uint8_t*)src->data());
        memcpy(mutatedData.data() + offset, &mutatedValue, sizeof(mutatedValue));
        sk_sp<sktext::gpu::Slug> result =
                sktext::gpu::Slug::Deserialize(mutatedData.data(), mutatedData.size(), nullptr);
        return result != nullptr;
    };

    // 2. Fractional X coordinate must be rejected.
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), xOffset, origX + 0.5f));

    // 3. Fractional Y coordinate must be rejected.
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), yOffset, origY + 0.5f));

    // 4. Non-finite coordinates must be rejected.
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), xOffset, SK_ScalarNaN));
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), yOffset, SK_ScalarInfinity));

    // 5. Extremely large float (out of magnitude bounds) must be rejected.
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), xOffset, 1e25f));
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), yOffset, -1e25f));

    // 6. Flipping canDrawDirect to 0 in a DirectMaskSubRun must be rejected.
    const size_t canDrawDirectOffset = data->size() - 52;
    uint32_t origCanDrawDirect = 0;
    memcpy(&origCanDrawDirect, (const uint8_t*)data->data() + canDrawDirectOffset,
           sizeof(origCanDrawDirect));
    REPORTER_ASSERT(reporter, origCanDrawDirect == 1);
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), canDrawDirectOffset, 0u));

    // 7. Non-finite creationMatrix must be rejected.
    const size_t creationMatrixOffset = data->size() - 48;
    REPORTER_ASSERT(reporter,
                    !try_deserialize_mutated(data.get(), creationMatrixOffset, SK_ScalarNaN));

    // 8. creationMatrix mismatch relative to initialPositionMatrix must be rejected.
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), creationMatrixOffset, 2.0f));

    // 9. Slug creation with extreme coordinates must not assert in VertexFiller::Make.
    {
        surface->getCanvas()->save();
        surface->getCanvas()->translate(1e10f, 0);
        sk_sp<sktext::gpu::Slug> largeSlug =
                sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {0, 0}, SkPaint());
        surface->getCanvas()->restore();
        REPORTER_ASSERT(reporter, largeSlug == nullptr);
    }

    // 10. Deserialization must reject non-finite origins.
    const size_t originXOffset = 16;
    const size_t originYOffset = 20;
    REPORTER_ASSERT(reporter,
                    !try_deserialize_mutated(data.get(), originXOffset, SK_ScalarInfinity));
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), originXOffset, SK_ScalarNaN));
    REPORTER_ASSERT(reporter,
                    !try_deserialize_mutated(data.get(), originYOffset, SK_ScalarInfinity));
    REPORTER_ASSERT(reporter, !try_deserialize_mutated(data.get(), originYOffset, SK_ScalarNaN));

    // Deserializing with oversized origin succeeds because origin is finite, but canDraw
    // without bilerp atlas drops the draw due to translation bounds.
    {
        SkTDArray<uint8_t> mutatedData;
        mutatedData.append(data->size(), (const uint8_t*)data->data());
        float largeOrigin = 8388608.0f;
        memcpy(mutatedData.data() + originXOffset, &largeOrigin, sizeof(largeOrigin));
        sk_sp<sktext::gpu::Slug> result =
                sktext::gpu::Slug::Deserialize(mutatedData.data(), mutatedData.size(), nullptr);
        REPORTER_ASSERT(reporter, result != nullptr);
        auto slugImpl = static_cast<const sktext::gpu::SlugImpl*>(result.get());
        REPORTER_ASSERT(reporter, !slugImpl->canDraw(SkMatrix::I(), /*supportBilerpAtlas=*/false));
    }

    // 11. Transformed mask slug deserialization must reject coordinates that overflow safe bounds.
    {
        surface->getCanvas()->save();
        surface->getCanvas()->rotate(45.0f);
        sk_sp<sktext::gpu::Slug> xformedSlug =
                sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {10, 30}, SkPaint());
        surface->getCanvas()->restore();
        REPORTER_ASSERT(reporter, xformedSlug != nullptr);

        sk_sp<SkData> xformedData = xformedSlug->serialize();
        REPORTER_ASSERT(reporter, xformedData != nullptr && xformedData->size() >= 12);

        // Mutating leftTop to FLT_MAX causes glyph_bounds_from_left_top to overflow to +Infinity.
        const size_t xfPointXOffset = xformedData->size() - 8;
        const size_t xfPointYOffset = xformedData->size() - 4;

        REPORTER_ASSERT(reporter, !try_deserialize_mutated(
                xformedData.get(), xfPointXOffset, 3.4028234e38f));
        REPORTER_ASSERT(reporter, !try_deserialize_mutated(
                xformedData.get(), xfPointYOffset, 3.4028234e38f));
        REPORTER_ASSERT(reporter, !try_deserialize_mutated(
                xformedData.get(), xfPointXOffset, -3.4028234e38f));
        REPORTER_ASSERT(reporter, !try_deserialize_mutated(
                xformedData.get(), xfPointYOffset, -3.4028234e38f));
    }

    // 12. ConvertBlob with coordinates near FLT_MAX must not produce slugs with extreme/infinite
    // bounds that fail deserialization.
    {
        surface->getCanvas()->save();
        surface->getCanvas()->rotate(45.0f);
        sk_sp<sktext::gpu::Slug> nearMaxSlug =
                sktext::gpu::Slug::ConvertBlob(
                        surface->getCanvas(), *blob, {3.402823e38f, 0}, SkPaint());
        surface->getCanvas()->restore();
        // The extreme coordinates exceed kMaxGlyphPositionCoord and are cleanly dropped.
        REPORTER_ASSERT(reporter, nearMaxSlug == nullptr);
    }

    // 13. Perspective initialPositionMatrix must deserialize successfully for non-direct runs.
    {
        SkMatrix persMatrix;
        persMatrix.setPerspY(0.001f);
        surface->getCanvas()->save();
        surface->getCanvas()->concat(persMatrix);
        sk_sp<sktext::gpu::Slug> persSlug =
                sktext::gpu::Slug::ConvertBlob(surface->getCanvas(), *blob, {10, 30}, SkPaint());
        surface->getCanvas()->restore();
        REPORTER_ASSERT(reporter, persSlug != nullptr);
        sk_sp<SkData> persData = persSlug->serialize();
        REPORTER_ASSERT(reporter, persData != nullptr && persData->size() > 0);
        sk_sp<sktext::gpu::Slug> deserializedPersSlug =
                sktext::gpu::Slug::Deserialize(persData->data(), persData->size(), nullptr);
        REPORTER_ASSERT(reporter, deserializedPersSlug != nullptr);
        auto persSlugImpl = static_cast<const sktext::gpu::SlugImpl*>(deserializedPersSlug.get());
        REPORTER_ASSERT(reporter, persSlugImpl->canDraw(persMatrix, /*supportBilerpAtlas=*/false));
    }

    // 14. Large canvas translation (> kMaxDirectCoord) with a glyph that maps to {0, 0}
    // is safely dropped by prepare_for_direct_mask_drawing instead of crashing.
    {
        auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
        SkFont testFont(typeface, 16);
        SkTextBlobBuilder posBuilder;
        const SkTextBlobBuilder::RunBuffer& runBuf = posBuilder.allocRunPos(testFont, 1);
        testFont.textToGlyphs("A", 1, SkTextEncoding::kUTF8, SkSpan(runBuf.glyphs, 1));
        runBuf.points()[0] = {-5e6f, 0};
        sk_sp<SkTextBlob> posBlob = posBuilder.make();
        REPORTER_ASSERT(reporter, posBlob != nullptr);

        surface->getCanvas()->save();
        surface->getCanvas()->translate(5e6f, 0);
        sk_sp<sktext::gpu::Slug> largeTransSlug =
                sktext::gpu::Slug::ConvertBlob(
                        surface->getCanvas(), *posBlob, {0, 0}, SkPaint());
        surface->getCanvas()->restore();
        // The extreme translation is rejected for direct mask drawing and cleanly dropped.
        REPORTER_ASSERT(reporter, largeTransSlug == nullptr);
    }

    // 15. Opposing large translation Slug can draw when replayed with creation matrix.
    {
        surface->getCanvas()->save();
        surface->getCanvas()->translate(-3e6f, 0);
        sk_sp<sktext::gpu::Slug> opposingSlug =
                sktext::gpu::Slug::ConvertBlob(
                        surface->getCanvas(), *blob, {3e6f, 30}, SkPaint());
        surface->getCanvas()->restore();
        REPORTER_ASSERT(reporter, opposingSlug != nullptr);
        auto oppSlugImpl = static_cast<const sktext::gpu::SlugImpl*>(opposingSlug.get());
        REPORTER_ASSERT(reporter, oppSlugImpl->canDraw(SkMatrix::Translate(-3e6f, 0),
                                                       /*supportBilerpAtlas=*/false));

        // Verify end-to-end execution through GPU op creation and vertex generation.
        SkPaint testPaint;
        testPaint.setColor(SK_ColorBLACK);
        surface->getCanvas()->clear(SK_ColorWHITE);
        surface->getCanvas()->save();
        surface->getCanvas()->translate(-3e6f, 0);
        opposingSlug->draw(surface->getCanvas(), testPaint);
        surface->getCanvas()->restore();
        dContext->flushAndSubmit();
        REPORTER_ASSERT(reporter, surface_has_non_white_pixels(surface.get()));
    }
}



