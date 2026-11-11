/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

#include <chrono>
#include <cstring>
#include <memory>

using namespace skgpu::graphite;
using Mipmapped = skgpu::Mipmapped;

namespace {

struct ReadPixelsContext {
    SkBitmap* fBitmap = nullptr;
    bool fCalled = false;
    bool fSuccess = false;
};

void async_read_callback(SkImage::ReadPixelsContext ctx,
                         std::unique_ptr<const SkImage::AsyncReadResult> result) {
    auto* readCtx = static_cast<ReadPixelsContext*>(ctx);
    readCtx->fCalled = true;
    if (!result || result->count() != 1 || !result->data(0)) {
        return;
    }
    const SkImageInfo& info = readCtx->fBitmap->info();
    const size_t dstRowBytes = readCtx->fBitmap->rowBytes();
    const size_t srcRowBytes = result->rowBytes(0);
    const size_t copyBytes = info.minRowBytes();
    const auto* src = static_cast<const char*>(result->data(0));
    auto* dst = static_cast<char*>(readCtx->fBitmap->getPixels());
    for (int y = 0; y < info.height(); ++y) {
        std::memcpy(dst + y * dstRowBytes, src + y * srcRowBytes, copyBytes);
    }
    readCtx->fSuccess = true;
}

template <typename Src>
bool read_pixels(Context* context, const Src* src, SkBitmap* outBitmap) {
    const SkImageInfo info = src->imageInfo().makeColorType(kRGBA_8888_SkColorType);
    outBitmap->allocPixels(info);
    ReadPixelsContext readCtx{outBitmap};
    context->asyncRescaleAndReadPixels(src,
                                       info,
                                       SkIRect::MakeSize(info.dimensions()),
                                       SkImage::RescaleGamma::kSrc,
                                       SkImage::RescaleMode::kNearest,
                                       async_read_callback,
                                       &readCtx);
    if (!context->submit(SyncToCpu::kYes)) {
        return false;
    }
    return readCtx.fCalled && readCtx.fSuccess;
}

}  // namespace

// Tests to make sure the managing of back pointers between Recorder and Device all work properly.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecorderDevicePtrTest, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    SkImageInfo info = SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    // Add multiple devices to later test different patterns of destruction.
    sk_sp<Device> device1 = Device::Make(recorder.get(),
                                         info,
                                         skgpu::Budgeted::kYes,
                                         Mipmapped::kNo,
                                         SkBackingFit::kExact,
                                         SkSurfaceProps(),
                                         LoadOp::kClear,
                                         "RecorderTestTexture");
    sk_sp<Device> device2 = Device::Make(recorder.get(),
                                         info,
                                         skgpu::Budgeted::kYes,
                                         Mipmapped::kNo,
                                         SkBackingFit::kExact,
                                         SkSurfaceProps(),
                                         LoadOp::kClear,
                                         "RecorderTestTexture");
    sk_sp<Device> device3 = Device::Make(recorder.get(),
                                         info,
                                         skgpu::Budgeted::kYes,
                                         Mipmapped::kNo,
                                         SkBackingFit::kExact,
                                         SkSurfaceProps(),
                                         LoadOp::kClear,
                                         "RecorderTestTexture");
    sk_sp<Device> device4 = Device::Make(recorder.get(),
                                         info,
                                         skgpu::Budgeted::kYes,
                                         Mipmapped::kNo,
                                         SkBackingFit::kExact,
                                         SkSurfaceProps(),
                                         LoadOp::kClear,
                                         "RecorderTestTexture");
    REPORTER_ASSERT(reporter, device1->recorder() == recorder.get());
    REPORTER_ASSERT(reporter, device2->recorder() == recorder.get());
    REPORTER_ASSERT(reporter, device3->recorder() == recorder.get());
    REPORTER_ASSERT(reporter, device4->recorder() == recorder.get());
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device1.get()));
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device2.get()));
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device3.get()));
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device4.get()));

    // Test freeing a device in the middle, marking it as immutable as ~Surface() our FilterResult
    // would when done with the device.
    device2->setImmutable();
    REPORTER_ASSERT(reporter, device2->recorder() == nullptr);
    REPORTER_ASSERT(reporter, device2->unique()); // Only the test holds a ref now
    REPORTER_ASSERT(reporter, !recorder->priv().deviceIsRegistered(device2.get()));
    device2.reset();

    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device1.get()));
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device3.get()));
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device4.get()));

    // Test freeing a device that wasn't marked as immutable, which should have its ref dropped
    // automatically when the recorder flushes.
    Device* dev4Ptr = device4.get();
    device4.reset();
    REPORTER_ASSERT(reporter, dev4Ptr->unique()); // The recorder holds a ref still
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(dev4Ptr));
    recorder->priv().flushTrackedDevices(
            SK_DUMP_TASKS_CODE("RecorderTest")); // should delete device4 now
    REPORTER_ASSERT(reporter, !recorder->priv().deviceIsRegistered(dev4Ptr));

    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device1.get()));
    REPORTER_ASSERT(reporter, recorder->priv().deviceIsRegistered(device3.get()));

    // Delete the recorder and make sure remaining devices no longer have a valid recorder.
    recorder.reset();
    REPORTER_ASSERT(reporter, device1->recorder() == nullptr);
    REPORTER_ASSERT(reporter, device3->recorder() == nullptr);

    // Make sure freeing Devices after recorder doesn't cause any crash. This would get checked
    // naturually when these devices go out of scope, but manually reseting will give us a better
    // stack trace if something does go wrong.
    device1.reset();
    device3.reset();
}

