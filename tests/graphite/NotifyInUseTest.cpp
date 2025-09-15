/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "tests/Test.h"

using namespace skgpu::graphite;

DEF_GRAPHITE_TEST_FOR_CONTEXTS(NotifyInUseTest, /*context_filter=*/nullptr, reporter, context,
                               testContext, CtsEnforcement::kApiLevel_202404) {
    if (context->backend() != skgpu::BackendApi::kDawn) {
        return;
    }

    auto recorder = context->makeRecorder();
    SkImageInfo info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> scratchSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* scratchCanvas = scratchSurface->getCanvas();
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    scratchCanvas->drawRect(SkRect::Make(info.bounds()), greenPaint);

    sk_sp<SkImage> scratchImage = SkSurfaces::AsImage(scratchSurface);
    REPORTER_ASSERT(reporter, scratchImage);

    sk_sp<SkSurface> mainSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* mainCanvas = mainSurface->getCanvas();
    mainCanvas->clear(SK_ColorBLACK);

    mainCanvas->drawImage(scratchImage, 0, 0);

    sk_sp<SkSurface> finalSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* finalCanvas = finalSurface->getCanvas();
    finalCanvas->clear(SK_ColorBLACK);

    // Force a Device::makeImageCopy by calling makeImageSnapshot(). If mainSurface's dependency on
    // scratchSurface was not recorded with notifyImageInUse, the canvas will be black.
    finalCanvas->drawImage(mainSurface->makeImageSnapshot(), 0, 0);

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    REPORTER_ASSERT(reporter, finalSurface->readPixels(bitmap, 0, 0));

    SkColor topLeft = bitmap.getColor(0, 0);
    REPORTER_ASSERT(reporter, topLeft == SK_ColorGREEN,
                    "Expected green pixel from scratch surface, got 0x%08X", topLeft);
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(NotifyInUseTestAsImageBlend,
                               nullptr, reporter,
                               context, testContext, CtsEnforcement::kApiLevel_202404) {
    if (context->backend() != skgpu::BackendApi::kDawn) {
        return;
    }

    auto recorder = context->makeRecorder();
    SkImageInfo info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> scratchSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* scratchCanvas = scratchSurface->getCanvas();
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    scratchCanvas->drawRect(SkRect::Make(info.bounds()), greenPaint);

    sk_sp<SkImage> scratchImage = SkSurfaces::AsImage(scratchSurface);
    REPORTER_ASSERT(reporter, scratchImage);

    sk_sp<SkSurface> mainSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* mainCanvas = mainSurface->getCanvas();
    mainCanvas->clear(SK_ColorTRANSPARENT);

    // Apply a blend to the draw image rect, which forces a flush mid-way through draw geometry.
    // This can mess up the ordering of drawtasks if notify_in_use was called iniside keyhelpers.
    SkPaint blendPaint;
    blendPaint.setBlendMode(SkBlendMode::kHue);
    mainCanvas->drawImageRect(scratchImage, SkRect::MakeWH(10, 10), SkSamplingOptions(),
                              &blendPaint);

    sk_sp<SkSurface> finalSurface = SkSurfaces::RenderTarget(recorder.get(), info);
    SkCanvas* finalCanvas = finalSurface->getCanvas();
    finalCanvas->clear(SK_ColorBLACK);

    finalCanvas->drawImage(SkSurfaces::AsImage(mainSurface), 0, 0);

    SkBitmap bitmap;
    bitmap.allocPixels(info);
    REPORTER_ASSERT(reporter, finalCanvas->readPixels(bitmap, 0, 0));

    SkColor topLeft = bitmap.getColor(0, 0);
    REPORTER_ASSERT(reporter, topLeft == SK_ColorGREEN,
                    "Expected green pixel from scratch surface, got 0x%08X", topLeft);
}
