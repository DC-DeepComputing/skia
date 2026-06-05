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
#include "src/text/gpu/SlugImpl.h"
#include "src/utils/SkFloatUtils.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/chromium/Slug.h"
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

DEF_GANESH_TEST_FOR_CONTEXTS(Slug_PerspectiveStrideMismatch,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             set_sdf_options,
                             CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();

    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface(SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info));
    if (!surface) {
        return;
    }
    auto canvas = surface->getCanvas();

    // 1. Create a distinct scale matrix for creation.
    SkMatrix creationMatrix = SkMatrix::Scale(1.2345f, 6.7890f);

    // Save canvas state and set matrix to creationMatrix.
    canvas->save();
    canvas->setMatrix(creationMatrix);

    // 2. Prepare text blob. Use size 24 (with creation scale 6.789 -> ~162.9) to force SDFT.
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

    // 3. Convert the blob to a slug. Use {11, 13} as a robust origin sentinel.
    sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(canvas, *blob, {11, 13}, paint);
    canvas->restore();

    if (!slug) {
        return;
    }

    // 4. Serialize the slug. (Our doFlatten injection is removed, so we forge the bytes directly here!)
    sk_sp<SkData> data = slug->serialize();
    if (!data || data->size() == 0) {
        return;
    }

    // Copy data to a writable buffer.
    size_t size = data->size();
    std::unique_ptr<uint8_t[]> writableData(new uint8_t[size]);
    memcpy(writableData.get(), data->data(), size);

    // Verify and forge fOrigin (offset 16-24) from {11,13} to {0,0}:
    // fSourceBounds: 16 bytes (left, top, right, bottom) -> offset 0 to 16
    // fOrigin: 8 bytes (x, y) -> offset 16 to 24
    {
        SkFloatingPoint<float, 4> origX(*reinterpret_cast<float*>(writableData.get() + 16));
        SkFloatingPoint<float, 4> origY(*reinterpret_cast<float*>(writableData.get() + 20));
        if (!origX.AlmostEquals(SkFloatingPoint<float, 4>(11.0f)) ||
            !origY.AlmostEquals(SkFloatingPoint<float, 4>(13.0f))) {
            ERRORF(reporter, "fOrigin sentinel {11, 13} not found at expected offset!");
            return;
        }
        *reinterpret_cast<float*>(writableData.get() + 16) = 0.0f;
        *reinterpret_cast<float*>(writableData.get() + 20) = 0.0f;
    }

    // Use a search loop with SkFloatingPoint to find and forge the Scale(6.75, 6.75) creationMatrix.
    bool found = false;
    for (size_t i = 100; i <= size - 9 * sizeof(float); i += 4) {
        float* f = reinterpret_cast<float*>(writableData.get() + i);

        SkFloatingPoint<float, 4> f0(f[0]);
        SkFloatingPoint<float, 4> f1(f[1]);
        SkFloatingPoint<float, 4> f2(f[2]);
        SkFloatingPoint<float, 4> f3(f[3]);
        SkFloatingPoint<float, 4> f4(f[4]);
        SkFloatingPoint<float, 4> f5(f[5]);
        SkFloatingPoint<float, 4> f6(f[6]);
        SkFloatingPoint<float, 4> f7(f[7]);
        SkFloatingPoint<float, 4> f8(f[8]);

        if (f0.AlmostEquals(SkFloatingPoint<float, 4>(6.7500f)) &&
            f1.AlmostEquals(SkFloatingPoint<float, 4>(0.0f)) &&
            f2.AlmostEquals(SkFloatingPoint<float, 4>(0.0f)) &&
            f3.AlmostEquals(SkFloatingPoint<float, 4>(0.0f)) &&
            f4.AlmostEquals(SkFloatingPoint<float, 4>(6.7500f)) &&
            f5.AlmostEquals(SkFloatingPoint<float, 4>(0.0f)) &&
            f6.AlmostEquals(SkFloatingPoint<float, 4>(0.0f)) &&
            f7.AlmostEquals(SkFloatingPoint<float, 4>(0.0f)) &&
            f8.AlmostEquals(SkFloatingPoint<float, 4>(1.0f))) {
            // Found the creation matrix inside VertexFiller!
            // Forge it to be our perspective matrix: [1, 0, 0, 0, 1, 0, 0.0078125, 0, 1]
            f[0] = 1.0f; f[1] = 0.0f; f[2] = 0.0f;
            f[3] = 0.0f; f[4] = 1.0f; f[5] = 0.0f;
            f[6] = 0.0078125f; f[7] = 0.0f; f[8] = 1.0f;
            found = true;
            break;
        }
    }

    if (!found) {
        ERRORF(reporter, "Could not find creation matrix in serialized Slug");
        return;
    }

    // 5. Deserialize the forged slug.
    sk_sp<sktext::gpu::Slug> forgedSlug = sktext::gpu::Slug::Deserialize(writableData.get(), size, nullptr);
    if (!forgedSlug) {
        ERRORF(reporter, "Failed to deserialize forged slug");
        return;
    }

    // 6. Replay on a canvas with the forged perspective matrix.
    SkMatrix positionMatrix;
    positionMatrix.setAll(1.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f,
                          0.0078125f, 0.0f, 1.0f);

    canvas->save();
    canvas->setMatrix(positionMatrix);

    // Draw the forged slug. This should trigger the debug assert/ASAN crash if unfixed!
    forgedSlug->draw(canvas, paint);

    canvas->restore();

    // Force execution of the draw op by reading back pixels using safe allocN32Pixels
    SkBitmap bm;
    bm.allocN32Pixels(256, 256);
    surface->readPixels(bm, 0, 0);
}

