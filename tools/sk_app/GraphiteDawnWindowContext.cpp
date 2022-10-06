/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/GraphiteDawnWindowContext.h"

#include "include/core/SkSurface.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "src/core/SkAutoMalloc.h"
#include "tools/ToolUtils.h"

#include "dawn/dawn_proc.h"

namespace {

void PrintDeviceError(WGPUErrorType, const char* message, void*) {
    printf("Device error: %s\n", message);
    SkASSERT(false);
}

}

namespace sk_app {

GraphiteDawnWindowContext::GraphiteDawnWindowContext(const DisplayParams& params,
                                                     wgpu::TextureFormat swapChainFormat)
    : WindowContext(params)
    , fSwapChainFormat(swapChainFormat)
    , fInstance(std::make_unique<dawn::native::Instance>()) {
}

void GraphiteDawnWindowContext::initializeContext(int width, int height) {
    SkASSERT(!fContext);

    fWidth = width;
    fHeight = height;

    if (!onInitializeContext())
        return;

    SkASSERT(fDevice);
    SkASSERT(fSurface);
    SkASSERT(fSwapChain);

    skgpu::graphite::DawnBackendContext backendContext;
    backendContext.fDevice = fDevice;
    backendContext.fQueue = fDevice.GetQueue();
    fGraphiteContext = skgpu::graphite::Context::MakeDawn(backendContext,
                                                          skgpu::graphite::ContextOptions());
    if (!fGraphiteContext)
        return;

    fGraphiteRecorder = fGraphiteContext->makeRecorder(ToolUtils::CreateTestingRecorderOptions());
}

GraphiteDawnWindowContext::~GraphiteDawnWindowContext() = default;

void GraphiteDawnWindowContext::destroyContext() {
    if (!fDevice.Get())
        return;

    this->onDestroyContext();

    fGraphiteRecorder = nullptr;
    fGraphiteContext = nullptr;
    fSwapChain = nullptr;
    fSurface = nullptr;
    fDevice = nullptr;
    fInstance = nullptr;
}

sk_sp<SkSurface> GraphiteDawnWindowContext::getBackbufferSurface() {
    auto texture = fSwapChain.GetCurrentTextureView();
    skgpu::graphite::BackendTexture backendTex(this->dimensions(),
                                               std::move(texture),
                                               1,
                                               0,
                                               fSwapChainFormat,
                                               kTextureUsage);
    SkASSERT(this->graphiteRecorder());
    auto surface = SkSurface::MakeGraphiteFromBackendTexture(
        this->graphiteRecorder(),
        backendTex,
        kBGRA_8888_SkColorType,
        fDisplayParams.fColorSpace,
        &fDisplayParams.fSurfaceProps);
    SkASSERT(surface);
    return surface;
}

void GraphiteDawnWindowContext::swapBuffers() {
    fSwapChain.Present();
    this->onSwapBuffers();
}

void GraphiteDawnWindowContext::setDisplayParams(const DisplayParams& params) {
    fDisplayParams = params;
}

wgpu::Device GraphiteDawnWindowContext::createDevice(wgpu::BackendType type) {
    fInstance->DiscoverDefaultAdapters();
    DawnProcTable backendProcs = dawn::native::GetProcs();
    dawnProcSetProcs(&backendProcs);

    std::vector<dawn::native::Adapter> adapters = fInstance->GetAdapters();
    for (dawn::native::Adapter adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);
        if (properties.backendType == type) {
            wgpu::Device device(adapter.CreateDevice());
            device.SetUncapturedErrorCallback(PrintDeviceError, 0);
            return device;
        }
    }
    return nullptr;
}

}   //namespace sk_app