// Tests to make sure the Recorders can override the ordering requirement.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecorderOrderingTest, reporter, context,
                                   CtsEnforcement::kNever) {
    std::unique_ptr<Recorder> orderedRecorder, unorderedRecorder;

    if (context->priv().caps()->requireOrderedRecordings()) {
        orderedRecorder = context->makeRecorder();
        RecorderOptions opts;
        opts.fRequireOrderedRecordings = false;
        unorderedRecorder = context->makeRecorder(opts);
    } else {
        RecorderOptions opts;
        opts.fRequireOrderedRecordings = true;
        orderedRecorder = context->makeRecorder(opts);
        unorderedRecorder = context->makeRecorder();
    }

    auto insert = [context](Recording* r) {
        InsertRecordingInfo info;
        info.fRecording = r;
        return context->insertRecording(info);
    };

    std::unique_ptr<Recording> o1 = orderedRecorder->snap();
    std::unique_ptr<Recording> o2 = orderedRecorder->snap();
    std::unique_ptr<Recording> o3 = orderedRecorder->snap();

    std::unique_ptr<Recording> u1 = unorderedRecorder->snap();
    std::unique_ptr<Recording> u2 = unorderedRecorder->snap();

    // Unordered insertion of an unordered Recorder succeeds.
    // NOTE: These Recordings are all out-of-order with respect to orderedRecorder, which had been
    // snapped multiple times before unorderedRecorder. That is always allowed.
    REPORTER_ASSERT(reporter, insert(u2.get()));
    REPORTER_ASSERT(reporter, insert(u1.get()));

    // Unordered insertion of an ordered Recorder fails
    REPORTER_ASSERT(reporter, insert(o1.get())); // succeeds (first insertion)
    REPORTER_ASSERT(reporter, !insert(o3.get())); // fails for out of order
    REPORTER_ASSERT(reporter, insert(o2.get())); // succeeds and recovers
    REPORTER_ASSERT(reporter, insert(o3.get())); // now in order success
}

// Test to make sure inserting a null Recording (e.g. Recorder::snap() failed and the client didn't
// check it) returns the correct invalid status.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(NullRecordingInsertTest, reporter, context,
                                   CtsEnforcement::kNever) {
    struct FinishProcContext {
        skiatest::Reporter* fReporter;
        bool fFinishProcCalled = false;
    } finishProcContext{reporter};

    InsertRecordingInfo info;
    info.fRecording = nullptr; // Set to null to simulate a Recorder::snap() failure.
    info.fFinishedContext = &finishProcContext;
    info.fFinishedProc = [](void* ctx, skgpu::CallbackResult result) {
        FinishProcContext* finishProcContext = static_cast<FinishProcContext*>(ctx);
        REPORTER_ASSERT(finishProcContext->fReporter, result == skgpu::CallbackResult::kFailed);
        finishProcContext->fFinishProcCalled = true;
    };

    InsertStatus status = context->insertRecording(info); // Should not crash on the null Recording!
    REPORTER_ASSERT(reporter, status == InsertStatus::kInvalidRecording);
    REPORTER_ASSERT(reporter, finishProcContext.fFinishProcCalled);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(InsertRecordingPendingCountsTest, reporter, context,
                                   CtsEnforcement::kNever) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // 1. Inserting an empty recording first should return zero pending commands and passes.
    std::unique_ptr<Recording> emptyRecording1 = recorder->snap();
    REPORTER_ASSERT(reporter, emptyRecording1);

    InsertStatus status0 = context->insertRecording({emptyRecording1.get()});
    REPORTER_ASSERT(reporter, status0 == InsertStatus::kSuccess);
    REPORTER_ASSERT(reporter, status0.numPendingCommands() == 0);
    REPORTER_ASSERT(reporter, status0.numPendingPasses() == 0);

    // 2. Create a non-empty recording by drawing to a surface.
    SkImageInfo info = SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface);
    surface->getCanvas()->clear(SkColors::kRed);

    std::unique_ptr<Recording> nonEmptyRecording = recorder->snap();
    REPORTER_ASSERT(reporter, nonEmptyRecording);

    InsertStatus status1 = context->insertRecording({nonEmptyRecording.get()});
    REPORTER_ASSERT(reporter, status1 == InsertStatus::kSuccess);
    REPORTER_ASSERT(reporter, status1.numPendingCommands() > 0);
    REPORTER_ASSERT(reporter, status1.numPendingPasses() > 0);

    int pendingCommands = status1.numPendingCommands();
    int pendingPasses = status1.numPendingPasses();

    // 3. Inserting an empty recording afterwards should not change the pending commands/passes.
    std::unique_ptr<Recording> emptyRecording2 = recorder->snap();
    REPORTER_ASSERT(reporter, emptyRecording2);

    InsertStatus status2 = context->insertRecording({emptyRecording2.get()});
    REPORTER_ASSERT(reporter, status2 == InsertStatus::kSuccess);
    REPORTER_ASSERT(reporter, status2.numPendingCommands() == pendingCommands);
    REPORTER_ASSERT(reporter, status2.numPendingPasses() == pendingPasses);
}

