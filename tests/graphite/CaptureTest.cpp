/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/capture/SkCapture.h"
#include "src/utils/SkOSPath.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/TestOptions.h"

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

constexpr int kWidth = 100;
constexpr int kHeight = 100;

// Workflow 1:
// - RecorderAB draws image A fully, then image B fully.
// - Call makeImageSnapshot(A) and makeImageSnapshot(B).
// - RecorderC calls drawImage(imgA) and drawImage(imgB).
// - Snap recordings, end capture, and write to capture_1.capt.
void run_workflow_1(skiatest::Reporter* reporter, Context* context) {
    context->startCapture();

    const SkImageInfo infoA =
            SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    const SkImageInfo infoB =
            SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    std::unique_ptr<Recorder> recorderAB = context->makeRecorder();
    sk_sp<SkSurface> surfaceA = SkSurfaces::RenderTarget(recorderAB.get(), infoA);
    sk_sp<SkSurface> surfaceB = SkSurfaces::RenderTarget(recorderAB.get(), infoB);

    REPORTER_ASSERT(reporter, surfaceA != nullptr);
    REPORTER_ASSERT(reporter, surfaceB != nullptr);

    SkCanvas* canvasA = surfaceA->getCanvas();
    SkCanvas* canvasB = surfaceB->getCanvas();

    // 1. Draw image A fully
    canvasA->clear(SK_ColorRED);
    SkPaint paintA;
    paintA.setColor(SK_ColorBLUE);
    canvasA->drawRect(SkRect::MakeXYWH(10, 10, 80, 80), paintA);

    // 2. Draw image B fully
    canvasB->clear(SK_ColorGREEN);
    SkPaint paintB;
    paintB.setColor(SK_ColorYELLOW);
    canvasB->drawCircle(50, 50, 40, paintB);

    // 3. makeImageSnapshot(A) and makeImageSnapshot(B)
    sk_sp<SkImage> imgA = surfaceA->makeImageSnapshot();
    sk_sp<SkImage> imgB = surfaceB->makeImageSnapshot();

    REPORTER_ASSERT(reporter, imgA != nullptr);
    REPORTER_ASSERT(reporter, imgB != nullptr);

    // 4. RecorderC will drawImage(ImgA) and then drawImage(ImgB)
    std::unique_ptr<Recorder> recorderC = context->makeRecorder();
    const SkImageInfo infoC =
            SkImageInfo::Make(kWidth * 2, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surfaceC = SkSurfaces::RenderTarget(recorderC.get(), infoC);
    REPORTER_ASSERT(reporter, surfaceC != nullptr);

    SkCanvas* canvasC = surfaceC->getCanvas();
    canvasC->clear(SK_ColorWHITE);
    canvasC->drawImage(imgA, 0, 0);
    canvasC->drawImage(imgB, kWidth, 0);

    // 5. Snap GPU recordings and insert
    std::unique_ptr<Recording> recordingAB = recorderAB->snap();
    std::unique_ptr<Recording> recordingC = recorderC->snap();

    REPORTER_ASSERT(reporter, recordingAB != nullptr);
    REPORTER_ASSERT(reporter, recordingC != nullptr);

    InsertRecordingInfo insertInfoAB;
    insertInfoAB.fRecording = recordingAB.get();
    REPORTER_ASSERT(reporter, context->insertRecording(insertInfoAB));

    InsertRecordingInfo insertInfoC;
    insertInfoC.fRecording = recordingC.get();
    REPORTER_ASSERT(reporter, context->insertRecording(insertInfoC));

    // End capture
    sk_sp<SkCapture> capture = context->endCapture();
    REPORTER_ASSERT(reporter, capture != nullptr);

    // Note: Currently SkCaptureManager records pictures at breakpoints (surfaceA & surfaceB)
    // plus all open canvases when endCapture() is called (surfaceA, surfaceB, and surfaceC).
    // Total pictures = 5 in current implementation.
    SkCapture::Metadata meta = capture->getMetadata();
    REPORTER_ASSERT(reporter, meta.numAssets >= 3);

    // Serialize capture
    sk_sp<SkData> data = capture->serializeCapture();
    REPORTER_ASSERT(reporter, data != nullptr && data->size() > 0);

    // Write to capture_1.capt
    SkString tmpDir = skiatest::GetTmpDir();
    SkString filename = tmpDir.isEmpty() ? SkString("capture_1.capt")
                                         : SkOSPath::Join(tmpDir.c_str(), "capture_1.capt");

    SkFILEWStream stream(filename.c_str());
    REPORTER_ASSERT(reporter, stream.isValid());
    REPORTER_ASSERT(reporter, stream.write(data->data(), data->size()));
    stream.fsync();

    // Also write to current working directory
    SkFILEWStream localStream("capture_1.capt");
    if (localStream.isValid()) {
        localStream.write(data->data(), data->size());
        localStream.fsync();
    }

    // Verify deserialization from file data
    sk_sp<SkData> fileData = SkData::MakeFromFileName(filename.c_str());
    REPORTER_ASSERT(reporter, fileData != nullptr);
    sk_sp<SkCapture> deserializedCapture = SkCapture::MakeFromData(fileData);
    REPORTER_ASSERT(reporter, deserializedCapture != nullptr);
    REPORTER_ASSERT(reporter, deserializedCapture->getMetadata().numAssets == meta.numAssets);
}

// Workflow 2:
// - RecorderAB draws some of image A, then some of image B, then finishes A, then finishes B.
// - Call makeImageSnapshot(A) and makeImageSnapshot(B).
// - RecorderC calls drawImage(imgA) and drawImage(imgB).
// - Snap recordings, end capture, and write to capture_2.capt.
void run_workflow_2(skiatest::Reporter* reporter, Context* context) {
    context->startCapture();

    const SkImageInfo infoA =
            SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    const SkImageInfo infoB =
            SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    std::unique_ptr<Recorder> recorderAB = context->makeRecorder();
    sk_sp<SkSurface> surfaceA = SkSurfaces::RenderTarget(recorderAB.get(), infoA);
    sk_sp<SkSurface> surfaceB = SkSurfaces::RenderTarget(recorderAB.get(), infoB);

    REPORTER_ASSERT(reporter, surfaceA != nullptr);
    REPORTER_ASSERT(reporter, surfaceB != nullptr);

    SkCanvas* canvasA = surfaceA->getCanvas();
    SkCanvas* canvasB = surfaceB->getCanvas();

    // 1. Draw some of image A
    canvasA->clear(SK_ColorRED);

    // 2. Draw some of image B
    canvasB->clear(SK_ColorGREEN);

    // 3. Finish image A
    SkPaint paintA;
    paintA.setColor(SK_ColorBLUE);
    canvasA->drawRect(SkRect::MakeXYWH(10, 10, 80, 80), paintA);

    // 4. Finish image B
    SkPaint paintB;
    paintB.setColor(SK_ColorYELLOW);
    canvasB->drawCircle(50, 50, 40, paintB);

    // 5. makeImageSnapshot(A) and makeImageSnapshot(B)
    sk_sp<SkImage> imgA = surfaceA->makeImageSnapshot();
    sk_sp<SkImage> imgB = surfaceB->makeImageSnapshot();

    REPORTER_ASSERT(reporter, imgA != nullptr);
    REPORTER_ASSERT(reporter, imgB != nullptr);

    // 6. RecorderC will drawImage(ImgA) and then drawImage(ImgB)
    std::unique_ptr<Recorder> recorderC = context->makeRecorder();
    const SkImageInfo infoC =
            SkImageInfo::Make(kWidth * 2, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surfaceC = SkSurfaces::RenderTarget(recorderC.get(), infoC);
    REPORTER_ASSERT(reporter, surfaceC != nullptr);

    SkCanvas* canvasC = surfaceC->getCanvas();
    canvasC->clear(SK_ColorWHITE);
    canvasC->drawImage(imgA, 0, 0);
    canvasC->drawImage(imgB, kWidth, 0);

    // 7. Snap GPU recordings and insert
    std::unique_ptr<Recording> recordingAB = recorderAB->snap();
    std::unique_ptr<Recording> recordingC = recorderC->snap();

    REPORTER_ASSERT(reporter, recordingAB != nullptr);
    REPORTER_ASSERT(reporter, recordingC != nullptr);

    InsertRecordingInfo insertInfoAB;
    insertInfoAB.fRecording = recordingAB.get();
    REPORTER_ASSERT(reporter, context->insertRecording(insertInfoAB));

    InsertRecordingInfo insertInfoC;
    insertInfoC.fRecording = recordingC.get();
    REPORTER_ASSERT(reporter, context->insertRecording(insertInfoC));

    // End capture
    sk_sp<SkCapture> capture = context->endCapture();
    REPORTER_ASSERT(reporter, capture != nullptr);

    // Note: Currently SkCaptureManager records pictures at breakpoints (surfaceA & surfaceB)
    // plus all open canvases when endCapture() is called (surfaceA, surfaceB, and surfaceC).
    // Total pictures = 5 in current implementation.
    SkCapture::Metadata meta = capture->getMetadata();
    REPORTER_ASSERT(reporter, meta.numAssets >= 3);

    // Serialize capture
    sk_sp<SkData> data = capture->serializeCapture();
    REPORTER_ASSERT(reporter, data != nullptr && data->size() > 0);

    // Write to capture_2.capt
    SkString tmpDir = skiatest::GetTmpDir();
    SkString filename = tmpDir.isEmpty() ? SkString("capture_2.capt")
                                         : SkOSPath::Join(tmpDir.c_str(), "capture_2.capt");

    SkFILEWStream stream(filename.c_str());
    REPORTER_ASSERT(reporter, stream.isValid());
    REPORTER_ASSERT(reporter, stream.write(data->data(), data->size()));
    stream.fsync();

    // Also write to current working directory
    SkFILEWStream localStream("capture_2.capt");
    if (localStream.isValid()) {
        localStream.write(data->data(), data->size());
        localStream.fsync();
    }

    // Verify deserialization from file data
    sk_sp<SkData> fileData = SkData::MakeFromFileName(filename.c_str());
    REPORTER_ASSERT(reporter, fileData != nullptr);
    sk_sp<SkCapture> deserializedCapture = SkCapture::MakeFromData(fileData);
    REPORTER_ASSERT(reporter, deserializedCapture != nullptr);
    REPORTER_ASSERT(reporter, deserializedCapture->getMetadata().numAssets == meta.numAssets);
}

}  // anonymous namespace

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(SkCaptureWorkflowsTest_Workflow1,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           origContext,
                                           origTestContext,
                                           origOptions,
                                           /* opt_filter= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    TestOptions options(origOptions);
    options.fContextOptions.fEnableCapture = true;

    ContextFactory factory(options);
    ContextInfo ctxInfo = factory.getContextInfo(origTestContext->contextType());
    if (!ctxInfo.fContext) {
        return;
    }

    run_workflow_1(reporter, ctxInfo.fContext);
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(SkCaptureWorkflowsTest_Workflow2,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           origContext,
                                           origTestContext,
                                           origOptions,
                                           /* opt_filter= */ nullptr,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    TestOptions options(origOptions);
    options.fContextOptions.fEnableCapture = true;

    ContextFactory factory(options);
    ContextInfo ctxInfo = factory.getContextInfo(origTestContext->contextType());
    if (!ctxInfo.fContext) {
        return;
    }

    run_workflow_2(reporter, ctxInfo.fContext);
}
