/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkAutoMalloc.h"
#include "tools/sk_app/GraphiteDawnWindowContext.h"

#include "dawn/dawn_proc.h"

static wgpu::TextureUsage kUsage = wgpu::TextureUsage::RenderAttachment |
                                   wgpu::TextureUsage::CopySrc;

static void PrintDeviceError(WGPUErrorType, const char* message, void*) {
    printf("Device error: %s\n", message);
    SkASSERT(false);
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
    fDevice = onInitializeContext();

    fContext = GrDirectContext::MakeDawn(fDevice, fDisplayParams.fGrContextOptions);
    if (!fContext) {
        return;
    }
    fSwapChainImplementation = this->createSwapChainImplementation(-1, -1, fDisplayParams);
    wgpu::SwapChainDescriptor swapChainDesc;
    swapChainDesc.implementation = reinterpret_cast<int64_t>(&fSwapChainImplementation);
    fSwapChain = fDevice.CreateSwapChain(nullptr, &swapChainDesc);
    if (!fSwapChain) {
        fContext.reset();
        return;
    }
    fSwapChain.Configure(fSwapChainFormat, kUsage, width, height);
    fDevice.SetUncapturedErrorCallback(PrintDeviceError, 0);
}

GraphiteDawnWindowContext::~GraphiteDawnWindowContext() {
}

void GraphiteDawnWindowContext::destroyContext() {
    if (!fDevice.Get()) {
        return;
    }

    this->onDestroyContext();

    fContext.reset();
    fDevice = nullptr;
}

sk_sp<SkSurface> GraphiteDawnWindowContext::getBackbufferSurface() {
    GrDawnRenderTargetInfo rtInfo;
    rtInfo.fTextureView = fSwapChain.GetCurrentTextureView();
    rtInfo.fFormat = fSwapChainFormat;
    rtInfo.fLevelCount = 1; // FIXME
    GrBackendRenderTarget backendRenderTarget(fWidth, fHeight, fDisplayParams.fMSAASampleCount, 8,
                                              rtInfo);
    fSurface = SkSurface::MakeFromBackendRenderTarget(fContext.get(),
                                                      backendRenderTarget,
                                                      this->getRTOrigin(),
                                                      fDisplayParams.fColorType,
                                                      fDisplayParams.fColorSpace,
                                                      &fDisplayParams.fSurfaceProps);
    return fSurface;
}

void GraphiteDawnWindowContext::swapBuffers() {
    fSwapChain.Present();
    this->onSwapBuffers();
}

void GraphiteDawnWindowContext::resize(int w, int h) {
    fWidth = w;
    fHeight = h;
    fSwapChainImplementation = this->createSwapChainImplementation(w, h, fDisplayParams);
    wgpu::SwapChainDescriptor swapChainDesc;
    swapChainDesc.implementation = reinterpret_cast<int64_t>(&fSwapChainImplementation);
    fSwapChain = fDevice.CreateSwapChain(nullptr, &swapChainDesc);
    if (!fSwapChain) {
        fContext.reset();
        return;
    }
    fSwapChain.Configure(fSwapChainFormat, kUsage, fWidth, fHeight);
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
            return adapter.CreateDevice();
        }
    }
    return nullptr;
}

}   //namespace sk_app