// Regression test for b/584701908 (Case 1): Repeatedly wrapping a BackendTexture in an SkSurface
// and destroying the surface on an unsnapped Recorder (as Chromium DrDC does for replay target
// surfaces) must immediately remove the Device from Recorder::fTrackedDevices in O(1) without
// accumulating nullptr tombstones.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecorderTrackedDevicesWrapBackendTextureNoSnapTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);

    const Caps* caps = context->priv().caps();
    TextureInfo texInfo = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                             Mipmapped::kNo,
                                                             skgpu::Protected::kNo,
                                                             skgpu::Renderable::kYes);
    REPORTER_ASSERT(reporter, texInfo.isValid());

    BackendTexture backendTex = recorder->createBackendTexture({16, 16}, texInfo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    static constexpr int kIterations = 2000;
    for (int i = 0; i < kIterations; ++i) {
        sk_sp<SkSurface> surface = SkSurfaces::WrapBackendTexture(recorder.get(),
                                                                  backendTex,
                                                                  kRGBA_8888_SkColorType,
                                                                  /*cs=*/nullptr,
                                                                  /*props=*/nullptr);
        REPORTER_ASSERT(reporter, surface);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);

        // Destroy the surface without calling recorder->snap().
        surface.reset();
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);
    }

    recorder->deleteBackendTexture(backendTex);
}

// Regression test for b/584701908 (Case 2): Destroying RenderTarget and wrapped surfaces (both
// without and with recorded draws) before Recorder::snap() must flush pending work to the Recorder
// and immediately shrink trackedDevicesCount() to 0, while preserving all flushed draws on replay.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(RecorderTrackedDevicesUnsnappedSurfaceDrawsPreservedTest,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    // Protected RenderTarget textures cannot be read back, and !allowCpuSync()
    // (--neverYieldToWebGPU) cannot synchronously populate stack ReadPixelsContext.
    if (context->supportsProtectedContent() || !context->priv().caps()->allowCpuSync()) {
        return;
    }

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const SkImageInfo info =
            SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    // 1. RenderTarget surfaces without explicit draws: trackedDevicesCount() returns to 0 before
    // snap().
    {
        sk_sp<SkSurface> s0 = SkSurfaces::RenderTarget(recorder.get(), info);
        sk_sp<SkSurface> s1 = SkSurfaces::RenderTarget(recorder.get(), info);
        REPORTER_ASSERT(reporter, s0 && s1);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 2);
    }
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);

    // 2. RenderTarget and WrapBackendTexture surfaces with draws destroyed before snap():
    const Caps* caps = context->priv().caps();
    TextureInfo texInfo = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                             Mipmapped::kNo,
                                                             skgpu::Protected::kNo,
                                                             skgpu::Renderable::kYes);
    BackendTexture backendTex = recorder->createBackendTexture({16, 16}, texInfo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    sk_sp<SkImage> rtSnapshot;
    {
        sk_sp<SkSurface> rtSurface = SkSurfaces::RenderTarget(recorder.get(), info);
        sk_sp<SkSurface> wrappedSurface = SkSurfaces::WrapBackendTexture(recorder.get(),
                                                                         backendTex,
                                                                         kRGBA_8888_SkColorType,
                                                                         /*cs=*/nullptr,
                                                                         /*props=*/nullptr);
        REPORTER_ASSERT(reporter, rtSurface && wrappedSurface);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 2);

        SkPaint redPaint;
        redPaint.setColor(SK_ColorRED);
        rtSurface->getCanvas()->clear(SK_ColorGREEN);
        rtSurface->getCanvas()->drawRect(SkRect::MakeXYWH(8, 0, 8, 16), redPaint);
        // makeImageSnapshot() copies to a standalone texture without linking rtSurface's Device,
        // so rtSurface's Device will be fully deregistered and destroyed when rtSurface resets.
        rtSnapshot = rtSurface->makeImageSnapshot();
        REPORTER_ASSERT(reporter, rtSnapshot);

        SkPaint yellowPaint;
        yellowPaint.setColor(SK_ColorYELLOW);
        wrappedSurface->getCanvas()->clear(SK_ColorBLUE);
        wrappedSurface->getCanvas()->drawRect(SkRect::MakeXYWH(8, 0, 8, 16), yellowPaint);
    }

    // Both surfaces were destroyed before snap(); ~Surface -> setImmutable() must have flushed
    // their pending draws and removed both Devices immediately.
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);

    std::unique_ptr<Recording> recording = recorder->snap();
    REPORTER_ASSERT(reporter, recording != nullptr);
    InsertStatus status = context->insertRecording({recording.get()});
    REPORTER_ASSERT(reporter, status == InsertStatus::kSuccess);
    REPORTER_ASSERT(reporter, status.numPendingPasses() > 0);

    SkBitmap rtBitmap;
    REPORTER_ASSERT(reporter, read_pixels(context, rtSnapshot.get(), &rtBitmap));
    REPORTER_ASSERT(reporter, rtBitmap.getColor(2, 2) == SK_ColorGREEN);
    REPORTER_ASSERT(reporter, rtBitmap.getColor(12, 2) == SK_ColorRED);

    sk_sp<SkImage> wrappedImg = SkImages::WrapTexture(recorder.get(),
                                                      backendTex,
                                                      kPremul_SkAlphaType,
                                                      /*colorSpace=*/nullptr);
    REPORTER_ASSERT(reporter, wrappedImg);
    SkBitmap wrappedBitmap;
    REPORTER_ASSERT(reporter, read_pixels(context, wrappedImg.get(), &wrappedBitmap));
    REPORTER_ASSERT(reporter, wrappedBitmap.getColor(2, 2) == SK_ColorBLUE);
    REPORTER_ASSERT(reporter, wrappedBitmap.getColor(12, 2) == SK_ColorYELLOW);

    wrappedImg.reset();
    context->deleteBackendTexture(backendTex);
}

