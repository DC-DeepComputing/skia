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

#include <format>
#include <string_view>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

sk_sp<skgpu::graphite::DawnSharedContext> MakeDawnContext() {
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

skgpu::graphite::Context* MakeGraphiteContext(sk_sp<skgpu::graphite::DawnSharedContext> dawnCtx) {
    // TODO: Currently returns a raw pointer to avoid dealing with memory atm.
    SkASSERT(dawnCtx);

    auto queueManager = std::make_unique<skgpu::graphite::DawnQueueManager>(dawnCtx->queue(), dawnCtx.get());
    SkASSERT(queueManager);

    return skgpu::graphite::ContextCtorAccessor::MakeContext(std::move(dawnCtx), std::move(queueManager), {}).release();
}

struct GPUContext {
    sk_sp<skgpu::graphite::DawnSharedContext> dawnCtx;
    skgpu::graphite::Context* graphiteCtx;
};

GPUContext* MakeGPUContext() {
    auto dawnCtx = MakeDawnContext();
    auto* graphiteCtx = MakeGraphiteContext(dawnCtx);
    GPUContext* ctx = new GPUContext{dawnCtx, graphiteCtx};
    return ctx;
}

struct GPUCanvasContext {
    wgpu::Surface surface;
    skgpu::graphite::Context* graphiteCtx;
    skgpu::graphite::Recorder* recorder;
    SkCanvas* canvas;
};

GPUCanvasContext* MakeGPUCanvasContext(GPUContext* ctx, std::string_view canvasId, int width, int height) {
    // Create the wgpu::Surface.
    wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector canvasDesc{};
    std::string selector = std::format("#{}", canvasId);
    canvasDesc.selector = selector.c_str();

    wgpu::SurfaceDescriptor surfaceDesc = {};
    surfaceDesc.label = "WebGPU canvas surface";
    surfaceDesc.nextInChain = &canvasDesc;
    wgpu::Surface surface = ctx->dawnCtx->instance().CreateSurface(&surfaceDesc);

    // Configure the wgpu::Surface.
    wgpu::SurfaceCapabilities capabilities;
    surface.GetCapabilities(nullptr, &capabilities);
    wgpu::SurfaceConfiguration config = {};
    config.device = ctx->dawnCtx->device();
    config.format = capabilities.formats[0];
    config.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    config.width = width;
    config.height = height;
    SkASSERT(capabilities.presentModeCount > 0);
    config.presentMode = capabilities.presentModes[0];
    surface.Configure(&config);

    // Create the recorder.
    std::unique_ptr<skgpu::graphite::Recorder> recorder = ctx->graphiteCtx->makeRecorder();

    // We create a placeholder binding of the wgpu::Surface texture into Graphite so that we can
    // create a deferred Canvas.
    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);
    skgpu::graphite::BackendTexture texture = 
        skgpu::graphite::BackendTextures::MakeDawn(surfaceTexture.texture.Get());
    SkImageInfo imageInfo =
        SkImageInfo::Make(surfaceTexture.texture.GetWidth(), surfaceTexture.texture.GetHeight(), kBGRA_8888_SkColorType, kPremul_SkAlphaType);
    SkCanvas* canvas = 
        recorder->makeDeferredCanvas(imageInfo, texture.info());

    return new GPUCanvasContext{ surface, ctx->graphiteCtx, recorder.release(), canvas };
}

SkCanvas* GetGPUCanvas(GPUCanvasContext* canvasCtx) {
    return canvasCtx->canvas;
}

struct Surface {
    wgpu::Surface wgpuSurface;
    wgpu::Texture wgpuTexture;
    skgpu::graphite::BackendTexture skTexture;
    sk_sp<SkSurface> skSurface;
};

