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
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "include/gpu/graphite/dawn/DawnUtils.h"

#include "dawn/dawn_proc.h"

#define WIDTH 200
#define HEIGHT 400

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

    DawnProcTable backendProcs = dawn::native::GetProcs();
    dawnProcSetProcs(&backendProcs);
    WGPUInstanceDescriptor idesc{};
    std::unique_ptr<dawn::native::Instance> instance = std::make_unique<dawn::native::Instance>(&idesc);
    if (!instance) {
        printf("Cannot create instance\n");
        return 1;
    }
    printf("Found these adapters:\n");
    std::vector<dawn::native::Adapter> adapters = instance->EnumerateAdapters(&options);
    for (const auto& adapter : adapters) {
        wgpu::AdapterInfo props;
        adapter.GetInfo(&props);
        printf("GPU: %s\nDriver: %s\n", props.device, props.description);
    }

    wgpu::DawnTogglesDescriptor togglesDesc;

    wgpu::DeviceDescriptor desc;
    desc.nextInChain = &togglesDesc;
    wgpu::Device device = wgpu::Device::Acquire(matchedAdaptor.CreateDevice(&ddesc));

    skgpu::graphite::DawnBackendContext backendContext;
    backendContext.fInstance = wgpu::Instance(instance->Get());
    backendContext.fDevice = device;
    backendContext.fQueue  = device.GetQueue();

    skgpu::graphite::ContextOptions options;
    std::unique_ptr<skgpu::graphite::Context> context = skgpu::graphite::ContextFactory::MakeDawn(backendContext, options);
    if (!context) {
        printf("Cannot make graphite Dawn context\n");
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

    sk_sp<SkImage> img;
    auto callback = [](SkImage::ReadPixelsContext ctx,
                       std::unique_ptr<const SkImage::AsyncReadResult> result) {
        if (result->count() != 1) {
            printf("Didn't load exactly one frame\n");
        }
        auto ii = SkImageInfo::Make(WIDTH, HEIGHT, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        sk_sp<SkImage>* output = reinterpret_cast<sk_sp<SkImage>*>(ctx);

        SkPixmap pm(ii, result->data(0), result->rowBytes(0));
        *output = SkImages::RasterFromPixmapCopy(pm);
    };
    printf("Instructions given to surface, scheduling pixel readback\n");
    context->asyncRescaleAndReadPixels(surface.get(), imageInfo, SkIRect::MakeSize({WIDTH, HEIGHT}),
                                       SkImage::RescaleGamma::kSrc,
                                       SkImage::RescaleMode::kRepeatedCubic,
                                       callback, &img);

    printf("Submitting work to GPU and waiting for it to be done\n");
    // Note this doesn't work on all backend types, e.g. Dawn.
    context->submit(skgpu::graphite::SyncToCpu::kYes);
    if (context->hasUnfinishedGpuWork()) {
        printf("Work is still pending\n");
        return 1;
    }

    sk_sp<SkData> jpeg = SkJpegEncoder::Encode(nullptr, img.get(), {});
    if (!jpeg) {
        printf("Readback of pixels (or encoding) failed\n");
        return 1;
    }
    output.write(jpeg->data(), jpeg->size());
    output.fsync();
    printf("done\n");
    return 0;
}
