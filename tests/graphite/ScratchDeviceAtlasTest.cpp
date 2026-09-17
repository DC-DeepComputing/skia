/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE) && defined(GPU_TEST_UTILS)

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "tests/CtsEnforcement.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/TestOptions.h"

namespace {

void enable_scratch_device_atlas(skgpu::graphite::ContextOptions* options) {
    options->fUseScratchDeviceAtlas = true;
}

}  // namespace

DEF_GRAPHITE_TEST(ScratchDeviceAtlas_BatchedLayers, reporter, CtsEnforcement::kNever) {
    skiatest::graphite::TestOptions disabledOptions;
    disabledOptions.fContextOptions.fUseScratchDeviceAtlas = false;
    skiatest::graphite::ContextFactory disabledFactory(disabledOptions);

    skiatest::graphite::TestOptions enabledOptions;
    enabledOptions.fContextOptions.fUseScratchDeviceAtlas = true;
    skiatest::graphite::ContextFactory enabledFactory(enabledOptions);

    for (int typeInt = 0; typeInt < skgpu::kContextTypeCount; ++typeInt) {
        skgpu::ContextType contextType = static_cast<skgpu::ContextType>(typeInt);
        if (!skgpu::IsRenderingContext(contextType)) {
            continue;
        }
        auto drawLayers = [&](skgpu::graphite::Context* context) -> int {
            std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
            REPORTER_ASSERT(reporter, recorder != nullptr);

            SkImageInfo info = SkImageInfo::MakeN32Premul(800, 600);
            sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
            REPORTER_ASSERT(reporter, surface != nullptr);
            SkCanvas* canvas = surface->getCanvas();

            for (int i = 0; i < 16; ++i) {
                SkRect bounds = SkRect::MakeXYWH((i % 4) * 100, (i / 4) * 100, 64, 64);
                canvas->saveLayer(bounds, nullptr);
                SkPaint p;
                p.setColor(SkColorSetARGB(255, i * 15, 255 - i * 15, 128));
                canvas->drawRect(bounds, p);
                canvas->restore();
            }

            std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
            REPORTER_ASSERT(reporter, recording != nullptr);
            int numPasses = recording->priv().numRenderPasses();
            context->insertRecording({recording.get(), surface.get()});
            context->submit(skgpu::graphite::SyncToCpu::kYes);
            return numPasses;
        };

        skiatest::graphite::ContextInfo disabledCtx = disabledFactory.getContextInfo(contextType);
        if (disabledCtx.fContext) {
            int passesWithoutAtlas = drawLayers(disabledCtx.fContext);
            INFOF(reporter,
                  "[ScratchDeviceAtlas_BatchedLayers] Render passes without atlas: %d",
                  passesWithoutAtlas);
            REPORTER_ASSERT(reporter, passesWithoutAtlas == 17,
                            "Expected 17 render passes with atlas disabled, got %d",
                            passesWithoutAtlas);
        }

        skiatest::graphite::ContextInfo enabledCtx = enabledFactory.getContextInfo(contextType);
        if (!enabledCtx.fContext) {
            continue;
        }

        // Draw 16 small layers (64x64).
        // Without atlasing, each layer creates a separate render pass (16 + 1 = 17 passes).
        // With ScratchDeviceAtlas, all 16 layers batch into fChildAtlas (2048x2048).
        // Total render passes should be exactly 2 (1 atlas pass + 1 root surface pass).
        int numPasses = drawLayers(enabledCtx.fContext);
        INFOF(reporter, "[ScratchDeviceAtlas_BatchedLayers] Total render passes: %d", numPasses);
        REPORTER_ASSERT(reporter, numPasses <= 2,
                        "Expected <= 2 render passes for 16 batched layers, got %d", numPasses);
    }
}

