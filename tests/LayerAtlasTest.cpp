/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "tools/graphite/ContextFactory.h"

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(LayerAtlas_GraphiteRenderPassCount, reporter, context,
                                         CtsEnforcement::kApiLevel_202404) {
    using namespace skgpu::graphite;

    sk_sp<SkImageFilter> blur = SkImageFilters::Blur(2.0f, 2.0f, nullptr);
    SkPaint filterPaint;
    filterPaint.setImageFilter(blur);

    SkPaint fillPaint;
    fillPaint.setColor(SK_ColorRED);

    constexpr int kNumLayers = 10;

    // Test with LayerAtlas ENABLED: should produce <= 5 render passes
    {
        std::unique_ptr<Recorder> recorder = context->makeRecorder();
        REPORTER_ASSERT(reporter, recorder != nullptr);

        SkImageInfo info = SkImageInfo::MakeN32Premul(1000, 1000);
        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
        REPORTER_ASSERT(reporter, surface != nullptr);
        SkCanvas* canvas = surface->getCanvas();
        canvas->setLayerAtlasEnabled(true);

        for (int i = 0; i < kNumLayers; ++i) {
            SkRect bounds = SkRect::MakeXYWH(i * 60 + 10, 50, 40, 40);
            canvas->saveLayer(&bounds, &filterPaint);
            canvas->drawRect(bounds, fillPaint);
            canvas->restore();
        }

        canvas->flushLayerAtlas();

        std::unique_ptr<Recording> recording = recorder->snap();
        REPORTER_ASSERT(reporter, recording != nullptr);

        int numPasses = recording->priv().numRenderPasses();
        REPORTER_ASSERT(reporter, numPasses <= 5,
                        "Expected <= 5 render passes with LayerAtlas enabled, got %d", numPasses);

        context->insertRecording({recording.get(), surface.get()});
        context->submit(SyncToCpu::kYes);
    }

    // Test with LayerAtlas DISABLED: should produce >= 2 * kNumLayers render passes
    {
        std::unique_ptr<Recorder> recorder = context->makeRecorder();
        REPORTER_ASSERT(reporter, recorder != nullptr);

        SkImageInfo info = SkImageInfo::MakeN32Premul(1000, 1000);
        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
        REPORTER_ASSERT(reporter, surface != nullptr);
        SkCanvas* canvas = surface->getCanvas();
        canvas->setLayerAtlasEnabled(false);

        for (int i = 0; i < kNumLayers; ++i) {
            SkRect bounds = SkRect::MakeXYWH(i * 60 + 10, 50, 40, 40);
            canvas->saveLayer(&bounds, &filterPaint);
            canvas->drawRect(bounds, fillPaint);
            canvas->restore();
        }

        std::unique_ptr<Recording> recording = recorder->snap();
        REPORTER_ASSERT(reporter, recording != nullptr);

        int numPasses = recording->priv().numRenderPasses();
        REPORTER_ASSERT(reporter, numPasses >= 2 * kNumLayers,
                        "Expected >= %d render passes with LayerAtlas disabled, got %d",
                        2 * kNumLayers, numPasses);

        context->insertRecording({recording.get(), surface.get()});
        context->submit(SyncToCpu::kYes);
    }

    // Test with parent alpha folding: should also produce <= 5 render passes
    {
        std::unique_ptr<Recorder> recorder = context->makeRecorder();
        REPORTER_ASSERT(reporter, recorder != nullptr);

        SkImageInfo info = SkImageInfo::MakeN32Premul(1000, 1000);
        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
        REPORTER_ASSERT(reporter, surface != nullptr);
        SkCanvas* canvas = surface->getCanvas();
        canvas->setLayerAtlasEnabled(true);

        for (int i = 0; i < kNumLayers; ++i) {
            SkRect bounds = SkRect::MakeXYWH(i * 60 + 10, 50, 40, 40);
            canvas->saveLayerAlphaf(&bounds, 0.5f);
            canvas->saveLayer(&bounds, &filterPaint);
            canvas->drawRect(bounds, fillPaint);
            canvas->restore();
            canvas->restore();
        }

        canvas->flushLayerAtlas();

        std::unique_ptr<Recording> recording = recorder->snap();
        REPORTER_ASSERT(reporter, recording != nullptr);

        int numPasses = recording->priv().numRenderPasses();
        SkDebugf("[LayerAtlas_GraphiteParentAlphaFold] Total render passes: %d\n", numPasses);
        REPORTER_ASSERT(reporter, numPasses <= 5,
                        "Expected <= 5 render passes with parent alpha folding, got %d", numPasses);

        context->insertRecording({recording.get(), surface.get()});
        context->submit(SyncToCpu::kYes);
    }
}
#endif