// Regression test for b/584701908 (Case 3): Out-of-order / non-LIFO destruction of multiple
// simultaneously tracked surfaces (exercising removeShuffle across middle, head, and shuffled tail
// elements interleaved with new allocations) preserves exact trackedDevicesCount() and rendering
// integrity on all surviving surfaces.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(RecorderTrackedDevicesNonLifoDestructionTest,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    // Protected RenderTarget textures cannot be read back, and !allowCpuSync()
    // (--neverYieldToWebGPU) cannot synchronously populate stack ReadPixelsContext.
    if (context->supportsProtectedContent() || !context->priv().caps()->allowCpuSync()) {
        return;
    }

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const SkImageInfo info =
            SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    static constexpr SkColor kColors[6] = {
            SK_ColorRED,
            SK_ColorGREEN,
            SK_ColorBLUE,
            SK_ColorYELLOW,
            SK_ColorCYAN,
            SK_ColorMAGENTA,
    };

    sk_sp<SkSurface> surfaces[6];
    for (int i = 0; i < 6; ++i) {
        surfaces[i] = SkSurfaces::RenderTarget(recorder.get(), info);
        REPORTER_ASSERT(reporter, surfaces[i]);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == i + 1);
        surfaces[i]->getCanvas()->clear(kColors[i]);
    }

    // Destroy middle S2: removeShuffle(2) moves S5 into slot 2 -> [S0, S1, S5, S3, S4].
    surfaces[2].reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 5);

    // Destroy first S0: removeShuffle(0) moves S4 into slot 0 -> [S4, S1, S5, S3].
    surfaces[0].reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);

    // Destroy S5 (which was shuffled from tail slot 5 into slot 2!) -> [S4, S1, S3].
    surfaces[5].reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 3);

    // Interleave allocation of S6 -> [S4, S1, S3, S6].
    static constexpr SkColor kSurface6Color = 0xFFFF8000;
    sk_sp<SkSurface> surface6 = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface6);
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);
    surface6->getCanvas()->clear(kSurface6Color);

    // Record additional post-shuffle draws onto all surviving surfaces (S1, S3, S4, S6).
    SkPaint whiteCornerPaint;
    whiteCornerPaint.setColor(SK_ColorWHITE);
    for (SkSurface* live : {surfaces[1].get(), surfaces[3].get(), surfaces[4].get(),
                            surface6.get()}) {
        live->getCanvas()->drawRect(SkRect::MakeXYWH(0, 0, 4, 4), whiteCornerPaint);
    }

    std::unique_ptr<Recording> recording = recorder->snap();
    REPORTER_ASSERT(reporter, recording != nullptr);
    // Live client-owned surfaces must remain registered across snap().
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);
    REPORTER_ASSERT(reporter,
                    context->insertRecording({recording.get()}) == InsertStatus::kSuccess);

    struct ExpectedSurface {
        SkSurface* fSurface;
        SkColor fBaseColor;
    };
    const ExpectedSurface expected[] = {
            {surfaces[1].get(), kColors[1]},
            {surfaces[3].get(), kColors[3]},
            {surfaces[4].get(), kColors[4]},
            {surface6.get(), kSurface6Color},
    };
    for (const auto& exp : expected) {
        SkBitmap bm;
        REPORTER_ASSERT(reporter, read_pixels(context, exp.fSurface, &bm));
        REPORTER_ASSERT(reporter, bm.getColor(2, 2) == SK_ColorWHITE);
        REPORTER_ASSERT(reporter, bm.getColor(10, 10) == exp.fBaseColor);
    }

    // Destroy remaining surfaces in arbitrary order and verify trackedDevicesCount() hits 0.
    surfaces[3].reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 3);
    surface6.reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 2);
    surfaces[1].reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);
    surfaces[4].reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);
}

// Regression test for b/584701908 (Case 4): When an SkSurfaces::AsImage view outlives its
// originating SkSurface without snap(), resetting the SkSurface calls setImmutable() ->
// deregisterDevice() while the Device still has refcount > 1 via the linked Image. The Device must
// be removed from fTrackedDevices immediately, ~Device() debug assertions must hold whether the
// Image is drawn or dropped unused, and drawing the Image into a second surface must render
// expected pixels.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(RecorderTrackedDevicesAsImageOutlivesSurfaceTest,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    // Protected RenderTarget images cannot be drawn into unprotected wrapped surfaces or read back,
    // and !allowCpuSync() (--neverYieldToWebGPU) cannot synchronously populate ReadPixelsContext.
    if (context->supportsProtectedContent() || !context->priv().caps()->allowCpuSync()) {
        return;
    }

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const SkImageInfo info =
            SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    // 1. Surface A -> AsImage -> reset Surface A (Device kept alive by imgA), then draw imgA.
    sk_sp<SkSurface> surfaceA = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surfaceA);
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);

    SkPaint magentaPaint;
    magentaPaint.setColor(SK_ColorMAGENTA);
    surfaceA->getCanvas()->clear(SK_ColorCYAN);
    surfaceA->getCanvas()->drawRect(SkRect::MakeXYWH(8, 0, 8, 16), magentaPaint);

    sk_sp<SkImage> imgA = SkSurfaces::AsImage(surfaceA);
    REPORTER_ASSERT(reporter, imgA);

    surfaceA.reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);

    // 2. Also test Surface U -> AsImage -> reset Surface U -> reset imgU WITHOUT ever drawing imgU
    // (so ~Image_Base drops the final Device ref directly without notifyInUse() unlinking it).
    {
        sk_sp<SkSurface> surfaceUnused = SkSurfaces::RenderTarget(recorder.get(), info);
        REPORTER_ASSERT(reporter, surfaceUnused);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);
        surfaceUnused->getCanvas()->clear(SK_ColorGREEN);
        sk_sp<SkImage> imgUnused = SkSurfaces::AsImage(surfaceUnused);
        REPORTER_ASSERT(reporter, imgUnused);
        surfaceUnused.reset();
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);
        imgUnused.reset();
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);
    }

    // 3. Draw imgA into a wrapped destination surface, destroy that surface too before snap(),
    // then snap + replay + verify pixels.
    const Caps* caps = context->priv().caps();
    TextureInfo texInfo = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                             Mipmapped::kNo,
                                                             skgpu::Protected::kNo,
                                                             skgpu::Renderable::kYes);
    BackendTexture dstTex = recorder->createBackendTexture({16, 16}, texInfo);
    REPORTER_ASSERT(reporter, dstTex.isValid());

    {
        sk_sp<SkSurface> surfaceB = SkSurfaces::WrapBackendTexture(recorder.get(),
                                                                   dstTex,
                                                                   kRGBA_8888_SkColorType,
                                                                   /*cs=*/nullptr,
                                                                   /*props=*/nullptr);
        REPORTER_ASSERT(reporter, surfaceB);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);
        surfaceB->getCanvas()->clear(SK_ColorBLACK);
        surfaceB->getCanvas()->drawImage(imgA, 0, 0);
        imgA.reset();
    }
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);

    std::unique_ptr<Recording> recording = recorder->snap();
    REPORTER_ASSERT(reporter, recording != nullptr);
    REPORTER_ASSERT(reporter,
                    context->insertRecording({recording.get()}) == InsertStatus::kSuccess);

    sk_sp<SkImage> dstImg = SkImages::WrapTexture(recorder.get(),
                                                  dstTex,
                                                  kPremul_SkAlphaType,
                                                  /*colorSpace=*/nullptr);
    REPORTER_ASSERT(reporter, dstImg);
    SkBitmap bm;
    REPORTER_ASSERT(reporter, read_pixels(context, dstImg.get(), &bm));
    REPORTER_ASSERT(reporter, bm.getColor(2, 2) == SK_ColorCYAN);
    REPORTER_ASSERT(reporter, bm.getColor(12, 2) == SK_ColorMAGENTA);

    dstImg.reset();
    context->deleteBackendTexture(dstTex);
}

