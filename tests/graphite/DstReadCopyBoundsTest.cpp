/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "tests/Test.h"

#include <memory>

using namespace skgpu::graphite;

// Verifies that when DstReadStrategy::kTextureCopy uses an approximate-fit scratch texture from
// ResourceCache, fragments that rasterize outside the CPU-computed dstReadBounds (e.g. due to fp32
// rounding of conic tessellation at large local coordinates) clamp to the initialized subset of
// DstCopy and never sample stale texels from the uninitialized padding.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(DstReadCopyBoundsClampTest,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    if (context->supportsProtectedContent()) {
        return;
    }

    const Caps* caps = context->priv().caps();
    if (caps->getDstReadStrategy() != DstReadStrategy::kTextureCopy) {
        return;
    }

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);

    // Step 1: Populate a 1024x512 DstCopy scratch texture in ResourceCache with a sentinel color
    // (SK_ColorGREEN) across all 1024x512 texels, then release it back to the cache.
    {
        const SkImageInfo seedII =
                SkImageInfo::Make(1024, 512, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        sk_sp<SkSurface> seedSurface = SkSurfaces::RenderTarget(recorder.get(), seedII);
        REPORTER_ASSERT(reporter, seedSurface);

        SkCanvas* seedCanvas = seedSurface->getCanvas();
        seedCanvas->clear(SK_ColorGREEN);

        SkPaint multiplyWhite;
        multiplyWhite.setColor(SK_ColorWHITE);
        multiplyWhite.setBlendMode(SkBlendMode::kMultiply);
        seedCanvas->drawRect(SkRect::MakeWH(1024, 512), multiplyWhite);

        std::unique_ptr<Recording> seedRec = recorder->snap();
        REPORTER_ASSERT(reporter, seedRec);
        REPORTER_ASSERT(reporter, context->insertRecording({seedRec.get()}));
        REPORTER_ASSERT(reporter, context->submit(SyncToCpu::kYes));
    }

    // Step 2: On a fresh 1600x512 target cleared to SK_ColorBLUE, draw a convex conic at huge local
    // X = 4294978560 (ULP = 512) translated by (-X + 1024, 0) with a clip [508, 0, 1600, 512].
    //   - CPU drawBounds = [508, 0, 1024, 512] (516x512 -> GetApproxSize allocates 1024x512,
    //     reusing the SK_ColorGREEN scratch texture and writing only columns 0..515).
    //   - GPU conic De Casteljau at X rounds up to X + 512, shading overhanging triangles across
    //     device x in [1024, 1536) (which pass the [508, 0, 1600, 512] scissor).
    //   - With white * kMultiply, output == sampled dstColor. Clamping dstSampler coordinates must
    //     prevent any pixel from reading SK_ColorGREEN from columns 516..1023.
    constexpr int kTargetW = 1600;
    constexpr int kTargetH = 512;
    const SkImageInfo targetII =
            SkImageInfo::Make(kTargetW, kTargetH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> targetSurface = SkSurfaces::RenderTarget(recorder.get(), targetII);
    REPORTER_ASSERT(reporter, targetSurface);

    SkCanvas* canvas = targetSurface->getCanvas();
    canvas->clear(SK_ColorBLUE);

    constexpr float kHugeX = 4294978560.0f;  // 2^32 + 22 * 512, ULP = 512
    SkPathBuilder pb;
    pb.moveTo(kHugeX - 1024.0f, 0.0f);
    pb.lineTo(kHugeX, 0.0f);
    pb.conicTo({kHugeX, 256.0f}, {kHugeX, 512.0f}, 0.7406733f);
    pb.lineTo(kHugeX - 1024.0f, 512.0f);
    pb.close();
    SkPath path = pb.detach();

    canvas->save();
    canvas->clipRect(SkRect::MakeLTRB(508, 0, kTargetW, kTargetH));
    canvas->translate(-kHugeX + 1024.0f, 0.0f);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);
    paint.setBlendMode(SkBlendMode::kMultiply);
    canvas->drawPath(path, paint);
    canvas->restore();

    std::unique_ptr<Recording> rec = recorder->snap();
    REPORTER_ASSERT(reporter, rec);
    REPORTER_ASSERT(reporter, context->insertRecording({rec.get()}));

    SkBitmap readback;
    readback.allocPixels(targetII);
    REPORTER_ASSERT(reporter, targetSurface->readPixels(readback, 0, 0));

    int leakedPixels = 0;
    for (int y = 0; y < kTargetH; ++y) {
        for (int x = 0; x < kTargetW; ++x) {
            SkColor c = readback.getColor(x, y);
            if (c != SK_ColorBLUE) {
                ++leakedPixels;
            }
        }
    }
    REPORTER_ASSERT(reporter,
                    leakedPixels == 0,
                    "Expected all pixels to remain SK_ColorBLUE (no stale DstCopy read), "
                    "but %d pixels differed (sample at (1100, 256) = 0x%08x)",
                    leakedPixels,
                    readback.getColor(1100, 256));

    // Step 3: Verify exact in-bounds texel mapping at all corners of a non-power-of-two DstCopy
    // subset (offset (37, 29), size 300x200 -> approximate backing 512x256).
    {
        const SkImageInfo subII =
                SkImageInfo::Make(400, 300, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        sk_sp<SkSurface> subSurface = SkSurfaces::RenderTarget(recorder.get(), subII);
        REPORTER_ASSERT(reporter, subSurface);

        SkCanvas* subCanvas = subSurface->getCanvas();
        subCanvas->clear(SK_ColorBLACK);

        // Paint distinct colors at the exact 4 corner pixels of [37, 29, 337, 229).
        SkPaint srcPaint;
        srcPaint.setBlendMode(SkBlendMode::kSrc);
        srcPaint.setColor(SkColorSetARGB(255, 200, 0, 0));
        subCanvas->drawIRect(SkIRect::MakeXYWH(37, 29, 1, 1), srcPaint);
        srcPaint.setColor(SkColorSetARGB(255, 0, 200, 0));
        subCanvas->drawIRect(SkIRect::MakeXYWH(336, 29, 1, 1), srcPaint);
        srcPaint.setColor(SkColorSetARGB(255, 0, 0, 200));
        subCanvas->drawIRect(SkIRect::MakeXYWH(37, 228, 1, 1), srcPaint);
        srcPaint.setColor(SkColorSetARGB(255, 200, 200, 0));
        subCanvas->drawIRect(SkIRect::MakeXYWH(336, 228, 1, 1), srcPaint);

        // Blend RGB(100, 100, 100) with kLighten over [37, 29, 337, 229) so each corner's output
        // depends on sampling its exact corresponding dst texel.
        SkPaint lightenPaint;
        lightenPaint.setColor(SkColorSetARGB(255, 100, 100, 100));
        lightenPaint.setBlendMode(SkBlendMode::kLighten);
        subCanvas->drawIRect(SkIRect::MakeXYWH(37, 29, 300, 200), lightenPaint);

        std::unique_ptr<Recording> subRec = recorder->snap();
        REPORTER_ASSERT(reporter, subRec);
        REPORTER_ASSERT(reporter, context->insertRecording({subRec.get()}));

        SkBitmap subBM;
        subBM.allocPixels(subII);
        REPORTER_ASSERT(reporter, subSurface->readPixels(subBM, 0, 0));

        REPORTER_ASSERT(reporter, subBM.getColor(37, 29) == SkColorSetARGB(255, 200, 100, 100));
        REPORTER_ASSERT(reporter, subBM.getColor(336, 29) == SkColorSetARGB(255, 100, 200, 100));
        REPORTER_ASSERT(reporter, subBM.getColor(37, 228) == SkColorSetARGB(255, 100, 100, 200));
        REPORTER_ASSERT(reporter, subBM.getColor(336, 228) == SkColorSetARGB(255, 200, 200, 100));
        // Interior black pixel becomes (100, 100, 100)
        REPORTER_ASSERT(reporter, subBM.getColor(100, 100) == SkColorSetARGB(255, 100, 100, 100));
        // Outside pixel at (337, 228) untouched black
        REPORTER_ASSERT(reporter, subBM.getColor(337, 228) == SK_ColorBLACK);
    }
}
