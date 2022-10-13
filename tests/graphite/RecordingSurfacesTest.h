/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RecordingSurfacesTest_DEFINED
#define RecordingSurfacesTest_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPixmap.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Surface_Graphite.h"

namespace skgpu::graphite {

// Tests that a Recording can be replayed to a provided surface.
void RunRecordingSurfacesTest(skiatest::Reporter* reporter, Context* context, SkSurface* surface) {
    const SkImageInfo ii = surface->imageInfo();

    std::unique_ptr<Recorder> recorder = context->makeRecorder({}, ii.dimensions(), ii.colorInfo());
    SkCanvas* canvas = recorder->getRecordingCanvas();
    canvas->clear(SkColors::kRed);
    std::unique_ptr<Recording> recording = recorder->snap();
    context->insertRecording({recording.get(), surface});

    // For now, we cast and call directly into Surface. Once we have a better idea of
    // what the public API for synchronous graphite readPixels we can update this call to use
    // that instead.
    SkBitmap bitmap;
    SkPixmap pixmap;
    bitmap.allocPixels(ii);
    SkAssertResult(bitmap.peekPixels(&pixmap));
    Surface* graphiteSurface = static_cast<Surface*>(surface);
    SkAssertResult(
            graphiteSurface->onReadPixels(context, graphiteSurface->onGetRecorder(), pixmap, 0, 0));

    REPORTER_ASSERT(reporter, pixmap.getColor4f(0, 0) == SkColors::kRed);
    REPORTER_ASSERT(reporter, !recording->isTargetProxyInstantiated());
}

}  // namespace skgpu::graphite

#endif  // RecordingSurfacesTest_DEFINED
