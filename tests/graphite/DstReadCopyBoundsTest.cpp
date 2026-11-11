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
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/draw/DrawOrder.h"
#include "src/gpu/graphite/draw/DrawParams.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/NonMSAAClip.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "tests/Test.h"

#include <memory>
#include <optional>

using namespace skgpu::graphite;

// Directly verifies RenderStep::getScissor enforces a tight rounded-out drawBounds scissor for
// kTextureCopy dst-reading draws (instead of eliding the scissor via transformedShapeBounds), and
// restores the wider scissor on subsequent non-dst-reading draws.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(DstReadCopyScissorTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    const RenderStep& step = context->priv().rendererProvider()->nonAABounds()->step(0);
    const Transform identity = Transform::Identity();
    const SkIRect deviceBounds = SkIRect::MakeWH(1600, 512);
    const SkIRect clipScissor = SkIRect::MakeLTRB(508, 0, 1600, 512);

    const Rect shapeBounds(508.0f, 0.0f, 1024.2f, 512.0f);
    const Geometry geom{Shape{shapeBounds}};
    const Clip clip(shapeBounds, shapeBounds, clipScissor, NonMSAAClip{}, /*shader=*/nullptr);

    const DrawParams dstReadParams(identity,
                                   geom,
                                   clip,
                                   DrawOrder(PaintersDepth::First()),
                                   /*stroke=*/nullptr,
                                   BarrierType::kNone,
                                   /*readsDst=*/true);

    // Under kFramebufferFetch, getScissor elides the scissor change because shapeBounds clipped to
    // clipScissor already equals drawBounds.
    REPORTER_ASSERT(reporter,
                    !step.getScissor(dstReadParams,
                                     clipScissor,
                                     deviceBounds,
                                     DstReadStrategy::kFramebufferFetch)
                             .has_value());

    // Under kTextureCopy, getScissor must tighten the scissor to the rounded-out drawBounds.
    const SkIRect expectedTightScissor = SkIRect::MakeLTRB(508, 0, 1025, 512);
    std::optional<SkIRect> newScissor = step.getScissor(
            dstReadParams, clipScissor, deviceBounds, DstReadStrategy::kTextureCopy);
    REPORTER_ASSERT(reporter, newScissor.has_value() && *newScissor == expectedTightScissor);

    // Subsequent steps with the same tight scissor already active require no state change.
    REPORTER_ASSERT(reporter,
                    !step.getScissor(dstReadParams,
                                     expectedTightScissor,
                                     deviceBounds,
                                     DstReadStrategy::kTextureCopy)
                             .has_value());

    // A subsequent non-dst-reading draw outside expectedTightScissor restores deviceBounds.
    const Rect otherBounds(1200.0f, 100.0f, 1300.0f, 200.0f);
    const Geometry otherGeom{Shape{otherBounds}};
    const Clip otherClip(otherBounds, otherBounds, deviceBounds, NonMSAAClip{}, /*shader=*/nullptr);
    const DrawParams nonDstReadParams(identity,
                                      otherGeom,
                                      otherClip,
                                      DrawOrder(PaintersDepth::First()),
                                      /*stroke=*/nullptr,
                                      BarrierType::kNone,
                                      /*readsDst=*/false);
    std::optional<SkIRect> restoredScissor = step.getScissor(
            nonDstReadParams, expectedTightScissor, deviceBounds, DstReadStrategy::kTextureCopy);
    REPORTER_ASSERT(reporter, restoredScissor.has_value() && *restoredScissor == deviceBounds);
}

// End-to-end GPU verification on DstReadStrategy::kTextureCopy backends that per-draw drawBounds
// scissoring prevents overhanging GPU fragments from sampling uninitialized padding of a recycled
// approximate-fit DstCopy texture or overhanging into sibling draws' regions.
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
    //   - CPU drawBounds = [508, 0, 1025, 512] (517x512 -> GetApproxSize allocates 1024x512,
    //     reusing the SK_ColorGREEN scratch texture and writing only columns 0..516).
    //   - Tight per-draw hardware scissoring must discard any overhanging GPU fragments past x=1025
    //     so zero pixels sample SK_ColorGREEN from columns 517..1023.
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

    // Step 3: Verify per-draw scissoring when two dst-reading draws share a single DrawPass (so the
    // pass-level dstReadBounds union spans [508, 0, 1500, 512]) followed by a non-dst-reading draw.
    {
        canvas->clear(SK_ColorWHITE);

        canvas->save();
        canvas->clipRect(SkRect::MakeLTRB(508, 0, kTargetW, kTargetH));
        canvas->translate(-kHugeX + 1024.0f, 0.0f);
        SkPaint redMultiply;
        redMultiply.setAntiAlias(true);
        redMultiply.setColor(SK_ColorRED);
        redMultiply.setBlendMode(SkBlendMode::kMultiply);
        canvas->drawPath(path, redMultiply);
        canvas->restore();

        // Second dst-reading draw expands the pass's dstReadBounds union to x = 1500.
        SkPaint greenMultiply;
        greenMultiply.setColor(SK_ColorGREEN);
        greenMultiply.setBlendMode(SkBlendMode::kMultiply);
        canvas->drawRect(SkRect::MakeLTRB(1400, 0, 1500, kTargetH), greenMultiply);

        // Non-dst-reading draw outside the first draw's tight scissor must not be clipped by it.
        SkPaint yellowSrcOver;
        yellowSrcOver.setColor(SK_ColorYELLOW);
        canvas->drawRect(SkRect::MakeLTRB(1200, 100, 1300, 200), yellowSrcOver);

        std::unique_ptr<Recording> multiRec = recorder->snap();
        REPORTER_ASSERT(reporter, multiRec);
        REPORTER_ASSERT(reporter, context->insertRecording({multiRec.get()}));
        REPORTER_ASSERT(reporter, targetSurface->readPixels(readback, 0, 0));

        REPORTER_ASSERT(reporter, readback.getColor(800, 256) == SK_ColorRED);
        // Region [1026, 1200) must remain white (no red overhang from the first draw).
        REPORTER_ASSERT(reporter, readback.getColor(1100, 256) == SK_ColorWHITE);
        REPORTER_ASSERT(reporter, readback.getColor(1250, 150) == SK_ColorYELLOW);
        REPORTER_ASSERT(reporter, readback.getColor(1450, 256) == SK_ColorGREEN);
    }

    // Step 4: Verify exact in-bounds texel mapping at all corners of a non-power-of-two DstCopy
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
