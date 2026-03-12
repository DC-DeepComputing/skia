/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "tests/Test.h"

using namespace skgpu::graphite;

namespace {

struct FinishContext {
    skiatest::Reporter* fReporter;
    bool fCalled = false;
};

void FinishProc(GpuFinishedContext ctx, skgpu::CallbackResult result) {
    FinishContext* fc = static_cast<FinishContext*>(ctx);
    REPORTER_ASSERT(fc->fReporter, result == skgpu::CallbackResult::kSuccess);
    fc->fCalled = true;
}

}  // anonymous namespace

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(InsertFinishProc_PendingCommandBuffer, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    // Ensure the Context is fully idle initially.
    context->submit(SyncToCpu::kYes);

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    if (!surface) {
        ERRORF(reporter, "Failed to create surface");
        return;
    }
    surface->getCanvas()->clear(SkColors::kRed);

    // Add work to the pending command buffer (inserts it into QueueManager).
    auto recording = recorder->snap();
    InsertRecordingInfo info;
    info.fRecording = recording.get();
    context->insertRecording(info);

    FinishContext finishContext;
    finishContext.fReporter = reporter;

    // Scenario 1: There is a pending command buffer, so the proc gets attached to it.
    context->insertFinishProc(FinishProc, &finishContext);

    // Should not be called yet since we haven't submitted the pending command buffer.
    REPORTER_ASSERT(reporter, !finishContext.fCalled);

    // Submitting and syncing should trigger the callback.
    context->submit(SyncToCpu::kYes);
    REPORTER_ASSERT(reporter, finishContext.fCalled);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(InsertFinishProc_OutstandingGPUWork, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    // Ensure the Context is fully idle initially.
    context->submit(SyncToCpu::kYes);

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    if (!surface) {
        ERRORF(reporter, "Failed to create surface");
        return;
    }
    surface->getCanvas()->clear(SkColors::kBlue);

    auto recording = recorder->snap();
    InsertRecordingInfo info;
    info.fRecording = recording.get();
    context->insertRecording(info);

    // Submit the work without waiting, moving it to outstanding GPU work.
    context->submit(SyncToCpu::kNo);

    FinishContext finishContext;
    finishContext.fReporter = reporter;

    // Scenario 2: There is outstanding work but no pending command buffer.
    // The proc should be attached to the outstanding submission.
    context->insertFinishProc(FinishProc, &finishContext);

    // Depending on the backend (e.g., Vulkan/Dawn/Metal) and timing, it might be called
    // immediately if the work already finished in the background, but we can at least
    // submit with sync to guarantee it gets called.
    context->submit(SyncToCpu::kYes);
    REPORTER_ASSERT(reporter, finishContext.fCalled);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(InsertFinishProc_NoPendingNoOutstanding, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    // Ensure the Context is fully idle.
    context->submit(SyncToCpu::kYes);

    FinishContext finishContext;
    finishContext.fReporter = reporter;

    // Scenario 3: No pending nor outstanding work -> immediate call.
    context->insertFinishProc(FinishProc, &finishContext);

    REPORTER_ASSERT(reporter, finishContext.fCalled);
}
