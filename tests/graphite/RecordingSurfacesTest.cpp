/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"

namespace skgpu::graphite {

constexpr SkIVector kNoOffset = SkIVector::Make(0, 0);
constexpr SkIRect kEmptyClip = SkIRect::MakeEmpty();

using DrawCallback = std::function<void(SkCanvas*)>;

struct Expectation {
    int fX;
    int fY;
    SkColor4f fColor;
};

void check_expectations(skiatest::Reporter* reporter, SkSurface* surface, const std::vector<Expectation>& expectations) {
    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(surface->imageInfo());
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    for (const Expectation& e : expectations) {
        SkColor4f color = pixmap.getColor4f(e.fX, e.fY);
#ifdef SK_DEBUG
        if (color != e.fColor) {
            SkDebugf("Wrong color at %d, %d\n\texpected: %f %f %f %f\n\tactual: %f %f %f %f",
                     e.fX,
                     e.fY,
                     e.fColor.fR,
                     e.fColor.fG,
                     e.fColor.fB,
                     e.fColor.fA,
                     color.fR,
                     color.fG,
                     color.fB,
                     color.fA);
        }
#endif
        REPORTER_ASSERT(reporter, color == e.fColor);
    }
}

void run_test(skiatest::Reporter* reporter,
              Context* context,
              SkISize surfaceSize,
              SkISize recordingSize,
              SkIVector replayOffset,
              SkIRect replayClip,
              skgpu::Mipmapped canvasMipmapped,
              skgpu::Mipmapped targetMipmapped,
              DrawCallback draw,
              const std::vector<Expectation>& expectations) {
    const SkImageInfo surfaceImageInfo = SkImageInfo::Make(
            surfaceSize, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);

    std::unique_ptr<Recorder> surfaceRecorder = context->makeRecorder();
    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(surfaceRecorder.get(), surfaceImageInfo, canvasMipmapped);
    Surface* graphiteSurface = static_cast<Surface*>(surface.get());
    const TextureInfo& textureInfo = graphiteSurface->backingTextureProxy()->textureInfo();

    // Flush the initial clear added by MakeGraphite.
    std::unique_ptr<skgpu::graphite::Recording> surfaceRecording = surfaceRecorder->snap();
    context->insertRecording({surfaceRecording.get()});

    // Snap a recording without a bound target.
    const SkImageInfo recordingImageInfo = surfaceImageInfo.makeDimensions(recordingSize);
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkCanvas* canvas = recorder->makeDeferredCanvas(recordingImageInfo, textureInfo);
    draw(canvas);
    std::unique_ptr<Recording> recording = recorder->snap();

    // Play back recording. If the mipmap settings don't match, we have to create a new surface.
    if (canvasMipmapped != targetMipmapped) {
        surface =
                SkSurfaces::RenderTarget(surfaceRecorder.get(), surfaceImageInfo, targetMipmapped);
        // Flush the initial clear for this new surface.
        surfaceRecording = surfaceRecorder->snap();
        context->insertRecording({surfaceRecording.get()});
    }
    context->insertRecording({recording.get(), surface.get(), replayOffset, replayClip});

    check_expectations(reporter, surface.get(), expectations);
}

// Tests that clear does not clear an entire replayed-to surface if recorded onto a smaller surface.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestClear, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    SkISize surfaceSize = SkISize::Make(8, 4);
    SkISize recordingSize = SkISize::Make(4, 4);

    auto draw = [](SkCanvas* canvas) { canvas->clear(SkColors::kRed); };

    std::vector<Expectation> expectations = {{0, 0, SkColors::kRed},
                                             {4, 0, SkColors::kTransparent}};

    run_test(reporter,
             context,
             surfaceSize,
             recordingSize,
             kNoOffset,
             kEmptyClip,
             skgpu::Mipmapped::kNo,
             skgpu::Mipmapped::kNo,
             draw,
             expectations);
}

// Tests that a draw is translated correctly when replayed with an offset.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestDraw, reporter, context,
                                   CtsEnforcement::kApiLevel_202504) {
    SkISize surfaceSize = SkISize::Make(8, 4);
    SkISize recordingSize = SkISize::Make(4, 4);
    SkIVector replayOffset = SkIVector::Make(4, 0);

    auto draw = [](SkCanvas* canvas) {
        canvas->drawIRect(SkIRect::MakeXYWH(0, 0, 4, 4), SkPaint(SkColors::kRed));
    };


    std::vector<Expectation> expectations = {{0, 0, SkColors::kTransparent},
                                             {4, 0, SkColors::kRed}};

    run_test(reporter,
             context,
             surfaceSize,
             recordingSize,
             replayOffset,
             kEmptyClip,
             skgpu::Mipmapped::kNo,
             skgpu::Mipmapped::kNo,
             draw,
             expectations);
}