// Regression test for b/584701908 (Case 5): Re-entrant deregisterDevice() during active
// flushTrackedDevices() (both a depth-2 cross-surface dependency flush chain where mipmap scratch
// surfaces hit the unregistered deregisterDevice() scan while depth > 0, and registered-Device
// destruction while fFlushingTrackedDevicesDepth > 0 leaving nullptr tombstones) must never corrupt
// iteration, and subsequent freeGpuResources(), performDeferredCleanup(), and snap() calls must
// compact tombstones once fFlushingTrackedDevicesDepth returns to 0.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(RecorderTrackedDevicesReentrantFlushAndCompactionTest,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    // Protected RenderTarget textures cannot be read back, and !allowCpuSync()
    // (--neverYieldToWebGPU) cannot synchronously populate stack ReadPixelsContext.
    if (context->supportsProtectedContent() || !context->priv().caps()->allowCpuSync()) {
        return;
    }

    // Part A: Genuine depth-2 cross-surface dependency flush chain (C holds a pending read of
    // mipmapped B; B holds both a pending write and a pending read of A; plus sibling D). Mutating
    // and destroying A triggers flushTrackedDevices(proxyA) [depth=1] -> B flushes C via
    // flushTrackedDevices(proxyB) [depth=2] (preserving C == YELLOW) and generates mipmaps whose
    // scratch Surface::MakeScratch (registerWithRecorder=false) hits the not-found deregisterDevice
    // scan while depth > 0, before A deregisters and shrinks fTrackedDevices once depth == 0.
    {
        std::unique_ptr<Recorder> recorder = context->makeRecorder();
        const SkImageInfo info =
                SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        sk_sp<SkSurface> surfaceA = SkSurfaces::RenderTarget(recorder.get(), info, Mipmapped::kNo);
        surfaceA->getCanvas()->clear(SK_ColorBLUE);
        sk_sp<SkImage> imgA = SkSurfaces::AsImage(surfaceA);

        sk_sp<SkSurface> surfaceB = SkSurfaces::RenderTarget(recorder.get(), info, Mipmapped::kYes);
        surfaceB->getCanvas()->clear(SK_ColorYELLOW);
        sk_sp<SkImage> imgB = SkSurfaces::AsImage(surfaceB);

        sk_sp<SkSurface> surfaceC = SkSurfaces::RenderTarget(recorder.get(), info, Mipmapped::kYes);
        surfaceC->getCanvas()->clear(SK_ColorBLACK);
        // Flushes B's YELLOW clear via notifyInUse; C now holds a pending read of proxyB.
        surfaceC->getCanvas()->drawImage(imgB, 0, 0);

        // Flushes A's BLUE clear via notifyInUse; B now holds a pending read of proxyA and a
        // pending write to proxyB.
        surfaceB->getCanvas()->drawImage(imgA, 0, 0);

        sk_sp<SkSurface> surfaceD = SkSurfaces::RenderTarget(recorder.get(), info, Mipmapped::kNo);
        surfaceD->getCanvas()->clear(SK_ColorGREEN);

        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);
        REPORTER_ASSERT(reporter, recorder->priv().flushingTrackedDevicesDepth() == 0);

        // Mutate A and drop imgA/imgB + surfaceA: ~Surface(A) flushes dependent B [depth=1], which
        // first flushes dependent C [depth=2] before overwriting B with BLUE, then A deregisters at
        // depth=0.
        imgA.reset();
        imgB.reset();
        surfaceA->getCanvas()->clear(SK_ColorRED);
        surfaceA.reset();
        REPORTER_ASSERT(reporter, recorder->priv().flushingTrackedDevicesDepth() == 0);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 3);

        std::unique_ptr<Recording> recording = recorder->snap();
        REPORTER_ASSERT(reporter, recording != nullptr);
        REPORTER_ASSERT(reporter, recorder->priv().flushingTrackedDevicesDepth() == 0);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 3);
        REPORTER_ASSERT(reporter,
                        context->insertRecording({recording.get()}) == InsertStatus::kSuccess);

        SkBitmap bmB, bmC, bmD;
        REPORTER_ASSERT(reporter, read_pixels(context, surfaceB.get(), &bmB));
        REPORTER_ASSERT(reporter, read_pixels(context, surfaceC.get(), &bmC));
        REPORTER_ASSERT(reporter, read_pixels(context, surfaceD.get(), &bmD));
        REPORTER_ASSERT(reporter, bmB.getColor(4, 4) == SK_ColorBLUE);
        REPORTER_ASSERT(reporter, bmC.getColor(4, 4) == SK_ColorYELLOW);
        REPORTER_ASSERT(reporter, bmD.getColor(4, 4) == SK_ColorGREEN);
    }

    // Part B: Registered-Device destruction while fFlushingTrackedDevicesDepth > 0 leaves nullptr
    // tombstones (preserving indices during active flush, and resisting premature compaction during
    // nested cleanup calls), then compacts cleanly in freeGpuResources(), performDeferredCleanup(),
    // and snap() once depth == 0.
    {
        std::unique_ptr<Recorder> recorder = context->makeRecorder();
        const SkImageInfo info =
                SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        sk_sp<SkSurface> s0 = SkSurfaces::RenderTarget(recorder.get(), info);
        sk_sp<SkSurface> s1 = SkSurfaces::RenderTarget(recorder.get(), info);
        sk_sp<SkSurface> s2 = SkSurfaces::RenderTarget(recorder.get(), info);
        sk_sp<SkSurface> s3 = SkSurfaces::RenderTarget(recorder.get(), info);
        s0->getCanvas()->clear(SK_ColorRED);
        s1->getCanvas()->clear(SK_ColorGREEN);
        s2->getCanvas()->clear(SK_ColorBLUE);
        s3->getCanvas()->clear(SK_ColorYELLOW);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);
        REPORTER_ASSERT(reporter, recorder->priv().flushingTrackedDevicesDepth() == 0);

        {
            RecorderPriv::TrackedDevicesFlushScope outerFlush(recorder.get());
            REPORTER_ASSERT(reporter, recorder->priv().flushingTrackedDevicesDepth() == 1);
            s1.reset();
            // Must leave a nullptr tombstone rather than shrinking while depth == 1.
            REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);

            {
                RecorderPriv::TrackedDevicesFlushScope nestedFlush(recorder.get());
                REPORTER_ASSERT(reporter, recorder->priv().flushingTrackedDevicesDepth() == 2);
                s2.reset();
                REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);
            }
            REPORTER_ASSERT(reporter, recorder->priv().flushingTrackedDevicesDepth() == 1);
            // Still at depth == 1; cleanup methods must NOT compact while flushing is active.
            recorder->freeGpuResources();
            REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);
            recorder->performDeferredCleanup(std::chrono::milliseconds(0));
            REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);

            // Dependency flush must safely skip nullptr tombstones without crashing.
            recorder->priv().flushTrackedDevices(static_cast<const TextureProxy*>(nullptr));
            REPORTER_ASSERT(reporter, recorder->priv().flushingTrackedDevicesDepth() == 1);
            REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);
        }

        // Depth is now 0, with 2 live surfaces (s0, s3) and 2 nullptr tombstones.
        REPORTER_ASSERT(reporter, recorder->priv().flushingTrackedDevicesDepth() == 0);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);

        // 1. freeGpuResources() compacts nullptr tombstones down to exact live count (2).
        recorder->freeGpuResources();
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 2);

        // Create another tombstone at depth > 0 and verify performDeferredCleanup() compacts it.
        sk_sp<SkSurface> s4 = SkSurfaces::RenderTarget(recorder.get(), info);
        s4->getCanvas()->clear(SK_ColorCYAN);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 3);
        {
            RecorderPriv::TrackedDevicesFlushScope flushScope(recorder.get());
            s0.reset();
            REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 3);
        }
        recorder->performDeferredCleanup(std::chrono::milliseconds(0));
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 2);

        // Create one more tombstone at depth > 0 and verify snap() compacts it.
        {
            RecorderPriv::TrackedDevicesFlushScope flushScope(recorder.get());
            s3.reset();
            REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 2);
        }
        std::unique_ptr<Recording> recording = recorder->snap();
        REPORTER_ASSERT(reporter, recording != nullptr);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);
        REPORTER_ASSERT(reporter,
                        context->insertRecording({recording.get()}) == InsertStatus::kSuccess);

        SkBitmap bm4;
        REPORTER_ASSERT(reporter, read_pixels(context, s4.get(), &bm4));
        REPORTER_ASSERT(reporter, bm4.getColor(8, 8) == SK_ColorCYAN);

        s4.reset();
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);
    }
}