DEF_GRAPHITE_TEST(ScratchDeviceAtlas_BatchedBlurLayers, reporter, CtsEnforcement::kNever) {
    skiatest::graphite::TestOptions options;
    options.fContextOptions.fUseScratchDeviceAtlas = true;
    skiatest::graphite::ContextFactory factory(options);
    for (int typeInt = 0; typeInt < skgpu::kContextTypeCount; ++typeInt) {
        skgpu::ContextType contextType = static_cast<skgpu::ContextType>(typeInt);
        if (!skgpu::IsRenderingContext(contextType)) {
            continue;
        }
        skiatest::graphite::ContextInfo ctxInfo = factory.getContextInfo(contextType);
        if (!ctxInfo.fContext) {
            continue;
        }

        std::unique_ptr<skgpu::graphite::Recorder> recorder = ctxInfo.fContext->makeRecorder();
        REPORTER_ASSERT(reporter, recorder != nullptr);

        SkImageInfo info = SkImageInfo::MakeN32Premul(800, 600);
        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
        REPORTER_ASSERT(reporter, surface != nullptr);
        SkCanvas* canvas = surface->getCanvas();

        // Draw 8 small layers with blur image filter.
        // Each blurred layer has: child layer -> 2 blur passes -> root draw.
        // Without atlasing: 8 * 3 + 1 = 25 passes.
        // With ScratchDeviceAtlas:
        //   - fChildAtlas (all child layers) -> 1 pass
        //   - fFilterAtlas[0] (all blur pass 1) -> 1 pass
        //   - fFilterAtlas[1] (all blur pass 2) -> 1 pass
        //   - root surface -> 1 pass
        // Total: <= 4 passes!
        sk_sp<SkImageFilter> blur = SkImageFilters::Blur(4.0f, 4.0f, nullptr);
        SkPaint layerPaint;
        layerPaint.setImageFilter(blur);

        for (int i = 0; i < 8; ++i) {
            SkRect bounds = SkRect::MakeXYWH((i % 4) * 100, (i / 4) * 100, 64, 64);
            canvas->saveLayer(bounds, &layerPaint);
            SkPaint p;
            p.setColor(SkColorSetARGB(255, 200, 50, i * 30));
            canvas->drawRect(bounds, p);
            canvas->restore();
        }

        std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
        REPORTER_ASSERT(reporter, recording != nullptr);

        int numPasses = recording->priv().numRenderPasses();
        INFOF(reporter, "[ScratchDeviceAtlas_BatchedBlurLayers] Total render passes: %d",
              numPasses);
        REPORTER_ASSERT(reporter, numPasses <= 4,
                        "Expected <= 4 render passes for 8 batched blurred layers, got %d",
                        numPasses);

        ctxInfo.fContext->insertRecording({recording.get(), surface.get()});
        ctxInfo.fContext->submit(skgpu::graphite::SyncToCpu::kYes);
    }
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(ScratchDeviceAtlas_FilterAtlasOverflow,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           options,
                                           enable_scratch_device_atlas,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder != nullptr);

    SkImageInfo info = SkImageInfo::MakeN32Premul(800, 600);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface != nullptr);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    // Construct a filter where each 64x64 child layer in fChildAtlas produces a ~448x448
    // intermediate texture in fFilterAtlas[0] (via 7x scale + blur), so fFilterAtlas[0]
    // (capacity: 4x4 = 16 slots of 448x448 in 2048x2048) overflows on layer 16 while
    // fChildAtlas (capacity: >900 slots of 64x64) still has room.
    // When fFilterAtlas[0] overflows, it must flush fParentDevice->flushAtlases() in
    // dependency order (fChildAtlas before fFilterAtlas[0]) and reset both atlases so
    // layer 16 and all prior/subsequent layers render non-transparent pixels.
    sk_sp<SkImageFilter> scaleUp = SkImageFilters::MatrixTransform(
            SkMatrix::Scale(7.0f, 7.0f), SkSamplingOptions(SkFilterMode::kLinear), nullptr);
    sk_sp<SkImageFilter> filter = SkImageFilters::Blur(2.0f, 2.0f, std::move(scaleUp));

    constexpr int kNumLayers = 20;
    for (int i = 0; i < kNumLayers; ++i) {
        int col = i % 5;
        int row = i / 5;
        float x = col * 150.0f;
        float y = row * 140.0f;

        canvas->save();
        canvas->translate(x, y);
        SkRect localBounds = SkRect::MakeWH(64, 64);
        SkPaint layerPaint;
        layerPaint.setImageFilter(filter);
        canvas->saveLayer(localBounds, &layerPaint);
        SkPaint fillPaint;
        fillPaint.setColor(SkColorSetARGB(255, 200, 40 + i * 10, 80));
        canvas->drawRect(localBounds, fillPaint);
        canvas->restore();
        canvas->restore();
    }

    SkBitmap bm;
    bm.allocN32Pixels(800, 600);
    REPORTER_ASSERT(reporter, surface->readPixels(bm, 0, 0));

    for (int i = 0; i < kNumLayers; ++i) {
        int col = i % 5;
        int row = i / 5;
        int sampleX = col * 150 + 32;
        int sampleY = row * 140 + 32;
        SkColor c = bm.getColor(sampleX, sampleY);
        REPORTER_ASSERT(reporter, SkColorGetA(c) > 200 && SkColorGetR(c) > 150,
                        "Layer %d at (%d, %d) failed to render after filter atlas overflow: 0x%08x",
                        i, sampleX, sampleY, c);
    }
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(ScratchDeviceAtlas_DeferredClipPath,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           options,
                                           enable_scratch_device_atlas,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder != nullptr);

    SkImageInfo info = SkImageInfo::MakeN32Premul(200, 200);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface != nullptr);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    // Create an atlased saveLayer and apply a non-rectangular clipPath (which records a
    // deferred depth-only clip draw in Graphite's ClipStack that must be flushed before
    // the layer's draws are snapped in the shared atlas DrawContext).
    SkRect layerBounds = SkRect::MakeXYWH(20, 20, 100, 100);
    canvas->saveLayer(layerBounds, nullptr);

    SkPath triangleClip = SkPath::Polygon({{70, 25}, {115, 115}, {25, 115}}, /*isClosed=*/true);
    canvas->clipPath(triangleClip, SkClipOp::kIntersect, /*doAntiAlias=*/false);

    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    canvas->drawRect(layerBounds, greenPaint);
    canvas->restore();

    SkBitmap bm;
    bm.allocN32Pixels(200, 200);
    REPORTER_ASSERT(reporter, surface->readPixels(bm, 0, 0));

    // Inside the triangle clip (70, 80) should be green.
    SkColor insideColor = bm.getColor(70, 80);
    REPORTER_ASSERT(reporter, insideColor == SK_ColorGREEN,
                    "Expected green inside atlased clipPath, got 0x%08x", insideColor);

    // Outside the triangle clip but inside the layer bounds (30, 30) and (110, 30) must remain
    // transparent.
    SkColor outsideLeft = bm.getColor(30, 30);
    SkColor outsideRight = bm.getColor(110, 30);
    REPORTER_ASSERT(reporter, outsideLeft == SK_ColorTRANSPARENT,
                    "Expected transparent outside atlased clipPath at (30, 30), got 0x%08x",
                    outsideLeft);
    REPORTER_ASSERT(reporter, outsideRight == SK_ColorTRANSPARENT,
                    "Expected transparent outside atlased clipPath at (110, 30), got 0x%08x",
                    outsideRight);
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(ScratchDeviceAtlas_FullSlotClearAndDrawPaint,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           options,
                                           enable_scratch_device_atlas,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder != nullptr);

    SkImageInfo info = SkImageInfo::MakeN32Premul(300, 200);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface != nullptr);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    // Layer 1: draw green into the first atlased slot.
    SkRect bounds1 = SkRect::MakeXYWH(10, 10, 64, 64);
    canvas->saveLayer(bounds1, nullptr);
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    canvas->drawRect(bounds1, greenPaint);
    canvas->restore();

    // Layer 2: shares fChildAtlas with Layer 1 and performs a full-slot drawPaint/clear.
    // This must not call fDC->clear() or fDC->discard() on the shared atlas DrawContext,
    // which would wipe Layer 1's pixels.
    SkRect bounds2 = SkRect::MakeXYWH(100, 10, 64, 64);
    canvas->saveLayer(bounds2, nullptr);
    SkPaint redPaint;
    redPaint.setColor(SK_ColorRED);
    canvas->drawPaint(redPaint);
    canvas->restore();

    // Layer 3: shares fChildAtlas and calls canvas->clear(SK_ColorBLUE).
    SkRect bounds3 = SkRect::MakeXYWH(190, 10, 64, 64);
    canvas->saveLayer(bounds3, nullptr);
    canvas->clear(SK_ColorBLUE);
    canvas->restore();

    SkBitmap bm;
    bm.allocN32Pixels(300, 200);
    REPORTER_ASSERT(reporter, surface->readPixels(bm, 0, 0));

    SkColor c1 = bm.getColor(42, 42);
    SkColor c2 = bm.getColor(132, 42);
    SkColor c3 = bm.getColor(222, 42);
    REPORTER_ASSERT(reporter, c1 == SK_ColorGREEN,
                    "Expected Layer 1 to remain green (0x%08x), got 0x%08x",
                    SK_ColorGREEN, c1);
    REPORTER_ASSERT(reporter, c2 == SK_ColorRED,
                    "Expected Layer 2 to be red (0x%08x), got 0x%08x",
                    SK_ColorRED, c2);
    REPORTER_ASSERT(reporter, c3 == SK_ColorBLUE,
                    "Expected Layer 3 to be blue (0x%08x), got 0x%08x",
                    SK_ColorBLUE, c3);
}

#endif // SK_GRAPHITE