// Tests that writePixels is translated correctly when replayed with an offset.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestWritePixels, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(4, 4, true);
    SkCanvas bitmapCanvas(bitmap);
    SkPaint paint;
    paint.setColor(SkColors::kRed);
    bitmapCanvas.drawIRect(SkIRect::MakeXYWH(0, 0, 4, 4), paint);

    SkISize surfaceSize = SkISize::Make(8, 4);
    SkISize recordingSize = SkISize::Make(4, 4);
    SkIVector replayOffset = SkIVector::Make(4, 0);

    auto draw = [&bitmap](SkCanvas* canvas) { canvas->writePixels(bitmap, 0, 0); };

    std::vector<Expectation> expectations = {{0, 0, SkColors::kTransparent},
                                             {4, 0, SkColors::kRed}};

    run_test(reporter,
             context,
             surfaceSize,
             recordingSize,
             replayOffset,
             kEmptyClip,
             skgpu::Mipmapped::kNo,
             skgpu::Mipmapped::kNo,
             draw,
             expectations);
}

// Tests that the result of writePixels is cropped correctly when offscreen.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestWritePixelsOffscreen, reporter, context,
                                   CtsEnforcement::kApiLevel_202404) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(4, 4, true);
    SkCanvas bitmapCanvas(bitmap);
    SkPaint paint;
    paint.setColor(SkColors::kRed);
    bitmapCanvas.drawIRect(SkIRect::MakeXYWH(0, 0, 4, 4), paint);
    paint.setColor(SkColors::kGreen);
    bitmapCanvas.drawIRect(SkIRect::MakeXYWH(2, 2, 2, 2), paint);

    SkISize surfaceSize = SkISize::Make(4, 4);
    SkISize recordingSize = SkISize::Make(4, 4);
    SkIVector replayOffset = SkIVector::Make(-2, -2);

    auto draw = [&bitmap](SkCanvas* canvas) { canvas->writePixels(bitmap, 0, 0); };

    std::vector<Expectation> expectations = {{0, 0, SkColors::kGreen}};

    run_test(reporter,
             context,
             surfaceSize,
             recordingSize,
             replayOffset,
             kEmptyClip,
             skgpu::Mipmapped::kNo,
             skgpu::Mipmapped::kNo,
             draw,
             expectations);
}

// Tests that the result of a draw is cropped correctly with a provided clip on replay.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestDrawWithClip, reporter, context,
                                   CtsEnforcement::kApiLevel_202504) {
    SkISize surfaceSize = SkISize::Make(8, 4);
    SkISize recordingSize = SkISize::Make(8, 4);

    auto draw = [](SkCanvas* canvas) {
        canvas->drawIRect(SkIRect::MakeXYWH(0, 0, 4, 4), SkPaint(SkColors::kRed));
        canvas->drawIRect(SkIRect::MakeXYWH(4, 0, 4, 4), SkPaint(SkColors::kGreen));
    };

    {
        // Test that the clip applies in translated space.
        SkIVector replayOffset = SkIVector::Make(4, 0);
        SkIRect replayClip = SkIRect::MakeXYWH(0, 0, 2, 4);

        std::vector<Expectation> expectations = {{0, 0, SkColors::kTransparent},
                                                 {4, 0, SkColors::kRed},
                                                 {6, 0, SkColors::kTransparent}};

        run_test(reporter,
                 context,
                 surfaceSize,
                 recordingSize,
                 replayOffset,
                 replayClip,
                 skgpu::Mipmapped::kNo,
                 skgpu::Mipmapped::kNo,
                 draw,
                 expectations);
    }

    {
        // Test that the draw is not translated by the clip.
        SkIRect replayClip = SkIRect::MakeXYWH(4, 0, 2, 4);

        std::vector<Expectation> expectations = {{4, 0, SkColors::kGreen}};

        run_test(reporter,
                 context,
                 surfaceSize,
                 recordingSize,
                 kNoOffset,
                 replayClip,
                 skgpu::Mipmapped::kNo,
                 skgpu::Mipmapped::kNo,
                 draw,
                 expectations);
    }
}

// Tests that a scissor translated to negative coordinates is applied correctly.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestNegativeClip, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    SkISize surfaceSize = SkISize::Make(4, 4);
    SkISize recordingSize = SkISize::Make(4, 4);

    auto draw = [](SkCanvas* canvas) {
        canvas->clipIRect(SkIRect::MakeWH(4, 4));
        canvas->drawIRect(SkIRect::MakeWH(4, 4), SkPaint(SkColors::kRed));
    };

    SkIVector replayOffset = SkIVector::Make(-2, 0);

    std::vector<Expectation> expectations = {{0, 0, SkColors::kRed},
                                             {2, 0, SkColors::kTransparent}};

    run_test(reporter,
             context,
             surfaceSize,
             recordingSize,
             replayOffset,
             kEmptyClip,
             skgpu::Mipmapped::kNo,
             skgpu::Mipmapped::kNo,
             draw,
             expectations);
}

