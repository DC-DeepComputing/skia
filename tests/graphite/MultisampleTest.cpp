/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPixmap.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Surface_Graphite.h"

namespace skgpu::graphite {

// Tests that a drawing with MSAA will have contents retained between recordings.
// This is for testing MSAA load from resolve feature.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(MultisampleRetainTest, reporter, context) {
    const SkImageInfo surfaceImageInfo = SkImageInfo::Make(
            16, 16, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);

    std::unique_ptr<Recorder> surfaceRecorder = context->makeRecorder();
    sk_sp<SkSurface> surface = SkSurface::MakeGraphite(surfaceRecorder.get(), surfaceImageInfo);
    Surface* graphiteSurface = static_cast<Surface*>(surface.get());
    const TextureInfo& textureInfo = graphiteSurface->backingTextureProxy()->textureInfo();

    // Clear entire surface to red
    SkCanvas* surfaceCanvas = surfaceRecorder->makeDeferredCanvas(surfaceImageInfo, textureInfo);
    surfaceCanvas->clear(SkColors::kRed);
    std::unique_ptr<Recording> surfaceRecording = surfaceRecorder->snap();
    // Flush the clearing
    context->insertRecording({surfaceRecording.get(), surface.get()});

    // Draw a blue path. The old red background should be retained between recordings.
    const SkImageInfo recordingImageInfo = surfaceImageInfo.makeDimensions(SkISize::Make(8, 16));
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkCanvas* canvas = recorder->makeDeferredCanvas(recordingImageInfo, textureInfo);

    SkPaint paint;
    paint.setStrokeWidth(3);
    paint.setARGB(255, 0, 0, 255);
    paint.setStyle(SkPaint::Style::kStroke_Style);

    SkPath path;
    constexpr int kPathPoints[][2] = {
            {3, 2},
            {3, 4},
            {6, 8},
            {3, 15},
    };

    for (size_t i = 0; i < std::size(kPathPoints); ++i) {
        path.lineTo(kPathPoints[i][0], kPathPoints[i][1]);
    }

    canvas->drawPath(path, paint);

    std::unique_ptr<Recording> recording = recorder->snap();
    // Play back recording.
    context->insertRecording({recording.get(), surface.get()});

    // Read pixels.
    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(surfaceImageInfo);
    SkAssertResult(bitmap.peekPixels(&pixmap));
    if (!surface->readPixels(pixmap, 0, 0)) {
        ERRORF(reporter, "readPixels failed");
        return;
    }

    // Verify recording was replayed.
    REPORTER_ASSERT(reporter, pixmap.getColor4f(8, 0) == SkColors::kRed);
    REPORTER_ASSERT(reporter, pixmap.getColor4f(0, 8) == SkColors::kRed);
    REPORTER_ASSERT(reporter, pixmap.getColor4f(15, 14) == SkColors::kRed);

    // Very points on the path has blue color. We don't verify last point because it is on the edge
    // of the path thus might be blurry not having exact blue color.
    for (size_t i = 0; i < std::size(kPathPoints) - 1; ++i) {
        REPORTER_ASSERT(reporter,
                        pixmap.getColor4f(kPathPoints[i][0], kPathPoints[i][1]) == SkColors::kBlue);
    }
}

}  // namespace skgpu::graphite
