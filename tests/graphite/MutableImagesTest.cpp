/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#ifdef SK_GRAPHITE_ENABLED

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

using namespace skgpu::graphite;

namespace {

// We draw the larger image into the smaller surface to force mipmapping
const SkISize kImageSize = { 32, 32 };
const SkISize kSurfaceSize = { 16, 16 };

/*
 * We have 3 use cases. In each case there is a mutating task which changes the contents of an
 * image somehow and a shared draw task which just creates a single Recording which draws the
 * image that is being mutated. The mutation tasks are:
 *  1) The client has wrapped a backend texture and is changing its contents out of band w/ the
 *     raw graphics API (or, say, a video decoding chip).
 *  2) The client has wrapped a backend texture in an image and is changing its contents using
 *     Graphite's updateBackendTexture call
 *  3) The client has a surface and has snapped an image w/o a copy but keeps drawing to the
 *     surface
 *
 * There are also two scenarios for the mutation and redrawing tasks:
 *  a) Both are on the same thread and use the same recorder
 *  b) They are on different threads and use separate recorders
 * The latter, obviously, requires more synchronization.
 */
void case1(skiatest::Reporter* reporter, Context* context) {
    // Hmm - probably can't do this one. Maybe for one backend.
}

void case2(skiatest::Reporter* reporter, Context* context) {
    auto caps = context->priv().caps();
    auto recorder = context->makeRecorder();

    TextureInfo info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                          /*levelCount=*/1,
                                                          skgpu::Protected::kNo,
                                                          Renderable::kNo); // Note: not renderable
    REPORTER_ASSERT(reporter, info.isValid());

    BackendTexture beTexture = recorder->createBackendTexture(kImageSize, info);
    REPORTER_ASSERT(reporter, beTexture.isValid());

    // Will need updateBackendTexture

    recorder->deleteBackendTexture(beTexture);
}



void mutate_image(Context* context,
                  Recorder* recorder,
                  SkSurface* mutatingSurface,
                  SkColor4f color) {
    mutatingSurface->getCanvas()->clear(color);

    std::unique_ptr<Recording> recording = recorder->snap();

    skgpu::graphite::InsertRecordingInfo info;
    info.fRecording = recording.get();
    context->insertRecording(info);
}

void insert_draw_image_recording(Context* context, Recording* recording) {
    skgpu::graphite::InsertRecordingInfo info;
    info.fRecording = recording;
    context->insertRecording(info);
}

void check_result(skiatest::Reporter* reporter,
                  Context* context,
                  Recorder* recorder,
                  SkSurface* surface,
                  int testcaseID,
                  const SkColor4f& expectedColor) {
    SkImageInfo ii = SkImageInfo::Make(kSurfaceSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkBitmap result;
    result.allocPixels(ii);
    SkPixmap pm;

    SkAssertResult(result.peekPixels(&pm));

    // For now, we cast and call directly into Surface. Once we have a better idea of
    // what the public API for synchronous graphite readPixels we can update this call to use
    // that instead.
    if (!static_cast<skgpu::graphite::Surface*>(surface)->onReadPixels(context,
                                                                       recorder,
                                                                       pm,
                                                                       0,
                                                                       0)) {
        ERRORF(reporter, "readPixels failed");
    }

    auto error = std::function<ComparePixmapsErrorReporter>(
            [&](int x, int y, const float diffs[4]) {
                ERRORF(reporter,
                       "case %d: expected (%.1f %.1f %.1f %.1f) got (%.1f, %.1f, %.1f, %.1f)",
                       testcaseID,
                       expectedColor.fR, expectedColor.fG, expectedColor.fB, expectedColor.fA,
                       expectedColor.fR-diffs[0], expectedColor.fG-diffs[1],
                       expectedColor.fB-diffs[2], expectedColor.fA-diffs[3]);
            });

    static constexpr float kTol[] = {0, 0, 0, 0};
    CheckSolidPixels(expectedColor, pm, kTol, error);
}

// Mutation method: Surface that shared texture with an image
// Scenario: Mutation and draw occur on the same thread using the same recorder
void case3a(skiatest::Reporter* reporter, Context* context) {
    constexpr int kTestCase3 = 3;

    auto recorder = context->makeRecorder();

    sk_sp<SkSurface> mutatingSurface;
    sk_sp<SkImage> mutatingImg;
    {
        SkImageInfo ii = SkImageInfo::Make(kImageSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        mutatingSurface = SkSurface::MakeGraphite(recorder.get(), ii);
        REPORTER_ASSERT(reporter, mutatingSurface);

        mutatingSurface->getCanvas()->clear(SkColors::kRed);

        mutatingImg = mutatingSurface->experimental_makeImageSnapshotNoCopy();
        REPORTER_ASSERT(reporter, mutatingImg);

        std::unique_ptr<Recording> imgCreationRecording = recorder->snap();

        skgpu::graphite::InsertRecordingInfo info;
        info.fRecording = imgCreationRecording.get();
        context->insertRecording(info);
    }

    sk_sp<SkSurface> imageDrawSurface;
    std::unique_ptr<Recording> imgDrawRecording;
    {
        SkImageInfo ii = SkImageInfo::Make(kSurfaceSize, kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType);

        imageDrawSurface = SkSurface::MakeGraphite(recorder.get(), ii,
                                                   Mipmapped::kYes);
        REPORTER_ASSERT(reporter, imageDrawSurface);

        SkSamplingOptions sampling = SkSamplingOptions(SkFilterMode::kLinear,
                                                       SkMipmapMode::kNearest);

        SkCanvas* canvas = imageDrawSurface->getCanvas();

        canvas->drawImageRect(mutatingImg.get(),
                              SkRect::MakeWH(kSurfaceSize.width(), kSurfaceSize.height()),
                              sampling);

        imgDrawRecording = recorder->snap();
    }

    // The mutable image was initialized to red
    insert_draw_image_recording(context, imgDrawRecording.get());

    check_result(reporter, context, recorder.get(), imageDrawSurface.get(),
                 kTestCase3, SkColors::kRed);

    for (auto c : { SkColors::kGreen, SkColors::kBlue }) {
        // Mutate the image
        mutate_image(context, recorder.get(), mutatingSurface.get(), c);

        // Redraw the image
        insert_draw_image_recording(context, imgDrawRecording.get());

        // Check we got the new color
        check_result(reporter, context, recorder.get(), imageDrawSurface.get(), kTestCase3, c);
    }
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(MutableImagesTest, reporter, context) {
    case1(reporter, context);
    case2(reporter, context);
    case3a(reporter, context);
}

#endif // SK_GRAPHITE_ENABLED
