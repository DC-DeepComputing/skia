/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstdio>
#include <memory>

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/vk/VulkanGraphiteContext.h"
#include "src/capture/SkCapture.h"
#include "vulkan_context_helper.h"

#define WIDTH 200
#define HEIGHT 400

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Specify a single write location.\n");
        return 1;
    }

    SkFILEWStream output(argv[1]);
    if (!output.isValid()) {
        printf("Cannot open output file %s\n", argv[1]);
        return 1;
    }

    /* SET UP CONTEXT AND RECORDERS FOR DRAWING AND CAPTURE */

    VulkanContext vkCtx;
    if (!CreateVulkanContext(&vkCtx)) {
        printf("Could not create Vulkan backend context\n");
        return 1;
    }

    skgpu::graphite::ContextOptions options;
    options.fEnableCapture = true;

    std::unique_ptr<skgpu::graphite::Context> context =
            skgpu::graphite::ContextFactory::MakeVulkan(vkCtx.fBackendContext, options);

    if (!context) {
        printf("Could not make Graphite Native Vulkan context\n");
        return 1;
    }
    printf("Context made, now to make the surface\n");

    SkImageInfo imageInfo =
            SkImageInfo::Make(WIDTH, HEIGHT, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    if (!recorder) {
        printf("Could not make recorder\n");
        return 1;
    }
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), imageInfo);
    if (!surface) {
        printf("Could not make surface from Vulkan Recorder\n");
        return 1;
    }

    sk_sp<SkSurface> surfaceB = SkSurfaces::RenderTarget(recorder.get(), imageInfo);
    if (!surfaceB) {
        printf("Could not make second surface from Vulkan Recorder\n");
        return 1;
    }

    /* DRAWING */
    context->startCapture();
    printf("Capture started, now to draw\n");

    SkCanvas* canvas = surface->getCanvas();
    SkCanvas* canvasB = surfaceB->getCanvas();

    // Canvas A
    canvas->clear(SK_ColorCYAN);
    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(10, 20, 50, 70), 10, 10);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);

    canvas->drawRRect(rrect, paint);

    // Triggers a breakpoint in the capture
    auto contentImg = surface->makeImageSnapshot();

    // Do some more draws
    canvas->drawCircle(50, 50, 30, paint);
    paint.setColor(SK_ColorBLUE);
    canvas->drawCircle(30, 30, 10, paint);

    // Trigger another breakpoint
    auto circlesImg = surface->makeImageSnapshot();

    // Canvas B
    canvasB->clear(SK_ColorMAGENTA);
    paint.setColor(SK_ColorBLACK);
    canvasB->drawImage(contentImg, 0, 0);
    canvasB->drawImage(circlesImg, 20, 20);
    canvasB->drawCircle(10, 10, 5, paint);
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

    sk_sp<SkImage> img;
    auto callback = [](SkImage::ReadPixelsContext ctx,
                       std::unique_ptr<const SkImage::AsyncReadResult> result) {
        if (!result || result->count() != 1) {
            printf("Didn't load exactly one plane\n");
            return;
        }
        auto ii = SkImageInfo::Make(WIDTH, HEIGHT, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        sk_sp<SkImage>* output = reinterpret_cast<sk_sp<SkImage>*>(ctx);

        SkPixmap pm(ii, result->data(0), result->rowBytes(0));
        *output = SkImages::RasterFromPixmapCopy(pm);
    };

    printf("GPU operations modifying surface have been inserted in the command buffer,"
           "scheduling pixel readback\n");
    context->asyncRescaleAndReadPixels(surface.get(),
                                       imageInfo,
                                       SkIRect::MakeSize({WIDTH, HEIGHT}),
                                       SkImage::RescaleGamma::kSrc,
                                       SkImage::RescaleMode::kRepeatedCubic,
                                       callback,
                                       &img);

    printf("Submitting work to GPU and waiting for it to be done\n");
    context->submit(skgpu::graphite::SyncToCpu::kYes);
    if (context->hasUnfinishedGpuWork()) {
        printf("Sync with GPU completion failed\n");
        return 1;
    }

    auto capture = context->endCapture();
    if (!capture) {
        printf("No capture generated\n");
        return 1;
    }
    auto serializedCapture = capture->serializeCapture();

    /* WRITE DATA TO FILE LOCATION */
    if (serializedCapture) {
        output.write(serializedCapture->data(), serializedCapture->size());
        output.fsync();
        printf("Capture serialized and written to %s\n", argv[1]);
    } else {
        printf("Failed to serialize capture\n");
        return 1;
    }

    /* DESERIALIZE CAPTURE AND INSPECT CONTENTS */
    if (serializedCapture) {
        auto deserializedCapture = SkCapture::MakeFromData(serializedCapture);
        if (deserializedCapture) {
            printf("Successfully deserialized capture for inspection\n");
        }
    }

    printf("done\n");
    return 0;
}
