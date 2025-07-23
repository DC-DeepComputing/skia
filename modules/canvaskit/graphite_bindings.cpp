/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/dawn/DawnQueueManager.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

#include "modules/canvaskit/WasmCommon.h"

#include <string_view>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

sk_sp<skgpu::graphite::DawnSharedContext> MakeGPUContext() {
    skgpu::graphite::DawnBackendContext ctx;

    // Create the wgpu::Instance.
    {
        wgpu::InstanceDescriptor desc = {};
        static constexpr auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
        desc.requiredFeatureCount = 1;
        desc.requiredFeatures = &kTimedWaitAny;
        ctx.fInstance = wgpu::CreateInstance(&desc);
    }
    // Create the wgpu::Adapter.
    wgpu::Adapter adapter;
    {
        wgpu::RequestAdapterOptions opts = {};
        ctx.fInstance.WaitAny(
            ctx.fInstance.RequestAdapter(&opts, wgpu::CallbackMode::WaitAnyOnly,
                                         [&adapter](wgpu::RequestAdapterStatus status,
                                                    wgpu::Adapter res, wgpu::StringView message) {
                                             if (status != wgpu::RequestAdapterStatus::Success) {
                                                 SkDebugf("Failed to get an adapter: %.*s\n",
                                                          int(message.length), message.data);
                                                 return;
                                             }
                                             printf("Successfully created an adapter\n");
                                             adapter = std::move(res);
                                         }),
            UINT64_MAX);
        SkASSERT(adapter);
    }
    // Create the wgpu::Device.
    {
        wgpu::DeviceDescriptor desc = {};
        desc.SetDeviceLostCallback(
            wgpu::CallbackMode::AllowSpontaneous,
            [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
                const char* reasonName = "";
                switch (reason) {
                    case wgpu::DeviceLostReason::Unknown:
                        reasonName = "Unknown";
                        break;
                    case wgpu::DeviceLostReason::Destroyed:
                        reasonName = "Destroyed";
                        break;
                    case wgpu::DeviceLostReason::CallbackCancelled:
                        reasonName = "CallbackCancelled";
                        break;
                    case wgpu::DeviceLostReason::FailedCreation:
                        reasonName = "FailedCreation";
                        break;
                    default:
                        SkASSERT(false);
                }
                SkDebugf("Device lost because of %s: %.*s\n", reasonName, int(message.length),
                         message.data);
            });
        desc.SetUncapturedErrorCallback(
            [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
                const char* errorTypeName = "";
                switch (type) {
                    case wgpu::ErrorType::Validation:
                        errorTypeName = "Validation";
                        break;
                    case wgpu::ErrorType::OutOfMemory:
                        errorTypeName = "Out of memory";
                        break;
                    case wgpu::ErrorType::Internal:
                        errorTypeName = "Internal";
                        break;
                    case wgpu::ErrorType::Unknown:
                        errorTypeName = "Unknown";
                        break;
                    default:
                        SkASSERT(false);
                }
                SkDebugf("%s error: %.*s\n", errorTypeName, int(message.length), message.data);
            });

        ctx.fInstance.WaitAny(
            adapter.RequestDevice(&desc, wgpu::CallbackMode::WaitAnyOnly,
                                  [&ctx](wgpu::RequestDeviceStatus status, wgpu::Device device,
                                         wgpu::StringView message) {
                                      if (status != wgpu::RequestDeviceStatus::Success) {
                                          SkDebugf("Failed to get a device: %.*s\n",
                                                   int(message.length), message.data);
                                          return;
                                      }
                                      printf("Successfully created an device\n");
                                      ctx.fDevice = std::move(device);
                                      ctx.fQueue = ctx.fDevice.GetQueue();
                                  }),
            UINT64_MAX);
        SkASSERT(ctx.fDevice);
    }

    return sk_sp(static_cast<skgpu::graphite::DawnSharedContext*>(skgpu::graphite::DawnSharedContext::Make(ctx, {}).release()));
}

std::unique_ptr<skgpu::graphite::Context> MakeGraphiteContext(sk_sp<skgpu::graphite::DawnSharedContext> gpuContext) {
    SkASSERT(gpuContext);

    auto queueManager = std::make_unique<skgpu::graphite::DawnQueueManager>(gpuContext->queue(), gpuContext.get());
    SkASSERT(queueManager);

    return skgpu::graphite::ContextCtorAccessor::MakeContext(std::move(gpuContext), std::move(queueManager), {});
}