// Regression test for b/584701908 (Case 6): Calling freeGpuResources() and
// performDeferredCleanup() while some Surfaces and uniquely-held unflushed Devices are still live
// and other Surfaces were destroyed must compact ONLY nullptr tombstones, preserving all live
// Surfaces and unflushed unique Devices until snap().
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(RecorderTrackedDevicesCleanupCompactionSafetyTest,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    // Protected RenderTarget textures cannot be read back, and !allowCpuSync()
    // (--neverYieldToWebGPU) cannot synchronously populate stack ReadPixelsContext.
    if (context->supportsProtectedContent() || !context->priv().caps()->allowCpuSync()) {
        return;
    }

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const SkImageInfo info =
            SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> s0 = SkSurfaces::RenderTarget(recorder.get(), info);
    sk_sp<SkSurface> s1 = SkSurfaces::RenderTarget(recorder.get(), info);
    sk_sp<SkSurface> s2 = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, s0 && s1 && s2);
    s0->getCanvas()->clear(SK_ColorRED);
    s1->getCanvas()->clear(SK_ColorGREEN);
    s2->getCanvas()->clear(SK_ColorBLUE);
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 3);

    // Also create a raw registered Device wrapping a BackendTexture, record an unflushed clear on
    // it, and drop our local sk_sp<Device> WITHOUT setImmutable() so Recorder::fTrackedDevices is
    // the sole owner (device->unique() == true, unflushed!). Neither freeGpuResources() nor
    // performDeferredCleanup() may drop this Device prematurely before snap() flushes it!
    const Caps* caps = context->priv().caps();
    TextureInfo texInfo = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                             Mipmapped::kNo,
                                                             skgpu::Protected::kNo,
                                                             skgpu::Renderable::kYes);
    BackendTexture rawTex = recorder->createBackendTexture({16, 16}, texInfo);
    REPORTER_ASSERT(reporter, rawTex.isValid());
    {
        sk_sp<Texture> wrappedTex =
                recorder->priv().resourceProvider()->createWrappedTexture(rawTex, "RawDevTex");
        REPORTER_ASSERT(reporter, wrappedTex);
        sk_sp<TextureProxy> rawProxy = TextureProxy::Wrap(std::move(wrappedTex));
        sk_sp<Device> rawDevice = Device::Make(recorder.get(),
                                               std::move(rawProxy),
                                               {16, 16},
                                               info.colorInfo(),
                                               SkSurfaceProps(),
                                               LoadOp::kLoad);
        REPORTER_ASSERT(reporter, rawDevice);
        SkCanvas rawCanvas(rawDevice);
        rawCanvas.clear(SK_ColorMAGENTA);
    }
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);

    // Destroy s1 normally (depth == 0 -> immediate removal, count == 3).
    s1.reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 3);

    // Destroy s2 during an active flush scope to leave a nullptr tombstone (count stays 3:
    // s0, rawDevice, nullptr).
    {
        RecorderPriv::TrackedDevicesFlushScope flushScope(recorder.get());
        s2.reset();
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 3);
    }

    // Both cleanup APIs must compact the nullptr tombstone while keeping BOTH s0 AND the uniquely
    // held unflushed rawDevice alive (count == 2).
    recorder->freeGpuResources();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 2);
    recorder->performDeferredCleanup(std::chrono::milliseconds(0));
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 2);

    // Record post-cleanup draw onto s0.
    SkPaint yellowPaint;
    yellowPaint.setColor(SK_ColorYELLOW);
    s0->getCanvas()->drawRect(SkRect::MakeXYWH(0, 0, 8, 8), yellowPaint);

    // snap() flushes both s0 and rawDevice, and then drops uniquely-held rawDevice (leaving s0: 1).
    std::unique_ptr<Recording> recording = recorder->snap();
    REPORTER_ASSERT(reporter, recording != nullptr);
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);
    REPORTER_ASSERT(reporter,
                    context->insertRecording({recording.get()}) == InsertStatus::kSuccess);

    SkBitmap bm0;
    REPORTER_ASSERT(reporter, read_pixels(context, s0.get(), &bm0));
    REPORTER_ASSERT(reporter, bm0.getColor(2, 2) == SK_ColorYELLOW);
    REPORTER_ASSERT(reporter, bm0.getColor(12, 12) == SK_ColorRED);

    sk_sp<SkImage> rawImg = SkImages::WrapTexture(recorder.get(),
                                                  rawTex,
                                                  kPremul_SkAlphaType,
                                                  /*colorSpace=*/nullptr);
    REPORTER_ASSERT(reporter, rawImg);
    SkBitmap bmRaw;
    REPORTER_ASSERT(reporter, read_pixels(context, rawImg.get(), &bmRaw));
    REPORTER_ASSERT(reporter, bmRaw.getColor(8, 8) == SK_ColorMAGENTA);

    rawImg.reset();
    s0.reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);
    context->deleteBackendTexture(rawTex);
}

