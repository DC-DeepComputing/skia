/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkTDArray.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkStrikeCache.h"
#include "src/gpu/MaskFormat.h"
#include "src/text/gpu/SlugImpl.h"
#include "src/utils/SkFloatUtils.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/ProcsUtils.h"
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


    // The creationMatrix of VertexFiller inside SDFTSubRun is located 220 bytes in.
    // We can overwrite it to have a matrix with perspective.
    if (size < 220 + 9 * sizeof(float)) {
        ERRORF(reporter, "Serialized Slug is too small to contain creationMatrix!");
        return;
    }
    float* f = reinterpret_cast<float*>(writableData.get() + 220);
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

    // Corrupt the maskFormat of VertexFiller inside SDFTSubRun at offset 212 from
    // kA8 (0) to kARGB (2).
    if (size < 216) {
        ERRORF(reporter, "Serialized Slug is too small to contain maskFormat!");
        return;
    }
    int32_t* formatPtr = reinterpret_cast<int32_t*>(writableData.get() + 212);
    if (*formatPtr != 0) {
        ERRORF(reporter, "maskFormat is not kA8 at expected offset 212!");
        return;
    }
    *formatPtr = static_cast<int32_t>(skgpu::MaskFormat::kARGB);

    // Since our validation fix will be active, this must fail and return nullptr. Previously, when
    // we went to draw it, the unexpected mask format would cause issues.
    sk_sp<sktext::gpu::Slug> forgedSlug =
            sktext::gpu::Slug::Deserialize(writableData.get(), size, nullptr);
    REPORTER_ASSERT(reporter, forgedSlug == nullptr);
}

DEF_GANESH_TEST_FOR_CONTEXTS(Slug_PictureSerialization,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             set_sdf_options,
                             CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    REPORTER_ASSERT(reporter, surface);
    auto* canvas = surface->getCanvas();

    auto typeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    SkFont font(typeface, 24);
    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kAntiAlias);
    auto blob = SkTextBlob::MakeFromString("Hello Slug!", font);

    SkFont bigFont(typeface, 300);
    bigFont.setEdging(SkFont::Edging::kAntiAlias);
    auto pathBlob = SkTextBlob::MakeFromString("P", bigFont);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);

    // Create a direct mask slug, an SDFT slug, and a path slug.
    sk_sp<sktext::gpu::Slug> directSlug =
            sktext::gpu::Slug::ConvertBlob(canvas, *blob, {10, 40}, paint);
    REPORTER_ASSERT(reporter, directSlug != nullptr);

    canvas->save();
    canvas->scale(2.0f, 2.0f);
    sk_sp<sktext::gpu::Slug> sdfSlug =
            sktext::gpu::Slug::ConvertBlob(canvas, *blob, {5, 45}, paint);
    canvas->restore();
    REPORTER_ASSERT(reporter, sdfSlug != nullptr);

    sk_sp<sktext::gpu::Slug> pathSlug =
            sktext::gpu::Slug::ConvertBlob(canvas, *pathBlob, {10, 230}, paint);
    REPORTER_ASSERT(reporter, pathSlug != nullptr);

    // Record into an SkPicture before drawing to surface (or after).
    SkPictureRecorder recorder;
    SkCanvas* recCanvas = recorder.beginRecording(256, 256);
    recCanvas->clear(SK_ColorWHITE);
    directSlug->draw(recCanvas, paint);
    recCanvas->save();
    recCanvas->scale(2.0f, 2.0f);
    sdfSlug->draw(recCanvas, paint);
    recCanvas->restore();
    pathSlug->draw(recCanvas, paint);
    sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();

    // Draw the slugs directly to surface to get expected image.
    canvas->clear(SK_ColorWHITE);
    directSlug->draw(canvas, paint);
    canvas->save();
    canvas->scale(2.0f, 2.0f);
    sdfSlug->draw(canvas, paint);
    canvas->restore();
    pathSlug->draw(canvas, paint);

    SkBitmap expected;
    expected.allocN32Pixels(256, 256);
    REPORTER_ASSERT(reporter, surface->readPixels(expected, 0, 0));

    SkSerialProcs sProcs = ToolUtils::default_serial_procs();
    sk_sp<SkData> skpData = pic->serialize(&sProcs);
    REPORTER_ASSERT(reporter, skpData && skpData->size() > 0);

    // Purge all strikes from the global strike cache to ensure deserialization reconstructs them
    // from the embedded strike data.
    SkStrikeCache::GlobalStrikeCache()->purgeAll();

    SkDeserialProcs dProcs = ToolUtils::default_deserial_procs();
    sk_sp<SkPicture> deserializedPic = SkPicture::MakeFromData(skpData.get(), &dProcs);
    REPORTER_ASSERT(reporter, deserializedPic != nullptr);

    canvas->clear(SK_ColorTRANSPARENT);
    canvas->drawPicture(deserializedPic);

    SkBitmap actual;
    actual.allocN32Pixels(256, 256);
    REPORTER_ASSERT(reporter, surface->readPixels(actual, 0, 0));

    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            if (expected.getColor(x, y) != actual.getColor(x, y)) {
                ERRORF(reporter, "Pixel mismatch at (%d, %d): expected 0x%08x, got 0x%08x",
                       x, y, expected.getColor(x, y), actual.getColor(x, y));
                return;
            }
        }
    }
}