// Tests that the result of writePixels is cropped correctly with a provided clip on replay.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestWritePixelsWithClip, reporter, context,
                                   CtsEnforcement::kApiLevel_202504) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(4, 4, true);
    SkCanvas bitmapCanvas(bitmap);
    SkPaint paint;
    paint.setColor(SkColors::kRed);
    bitmapCanvas.drawIRect(SkIRect::MakeXYWH(0, 0, 4, 4), paint);

    SkISize surfaceSize = SkISize::Make(8, 4);
    SkISize recordingSize = SkISize::Make(4, 4);
    SkIVector replayOffset = SkIVector::Make(4, 0);
    SkIRect replayClip = SkIRect::MakeXYWH(0, 0, 2, 4);

    auto draw = [&bitmap](SkCanvas* canvas) { canvas->writePixels(bitmap, 0, 0); };

    std::vector<Expectation> expectations = {{0, 0, SkColors::kTransparent},
                                             {4, 0, SkColors::kRed},
                                             {6, 0, SkColors::kTransparent}};

    run_test(reporter,
             context,
             surfaceSize,
             recordingSize,
             replayOffset,
             replayClip,
             skgpu::Mipmapped::kNo,
             skgpu::Mipmapped::kNo,
             draw,
             expectations);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestMipmapped, reporter, context,
                                   CtsEnforcement::kApiLevel_202504) {
    SkISize recordingSize = SkISize::Make(1, 1);

    auto draw = [](SkCanvas* canvas) { canvas->clear(SkColors::kRed); };

    {
        SkISize surfaceSize = SkISize::Make(1, 1);

        std::vector<Expectation> expectations = {{0, 0, SkColors::kRed}};

        run_test(reporter,
                 context,
                 surfaceSize,
                 recordingSize,
                 kNoOffset,
                 kEmptyClip,
                 skgpu::Mipmapped::kYes,
                 skgpu::Mipmapped::kYes,
                 draw,
                 expectations);
    }

    {
        // Test that draw fails if canvas and surface mipmap settings don't match.
        SkISize surfaceSize = SkISize::Make(1, 1);

        std::vector<Expectation> expectations = {{0, 0, SkColors::kTransparent}};

        run_test(reporter,
                 context,
                 surfaceSize,
                 recordingSize,
                 kNoOffset,
                 kEmptyClip,
                 skgpu::Mipmapped::kYes,
                 skgpu::Mipmapped::kNo,
                 draw,
                 expectations);

        run_test(reporter,
                 context,
                 surfaceSize,
                 recordingSize,
                 kNoOffset,
                 kEmptyClip,
                 skgpu::Mipmapped::kNo,
                 skgpu::Mipmapped::kYes,
                 draw,
                 expectations);
    }

    {
        // Test that draw fails if canvas and surface dimensions don't match.
        SkISize surfaceSize = SkISize::Make(2, 1);

        std::vector<Expectation> expectations = {{0, 0, SkColors::kTransparent}};

        run_test(reporter,
                 context,
                 surfaceSize,
                 recordingSize,
                 kNoOffset,
                 kEmptyClip,
                 skgpu::Mipmapped::kYes,
                 skgpu::Mipmapped::kYes,
                 draw,
                 expectations);
    }

    {
        // Test that draw fails if offset is provided.
        SkISize surfaceSize = SkISize::Make(1, 1);
        SkIVector replayOffset = SkIVector::Make(1, 0);

        std::vector<Expectation> expectations = {{0, 0, SkColors::kTransparent}};

        run_test(reporter,
                 context,
                 surfaceSize,
                 recordingSize,
                 replayOffset,
                 kEmptyClip,
                 skgpu::Mipmapped::kYes,
                 skgpu::Mipmapped::kYes,
                 draw,
                 expectations);
    }

    {
        // Test that draw fails if clip is provided.
        SkISize surfaceSize = SkISize::Make(1, 1);
        SkIRect replayClip = SkIRect::MakeXYWH(0, 0, 1, 1);

        std::vector<Expectation> expectations = {{0, 0, SkColors::kTransparent}};

        run_test(reporter,
                 context,
                 surfaceSize,
                 recordingSize,
                 kNoOffset,
                 replayClip,
                 skgpu::Mipmapped::kYes,
                 skgpu::Mipmapped::kYes,
                 draw,
                 expectations);
    }
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestMipmappedWritePixels, reporter, context,
                                   CtsEnforcement::kApiLevel_202504) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(1, 1, true);
    SkCanvas bitmapCanvas(bitmap);
    SkPaint paint;
    paint.setColor(SkColors::kRed);
    bitmapCanvas.drawIRect(SkIRect::MakeXYWH(0, 0, 1, 1), paint);

    SkISize recordingSize = SkISize::Make(1, 1);
    SkISize surfaceSize = SkISize::Make(1, 1);

    auto draw = [&bitmap](SkCanvas* canvas) { canvas->writePixels(bitmap, 0, 0); };

    std::vector<Expectation> expectations = {{0, 0, SkColors::kRed}};

    run_test(reporter,
             context,
             surfaceSize,
             recordingSize,
             kNoOffset,
             kEmptyClip,
             skgpu::Mipmapped::kYes,
             skgpu::Mipmapped::kYes,
             draw,
             expectations);
}