// Regression test for b/584701908 (Case 7): Destroying the Recorder while live SkSurfaces (with
// unflushed draws and open saveLayers) and AsImage views still exist must cleanly abandon all live
// tracked Devices so subsequent inspection and destruction of the remaining Surfaces and Images
// neither crashes nor triggers ~Device() debug assertions.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecorderDestroyedBeforeLiveSurfacesTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const SkImageInfo info =
            SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surfaces[6];
    for (int i = 0; i < 6; ++i) {
        surfaces[i] = SkSurfaces::RenderTarget(recorder.get(), info);
        REPORTER_ASSERT(reporter, surfaces[i]);
        surfaces[i]->getCanvas()->clear(SK_ColorBLUE);
    }
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 6);

    // Leave an open saveLayer on surfaces[5] (adds a child Device to fTrackedDevices) and hold an
    // AsImage view on surfaces[1].
    surfaces[5]->getCanvas()->saveLayer(nullptr, nullptr);
    surfaces[5]->getCanvas()->clear(SK_ColorGREEN);
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 7);

    sk_sp<SkImage> img1 = SkSurfaces::AsImage(surfaces[1]);
    REPORTER_ASSERT(reporter, img1);

    // Destroy half the surfaces before destroying the Recorder (including leaving one mid-flush
    // nullptr tombstone to ensure ~Recorder() handles mixed null and live entries cleanly).
    surfaces[0].reset();
    surfaces[2].reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 5);
    {
        RecorderPriv::TrackedDevicesFlushScope flushScope(recorder.get());
        surfaces[4].reset();
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 5);
    }

    // Destroy the Recorder FIRST while surfaces[1], surfaces[3], surfaces[5] (plus its saveLayer
    // child Device), and img1 are still alive.
    recorder.reset();

    for (int idx : {1, 3, 5}) {
        REPORTER_ASSERT(reporter, surfaces[idx]->recorder() == nullptr);
        REPORTER_ASSERT(reporter, surfaces[idx]->getCanvas()->baseRecorder() == nullptr);
    }

    // Using img1 on a second Recorder must cleanly unlink the abandoned Device without crashing.
    std::unique_ptr<Recorder> recorder2 = context->makeRecorder();
    sk_sp<SkSurface> dstOnRec2 = SkSurfaces::RenderTarget(recorder2.get(), info);
    REPORTER_ASSERT(reporter, dstOnRec2);
    dstOnRec2->getCanvas()->drawImage(img1, 0, 0);
    std::unique_ptr<Recording> rec2 = recorder2->snap();
    REPORTER_ASSERT(reporter, rec2 != nullptr);

    // Destroying the remaining Surfaces (including surfaces[5] with its open saveLayer) and Images
    // after Recorder destruction must not crash or fire ~Device() asserts.
    surfaces[1].reset();
    surfaces[3].reset();
    surfaces[5].reset();
    img1.reset();
}