DEF_TEST(LayerAtlas_InterleavedDraw, reporter) {
    SkBitmap bm;
    bm.allocN32Pixels(400, 400);
    SkCanvas canvas(bm);
    canvas.setLayerAtlasEnabled(true);
    canvas.clear(SK_ColorWHITE);

    sk_sp<SkImageFilter> blur = SkImageFilters::Blur(2.0f, 2.0f, nullptr);
    SkPaint filterPaint;
    filterPaint.setImageFilter(blur);

    SkPaint redPaint;
    redPaint.setColor(SK_ColorRED);

    SkPaint bluePaint;
    bluePaint.setColor(SK_ColorBLUE);

    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);

    // First layer
    SkRect r1 = SkRect::MakeXYWH(10, 10, 50, 50);
    canvas.saveLayer(&r1, &filterPaint);
    canvas.drawRect(r1, redPaint);
    canvas.restore();

    // Interleaved non-layer draw: triggers flush of pending atlas layers
    SkRect rMid = SkRect::MakeXYWH(100, 100, 50, 50);
    canvas.drawRect(rMid, bluePaint);

    // Second layer
    SkRect r2 = SkRect::MakeXYWH(200, 10, 50, 50);
    canvas.saveLayer(&r2, &filterPaint);
    canvas.drawRect(r2, greenPaint);
    canvas.restore();

    canvas.flushLayerAtlas();

    // Verify drawing completed without error
    REPORTER_ASSERT(reporter, bm.getColor(125, 125) == SK_ColorBLUE);
}

DEF_TEST(LayerAtlas_FilterMismatch, reporter) {
    SkBitmap bm;
    bm.allocN32Pixels(400, 400);
    SkCanvas canvas(bm);
    canvas.setLayerAtlasEnabled(true);
    canvas.clear(SK_ColorWHITE);

    sk_sp<SkImageFilter> blur1 = SkImageFilters::Blur(2.0f, 2.0f, nullptr);
    sk_sp<SkImageFilter> blur2 = SkImageFilters::Blur(5.0f, 5.0f, nullptr);

    SkPaint filterPaint1;
    filterPaint1.setImageFilter(blur1);

    SkPaint filterPaint2;
    filterPaint2.setImageFilter(blur2);

    SkPaint redPaint;
    redPaint.setColor(SK_ColorRED);

    // Layer 1 with blur1
    SkRect r1 = SkRect::MakeXYWH(10, 10, 50, 50);
    canvas.saveLayer(&r1, &filterPaint1);
    canvas.drawRect(r1, redPaint);
    canvas.restore();

    // Layer 2 with blur2 (mismatch -> flushes layer 1 and starts new atlas)
    SkRect r2 = SkRect::MakeXYWH(100, 10, 50, 50);
    canvas.saveLayer(&r2, &filterPaint2);
    canvas.drawRect(r2, redPaint);
    canvas.restore();

    canvas.flushLayerAtlas();
}

DEF_TEST(LayerAtlas_OverlappingBounds, reporter) {
    SkBitmap bm;
    bm.allocN32Pixels(400, 400);
    SkCanvas canvas(bm);
    canvas.setLayerAtlasEnabled(true);
    canvas.clear(SK_ColorWHITE);

    sk_sp<SkImageFilter> blur = SkImageFilters::Blur(2.0f, 2.0f, nullptr);
    SkPaint filterPaint;
    filterPaint.setImageFilter(blur);

    SkPaint redPaint;
    redPaint.setColor(SK_ColorRED);

    // Layer 1
    SkRect r1 = SkRect::MakeXYWH(50, 50, 50, 50);
    canvas.saveLayer(&r1, &filterPaint);
    canvas.drawRect(r1, redPaint);
    canvas.restore();

    // Layer 2 with overlapping bounds -> flushes layer 1 to preserve Z-order
    SkRect r2 = SkRect::MakeXYWH(60, 60, 50, 50);
    canvas.saveLayer(&r2, &filterPaint);
    canvas.drawRect(r2, redPaint);
    canvas.restore();

    canvas.flushLayerAtlas();
}

DEF_TEST(LayerAtlas_CapacityOverflow, reporter) {
    SkBitmap bm;
    bm.allocN32Pixels(1000, 1000);
    SkCanvas canvas(bm);
    canvas.setLayerAtlasEnabled(true);
    canvas.clear(SK_ColorWHITE);

    sk_sp<SkImageFilter> blur = SkImageFilters::Blur(2.0f, 2.0f, nullptr);
    SkPaint filterPaint;
    filterPaint.setImageFilter(blur);

    SkPaint fillPaint;
    fillPaint.setColor(SK_ColorRED);

    // Draw 40 layers of 200x200 (exceeds single 2048x2048 atlas)
    for (int i = 0; i < 40; ++i) {
        SkRect r = SkRect::MakeXYWH((i % 8) * 100, (i / 8) * 100, 80, 80);
        canvas.saveLayer(&r, &filterPaint);
        canvas.drawRect(r, fillPaint);
        canvas.restore();
    }

    canvas.flushLayerAtlas();
}