// Tests that you can't create two deferred canvases before snapping the first.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestTwoCanvases, reporter, context,
                                   CtsEnforcement::kApiLevel_202504) {
    const SkImageInfo kImageInfo = SkImageInfo::Make(SkISize::Make(1, 1),
                                                     SkColorType::kRGBA_8888_SkColorType,
                                                     SkAlphaType::kPremul_SkAlphaType);
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(recorder.get(), kImageInfo, skgpu::Mipmapped::kNo);
    const TextureInfo& textureInfo =
            static_cast<Surface*>(surface.get())->backingTextureProxy()->textureInfo();

    // First canvas is created successfully.
    REPORTER_ASSERT(reporter, recorder->makeDeferredCanvas(kImageInfo, textureInfo) != nullptr);
    // Second canvas fails to be created.
    REPORTER_ASSERT(reporter, recorder->makeDeferredCanvas(kImageInfo, textureInfo) == nullptr);
}

// Tests that inserting a recording with a surface does not crash even if no draws to a deferred
// canvas were recorded.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestUnnecessarySurface, reporter, context,
                                   CtsEnforcement::kApiLevel_202504) {
    const SkImageInfo kImageInfo = SkImageInfo::Make(SkISize::Make(1, 1),
                                                     SkColorType::kRGBA_8888_SkColorType,
                                                     SkAlphaType::kPremul_SkAlphaType);
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(recorder.get(), kImageInfo, skgpu::Mipmapped::kNo);
    std::unique_ptr<Recording> recording = recorder->snap();
    REPORTER_ASSERT(reporter, context->insertRecording({recording.get(), surface.get()}));
}


// Tests that a surface created on one recorder is correctly marked as initialized
// after having a recording from another recorder played back onto it.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(RecordingSurfacesTestSurfaceLifecycle, reporter, context,
                                   CtsEnforcement::kApiLevel_202504) {
    const SkImageInfo kImageInfo = SkImageInfo::Make(SkISize::Make(8, 8),
                                                     SkColorType::kRGBA_8888_SkColorType,
                                                     SkAlphaType::kPremul_SkAlphaType);

    // Create a render target surface on recorder A. This surface will be considered uninitialized and
    // have its load op set to kClear so that the surface is cleared when it is first used.
    std::unique_ptr<Recorder> recorderA = context->makeRecorder();
    sk_sp<SkSurface> surfaceA =
            SkSurfaces::RenderTarget(recorderA.get(), kImageInfo, skgpu::Mipmapped::kNo);

    // Draw red to a deferred canvas on recorder B. Match the image and texture info of surface A.
    std::unique_ptr<Recorder> recorderB = context->makeRecorder();
    auto* graphiteSurfaceA = static_cast<Surface*>(surfaceA.get());
    const TextureInfo& textureInfo = graphiteSurfaceA->backingTextureProxy()->textureInfo();
    SkCanvas* canvasB = recorderB->makeDeferredCanvas(kImageInfo, textureInfo);

    SkPaint redPaint;
    redPaint.setColor(SkColors::kRed);
    canvasB->drawRect(SkRect::MakeWH(8, 8), redPaint);

    // Snap and insert the recording from B, targeting surface A. This should mark surface A as
    // initialized.
    std::unique_ptr<Recording> recordingB = recorderB->snap();
    InsertRecordingInfo infoB;
    infoB.fRecording = recordingB.get();
    infoB.fTargetSurface = surfaceA.get();
    context->insertRecording(infoB);

    // Now snap an empty recording from A. This should NOT clear the surface since it is now
    // considered initialized.
    std::unique_ptr<Recording> recordingA2 = recorderA->snap();
    InsertRecordingInfo infoA2;
    infoA2.fRecording = recordingA2.get();
    infoA2.fTargetSurface = surfaceA.get();
    context->insertRecording(infoA2);

    // Check that the surface is not cleared and has the red color drawn to the deferred canvas.
    std::vector<Expectation> expectations = {{0, 0, SkColors::kRed}};
    check_expectations(reporter, surfaceA.get(), expectations);
}

}  // namespace skgpu::graphite