struct Surface {
    wgpu::Surface wgpuSurface;
    wgpu::Texture wgpuTexture;
    skgpu::graphite::BackendTexture skTexture;
    sk_sp<SkSurface> skSurface;
};

Surface MakeGPUSurface(skgpu::graphite::DawnSharedContext* gpuContext, skgpu::graphite::Recorder* recorder, std::string_view canvas, int width, int height) {
    Surface surface;

    // Create the wgpu::Surface.
    wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector canvasDesc{};
    canvasDesc.selector = canvas;

    wgpu::SurfaceDescriptor surfaceDesc = {};
    surfaceDesc.label = "WebGPU canvas surface";
    surfaceDesc.nextInChain = &canvasDesc;
    surface.wgpuSurface = gpuContext->instance().CreateSurface(&surfaceDesc);

    // Configure the wgpu::Surface.
    wgpu::SurfaceCapabilities capabilities;
    surface.wgpuSurface.GetCapabilities(nullptr, &capabilities);
    wgpu::SurfaceConfiguration config = {};
    config.device = gpuContext->device();
    config.format = capabilities.formats[0];
    config.width = width;
    config.height = height;
    SkASSERT(capabilities.presentModeCount > 0);
    config.presentMode = capabilities.presentModes[0];
    surface.wgpuSurface.Configure(&config);

    // Create the BackendTexture and SkSurface.
    wgpu::SurfaceTexture surfaceTexture;
    surface.wgpuSurface.GetCurrentTexture(&surfaceTexture);
    surface.wgpuTexture = surfaceTexture.texture;
    surface.skTexture = skgpu::graphite::BackendTextures::MakeDawn(surfaceTexture.texture.Get());
    SkSurfaceProps props;
    surface.skSurface = SkSurfaces::WrapBackendTexture(recorder, surface.skTexture, kBGRA_8888_SkColorType, SkColorSpace::MakeSRGB(), &props);

    return surface;
}

EMSCRIPTEN_BINDINGS(Graphite) {
    constant("graphite", true);
    constant("webgpu", true);

    function("MakeGPUContext", &MakeGPUContext);
    function("MakeGraphiteContext", &MakeGraphiteContext);

    function("TestGraphite", optional_override([](std::string canvasName)->bool {
        SkDebugf("%s\n", canvasName.c_str());

        constexpr int WIDTH = 800;
        constexpr int HEIGHT = 600;

        auto gpuContext = MakeGPUContext();
        auto graphiteContext = MakeGraphiteContext(gpuContext);

        if (!graphiteContext) {
            return false;
        }
        SkDebugf("Have context\n");

        std::unique_ptr<skgpu::graphite::Recorder> recorder = graphiteContext->makeRecorder();
        if (!recorder) {
            printf("Could not make recorder\n");
            return 1;
        }

        // Create the surfaces.
        auto surfaces = MakeGPUSurface(gpuContext.get(), recorder.get(), canvasName, WIDTH, HEIGHT);

        SkImageInfo imageInfo =
            SkImageInfo::Make(WIDTH, HEIGHT, kBGRA_8888_SkColorType, kPremul_SkAlphaType);

        SkCanvas* canvas = recorder->makeDeferredCanvas(surfaces.skSurface->imageInfo(), surfaces.skTexture.info());
        if (!canvas) {
            printf("Could not make canvas\n");
            return 1;
        }
        canvas->clear(SK_ColorCYAN);
        SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(10, 20, 50, 70), 10, 10);

        SkPaint paint;
        paint.setColor(SK_ColorYELLOW);
        paint.setAntiAlias(true);

        canvas->drawRRect(rrect, paint);

        printf("ready to snap the GPU calls\n");
        // Now to send the draws to the GPU
        std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
        printf("created a recording\n");
        if (!recording) {
            printf("Could not create a recording\n");
            return 1;
        }
        skgpu::graphite::InsertRecordingInfo info;
        info.fTargetSurface = surfaces.skSurface.get();
        info.fRecording = recording.get();
        printf("Got a recording\n");
        if (!graphiteContext->insertRecording(info)) {
            printf("Context::insertRecording failed\n");
            return 1;
        }

        if (!graphiteContext->submit(skgpu::graphite::SyncToCpu::kYes)) {
            printf("Context::submit failed\n");
        }

        printf("about to return from TestGraphite\n");
        return true;
    }), allow_raw_pointers());
}
