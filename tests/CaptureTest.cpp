/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "tests/Test.h"

#if defined(SK_GRAPHITE)
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/capture/SkCapture.h"

#include <memory>

namespace {
void enable_capture(skgpu::graphite::ContextOptions* options) {
    options->fEnableCapture = true;
}
}  // namespace

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(SkCaptureCanvas_LazilyReattachAfterSnapPicture,
                                           /* filter= */ nullptr,
                                           reporter,
                                           context,
                                           /* test_ctx= */,
                                           /* options_name= */,
                                           enable_capture,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder);

    SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), info);
    REPORTER_ASSERT(reporter, surface);

    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);

    context->startCapture();

    // Draw something to trigger pollCapturingStatus() and attach a recording canvas.
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeWH(50, 50), paint);

    // Snapping the recording and inserting it should capture the recorded draw and reset
    // fCapturing to false without immediately re-attaching a new recording canvas.
    std::unique_ptr<skgpu::graphite::Recording> recording1 = recorder->snap();
    REPORTER_ASSERT(reporter, recording1);
    REPORTER_ASSERT(reporter, context->insertRecording({recording1.get()}));

    // A subsequent snap with no intervening draws should not capture an empty SkPicture asset.
    std::unique_ptr<skgpu::graphite::Recording> recording2 = recorder->snap();
    REPORTER_ASSERT(reporter, recording2);
    REPORTER_ASSERT(reporter, context->insertRecording({recording2.get()}));

    // Ending capture without any new draws should not add an empty picture asset via
    // captureUninsertedDrawTasks().
    sk_sp<SkCapture> capture = context->endCapture();
    REPORTER_ASSERT(reporter, capture != nullptr);
    REPORTER_ASSERT(reporter, capture->getMetadata().numAssets == 1);
    REPORTER_ASSERT(reporter, capture->getMetadata().numRecordingCaptures == 2);
    REPORTER_ASSERT(reporter, capture->getAsset(0) != nullptr);
    REPORTER_ASSERT(reporter, capture->getAsset(0)->approximateOpCount() > 0);

    // Start a new capture and verify that a draw after snapping lazily re-attaches a
    // recording canvas.
    context->startCapture();
    canvas->drawRect(SkRect::MakeWH(25, 25), paint);
    std::unique_ptr<skgpu::graphite::Recording> recording3 = recorder->snap();
    REPORTER_ASSERT(reporter, recording3);
    REPORTER_ASSERT(reporter, context->insertRecording({recording3.get()}));

    // Draw again after snapping -> should re-attach and be captured on endCapture().
    paint.setColor(SK_ColorBLUE);
    canvas->drawRect(SkRect::MakeXYWH(25, 25, 50, 50), paint);

    capture = context->endCapture();
    REPORTER_ASSERT(reporter, capture != nullptr);
    REPORTER_ASSERT(reporter, capture->getMetadata().numAssets == 2);
    REPORTER_ASSERT(reporter, capture->getMetadata().numRecordingCaptures == 1);
    REPORTER_ASSERT(reporter, capture->getAsset(0) != nullptr);
    REPORTER_ASSERT(reporter, capture->getAsset(0)->approximateOpCount() > 0);
    REPORTER_ASSERT(reporter, capture->getAsset(1) != nullptr);
    REPORTER_ASSERT(reporter, capture->getAsset(1)->approximateOpCount() > 0);
}
#endif  // defined(SK_GRAPHITE)
