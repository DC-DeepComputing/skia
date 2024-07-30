/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "include/gpu/graphite/mtl/MtlGraphiteUtils.h"

#include "graphite_metal_context_helper.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <name.jpeg>\n", argv[0]);
        return 1;
    }

    SkFILEWStream output(argv[1]);
    if (!output.isValid()) {
        printf("Cannot open output file %s\n", argv[1]);
        return 1;
    }

    skgpu::graphite::MtlBackendContext backendContext = GetMetalContext();
    skgpu::graphite::ContextOptions options;

    std::unique_ptr<skgpu::graphite::Context> context =
        skgpu::graphite::ContextFactory::MakeMetal(backendContext, options);
    if (!context) {
        printf("Could not make Graphite Native Metal context\n");
        return 1;
    }
    printf("Context made, now to make the surface\n");

    SkImageInfo imageInfo =
            SkImageInfo::Make(200, 400, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    if (!recorder) {
        printf("Could not make recorder\n");
        return 1;
    }
    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(recorder.get(), imageInfo);
    if (!surface) {
        printf("Could not make surface from Metal Recorder\n");
        return 1;
    }

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorCYAN);
    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(10, 20, 50, 70), 10, 10);

    SkPaint paint;
    paint.setColor(SK_ColorYELLOW);
    paint.setAntiAlias(true);

    canvas->drawRRect(rrect, paint);

    printf("ready to snap the GPU calls\n");
    // Now to send the draws to the GPU
    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
    if (!recording) {
        printf("Could not create a recording\n");
        return 1;
    }
    skgpu::graphite::InsertRecordingInfo info;
    info.fRecording = recording.get();
    if (!context->insertRecording(info)) {
        printf("Context::insertRecording failed\n");
        return 1;
    }

    printf("Instructions given to surface\n");
    SkBitmap out;
    if (!out.tryAllocPixels(imageInfo, 0)) {
        printf("Could not allocate output pixels\n");
        return 1;
    }
    sk_sp<SkImage> img = SkImages::RasterFromBitmap(out);
    printf("Scheduling pixel readback\n");
    context->asyncRescaleAndReadPixels(img.get(), imageInfo, SkIRect::MakeSize({200, 400}),
                                       SkImage::RescaleGamma::kSrc,
                                       SkImage::RescaleMode::kRepeatedCubic,
                                       nullptr, nullptr);

    printf("Submitting work to GPU and waiting for it to be done\n");
    context->submit(skgpu::graphite::SyncToCpu::kYes); // Note this doesn't work on all backend types, e.g. Dawn.
    if (context->hasUnfinishedGpuWork()) {
        printf("Work is still pending\n");
        return 1;
    }
    context->checkAsyncWorkCompletion();

    sk_sp<SkData> jpeg = SkJpegEncoder::Encode(nullptr, img.get(), {});
    if (!jpeg) {
        printf("Readback of pixels (or encoding) failed\n");
        return 1;
    }
    output.write(jpeg->data(), jpeg->size());
    output.fsync();
    return 0;
}
