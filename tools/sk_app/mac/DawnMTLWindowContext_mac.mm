/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/DawnWindowContext.h"
#include "tools/sk_app/mac/WindowContextFactory_mac.h"
#include "webgpu/webgpu_cpp.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/MetalBackend.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>

namespace sk_app {

using sk_app::window_context_factory::MacWindowInfo;

class DawnMTLWindowContext : public DawnWindowContext {
public:
    DawnMTLWindowContext(const MacWindowInfo& info, const DisplayParams& params);
    ~DawnMTLWindowContext() override;
    wgpu::Device onInitializeContext() override;
    void onDestroyContext() override;
    wgpu::Surface createDawnSurface() override;
    void onSwapBuffers() override;
    void resize(int width, int height) override;
private:
    NSView*              fMainView;
    id<MTLDevice>        fMTLDevice;
    CAMetalLayer*        fLayer;
};

DawnMTLWindowContext::DawnMTLWindowContext(const MacWindowInfo& info, const DisplayParams& params)
    : DawnWindowContext(params, wgpu::TextureFormat::BGRA8Unorm)
    , fMainView(info.fMainView) {
    CGFloat backingScaleFactor = sk_app::GetBackingScaleFactor(fMainView);
    CGSize size = fMainView.bounds.size;
    size.width *= backingScaleFactor;
    size.height *= backingScaleFactor;
    this->initializeContext(size.width, size.height);
}

DawnMTLWindowContext::~DawnMTLWindowContext() {
    this->destroyContext();
}

wgpu::Surface DawnMTLWindowContext::createDawnSurface() {
    wgpu::SurfaceDescriptorFromMetalLayer layerDesc;
    layerDesc.layer = fLayer;

    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &layerDesc;

    wgpu::Instance instance = wgpu::Instance(fInstance->Get());
    return instance.CreateSurface(&surfaceDesc);
}

wgpu::Device DawnMTLWindowContext::onInitializeContext() {
    wgpu::Device device = this->createDevice(wgpu::BackendType::Metal);
    if (!device) {
        return nullptr;
    }

    // We assume that Dawn is using the default device. This could be wrong on multi-GPU systems.
    fMTLDevice = MTLCreateSystemDefaultDevice();

    CGSize size;
    size.width = width();
    size.height = height();

    fLayer = [CAMetalLayer layer];
    [fLayer setDevice:fMTLDevice];
    [fLayer setPixelFormat: MTLPixelFormatBGRA8Unorm];
    [fLayer setFramebufferOnly: YES];
    [fLayer setDrawableSize: size];
    [fLayer setColorspace: CGColorSpaceCreateDeviceRGB()];
    [fLayer setContentsScale: sk_app::GetBackingScaleFactor(fMainView)];
    [fLayer setContentsGravity: kCAGravityTopLeft];
    [fLayer setAutoresizingMask: kCALayerHeightSizable | kCALayerWidthSizable];

    [fMainView setWantsLayer: YES];
    [fMainView setLayer: fLayer];

    return device;
}

void DawnMTLWindowContext::onDestroyContext() {
}

void DawnMTLWindowContext::onSwapBuffers() {
}

void DawnMTLWindowContext::resize(int w, int h) {
    CGFloat backingScaleFactor = sk_app::GetBackingScaleFactor(fMainView);
    CGSize size = fMainView.bounds.size;
    size.width *= backingScaleFactor;
    size.height *= backingScaleFactor;

    fLayer.drawableSize = size;
    fLayer.contentsScale = backingScaleFactor;

    DawnWindowContext::resize(size.width, size.height);
}

namespace window_context_factory {

std::unique_ptr<WindowContext> MakeDawnMTLForMac(const MacWindowInfo& winInfo,
                                                 const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new DawnMTLWindowContext(winInfo, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}

}   //namespace sk_app