Surface MakeGPUSurfaces(skgpu::graphite::DawnSharedContext* gpuContext, skgpu::graphite::Recorder* recorder, std::string_view canvas, int width, int height) {
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

sk_sp<SkSurface> MakeGPUTextureSurface(GPUCanvasContext* canvasCtx) {
    // Create the BackendTexture and SkSurface.
    wgpu::SurfaceTexture surfaceTexture;
    canvasCtx->surface.GetCurrentTexture(&surfaceTexture);
    auto texture = skgpu::graphite::BackendTextures::MakeDawn(surfaceTexture.texture.Get());

    SkSurfaceProps props;
    auto surface = SkSurfaces::WrapBackendTexture(canvasCtx->recorder, texture, kBGRA_8888_SkColorType, SkColorSpace::MakeSRGB(), &props);
    return surface;
}

void Flush(GPUCanvasContext* canvasCtx) {
    // Send the draws to the GPU
    std::unique_ptr<skgpu::graphite::Recording> recording = canvasCtx->recorder->snap();
    if (!recording) {
        printf("Could not create a recording\n");
        return;
    }

    auto surface = MakeGPUTextureSurface(canvasCtx);

    skgpu::graphite::InsertRecordingInfo info;
    info.fTargetSurface = surface.get();
    info.fRecording = recording.get();

    if (!canvasCtx->graphiteCtx->insertRecording(info)) {
        printf("Context::insertRecording failed\n");
        return;
    }

    if (!canvasCtx->graphiteCtx->submit(skgpu::graphite::SyncToCpu::kNo)) {
        printf("Context::submit failed\n");
        return;
    }
}


EMSCRIPTEN_BINDINGS(Graphite) {
    constant("graphite", true);
    constant("webgpu", true);

    class_<GPUContext>("GPUContext");
    class_<GPUCanvasContext>("GPUCanvasContext");
    class_<skgpu::graphite::Recorder>("Recorder");
    class_<skgpu::graphite::DawnSharedContext>("DawnSharedContext");

    function("_MakeGPUContext", optional_override([]()->WASMPointer {
        return reinterpret_cast<WASMPointer>(MakeGPUContext());
    }), emscripten::async());
    function("_MakeGPUCanvasContext", optional_override([](WASMPointer ctxPtr, std::string canvasId, int width, int height)->WASMPointer {
        GPUContext* ctx = reinterpret_cast<GPUContext*>(ctxPtr);
        return reinterpret_cast<WASMPointer>(MakeGPUCanvasContext(ctx, canvasId, width, height));
    }));
    function("_MakeGPUTextureSurface", optional_override([](WASMPointer ctxPtr)->sk_sp<SkSurface> {
        GPUCanvasContext* ctx = reinterpret_cast<GPUCanvasContext*>(ctxPtr);
        return MakeGPUTextureSurface(ctx);
    }));
    function("_GetGPUCanvas", optional_override([](WASMPointer ctxPtr)->SkCanvas* {
        GPUCanvasContext* ctx = reinterpret_cast<GPUCanvasContext*>(ctxPtr);
        return GetGPUCanvas(ctx);
    }), allow_raw_pointers());
    function("_Flush", optional_override([](WASMPointer ctxPtr)->void {
        GPUCanvasContext* ctx = reinterpret_cast<GPUCanvasContext*>(ctxPtr);
        return Flush(ctx);
    }));

    // function("_MakeGPUContext", &MakeGPUContext, allow_raw_pointers());
    // function("_MakeGPUCanvasContext", &MakeGPUCanvasContext, allow_raw_pointers());
    // function("_MakeGPUTextureSurface", &MakeGPUTextureSurface, allow_raw_pointers());
    // function("_GetGPUCanvas", &GetGPUCanvas, allow_raw_pointers());

    function("TestGraphite", optional_override([](std::string canvasName)->bool {
        SkDebugf("%s\n", canvasName.c_str());

        constexpr int WIDTH = 800;
        constexpr int HEIGHT = 600;

        auto gpuContext = MakeDawnContext();
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
        auto surfaces = MakeGPUSurfaces(gpuContext.get(), recorder.get(), canvasName, WIDTH, HEIGHT);

        // Some native WebGPU code to test.
        // auto commandEncoder = gpuContext->device().CreateCommandEncoder();
        // wgpu::RenderPassDescriptor renderDesc = {};
        // wgpu::RenderPassColorAttachment attach = {};
        // attach.view = surfaces.wgpuTexture.CreateView();
        // attach.loadOp = wgpu::LoadOp::Clear;
	    // attach.storeOp = wgpu::StoreOp::Store;
        // attach.clearValue = { 1.0, 0., 0., 1.0 };
        // renderDesc.colorAttachmentCount = 1;
        // renderDesc.colorAttachments = &attach;

        // auto renderPass = commandEncoder.BeginRenderPass(&renderDesc);
        // renderPass.End();
        // auto commands = commandEncoder.Finish();
        // gpuContext->queue().Submit(1, &commands);

        // SkImageInfo imageInfo =
        //     SkImageInfo::Make(WIDTH, HEIGHT, kBGRA_8888_SkColorType, kPremul_SkAlphaType);

        SkCanvas* canvas = surfaces.skSurface->getCanvas();
        // SkCanvas* canvas = recorder->makeDeferredCanvas(surfaces.skSurface->imageInfo(), surfaces.skTexture.info());
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

        if (!graphiteContext->submit(skgpu::graphite::SyncToCpu::kNo)) {
            printf("Context::submit failed\n");
        }

        printf("about to return from TestGraphite\n");
        return true;
    }), allow_raw_pointers());
}