// Regression test for b/584701908 (Case 8a): Recorder::makeDeferredCanvas() registers a surface-
// less Device that Recorder::snap() marks immutable via fTargetProxyDevice->setImmutable() before
// flushing, shrinking trackedDevicesCount() to 0; tearing down a second unsnapped deferred canvas
// via ~Recorder() also cleans up without crashing or asserting.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecorderTrackedDevicesDeferredCanvasTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const SkImageInfo info =
            SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    const TextureInfo texInfo = context->priv().caps()->getDefaultSampledTextureInfo(
            kRGBA_8888_SkColorType,
            Mipmapped::kNo,
            recorder->priv().isProtected(),
            skgpu::Renderable::kYes);
    REPORTER_ASSERT(reporter, texInfo.isValid());

    SkCanvas* canvas1 = recorder->makeDeferredCanvas(info, texInfo);
    REPORTER_ASSERT(reporter, canvas1 != nullptr);
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);
    canvas1->clear(SK_ColorRED);

    std::unique_ptr<Recording> recording = recorder->snap();
    REPORTER_ASSERT(reporter, recording != nullptr);
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);

    // Second deferred canvas torn down without snap().
    SkCanvas* canvas2 = recorder->makeDeferredCanvas(info, texInfo);
    REPORTER_ASSERT(reporter, canvas2 != nullptr);
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);
    canvas2->clear(SK_ColorBLUE);
    recorder.reset();
}

// Regression test for b/584701908 (Case 8b): SkCanvas::saveLayer() / restore() marks each temporary
// layer Device immutable in internalRestore(), immediately removing it from fTrackedDevices on an
// unsnapped Recorder without accumulating tombstones.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecorderTrackedDevicesSaveLayerRestoreTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const SkImageInfo info =
            SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface);
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);

    SkCanvas* canvas = surface->getCanvas();
    canvas->saveLayer(nullptr, nullptr);
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 2);
    canvas->saveLayer(nullptr, nullptr);
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 3);
    canvas->clear(SK_ColorGREEN);
    canvas->restore();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 2);
    canvas->restore();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);

    // Keep iteration count <= 8 to stay below an unrelated >16 unflushed-draw SkBlockAllocator
    // debug alignment assert when operator new is 8-byte aligned.
    static constexpr int kLoopIterations = 8;
    for (int i = 0; i < kLoopIterations; ++i) {
        canvas->saveLayer(nullptr, nullptr);
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 2);
        canvas->clear(SK_ColorBLUE);
        canvas->restore();
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);
    }

    surface.reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);
}

// Regression test for b/584701908 (Case 8c): Deregistering Devices at depth == 0 while nullptr
// tombstones already exist in fTrackedDevices (including a tombstone at the tail slot that
// removeShuffle swaps into an earlier slot) shrinks fTrackedDevices cleanly and compacts remaining
// tombstones on cleanup.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecorderTrackedDevicesDepthZeroWithTombstonesTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    const SkImageInfo info =
            SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> s[5];
    for (int i = 0; i < 5; ++i) {
        s[i] = SkSurfaces::RenderTarget(recorder.get(), info);
        REPORTER_ASSERT(reporter, s[i]);
    }
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 5);

    // Leave tombstones at slot 1 and tail slot 4: [s0, nullptr, s2, s3, nullptr].
    {
        RecorderPriv::TrackedDevicesFlushScope flushScope(recorder.get());
        s[1].reset();
        s[4].reset();
        REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 5);
    }

    // At depth == 0, resetting s[0] shuffles the tail nullptr from slot 4 into slot 0 -> size 4;
    // resetting s[3] removes the new live tail -> size 3 ([nullptr, nullptr, s2]).
    s[0].reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);
    s[3].reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 3);

    sk_sp<SkSurface> s5 = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, s5);
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 4);

    recorder->performDeferredCleanup(std::chrono::milliseconds(0));
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 2);

    s[2].reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 1);
    s5.reset();
    REPORTER_ASSERT(reporter, recorder->priv().trackedDevicesCount() == 0);
}
