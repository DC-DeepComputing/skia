/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/GraphiteDawnWindowContext.h"
#include "tools/sk_app/mac/WindowContextFactory_mac.h"

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAConstraintLayoutManager.h>

using sk_app::DisplayParams;
using sk_app::window_context_factory::MacWindowInfo;
using sk_app::GraphiteDawnWindowContext;

namespace {

class GraphiteDawnMetalWindowContext_mac : public GraphiteDawnWindowContext {
public:
    GraphiteDawnMetalWindowContext_mac(const MacWindowInfo&, const DisplayParams&);

    ~GraphiteDawnMetalWindowContext_mac() override;

    bool onInitializeContext() override;
    void onDestroyContext() override;
    void onSwapBuffers() override;
    DawnSwapChainImplementation createSwapChainImplementation(int width, int height,
        const DisplayParams& params) override;
    void resize(int w, int h) override;

private:
    NSView*              fMainView;

    using INHERITED = GraphiteDawnWindowContext;
};

GraphiteDawnMetalWindowContext_mac::GraphiteDawnMetalWindowContext_mac(const MacWindowInfo& info,
                                                                       const DisplayParams& params)
    : INHERITED(params, wgpu::TextureFormat::RGBA8Unorm)
    , fMainView(info.fMainView) {

    // any config code here (particularly for msaa)?

    // TODO: use real size of the window.
    this->initializeContext(1, 1);
}

GraphiteDawnMetalWindowContext_mac::~GraphiteDawnMetalWindowContext_mac() {
    this->destroyContext();
}

bool GraphiteDawnMetalWindowContext_mac::onInitializeContext() {
    SkASSERT(nil != fMainView);

    // fMetalLayer = [CAMetalLayer layer];
    // fMetalLayer.device = fDevice.get();
    // fMetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;

    // // resize ignores the passed values and uses the fMainView directly.
    // this->resize(0, 0);

    // BOOL useVsync = fDisplayParams.fDisableVsync ? NO : YES;
    // fMetalLayer.displaySyncEnabled = useVsync;  // TODO: need solution for 10.12 or lower
    // fMetalLayer.layoutManager = [CAConstraintLayoutManager layoutManager];
    // fMetalLayer.autoresizingMask = kCALayerHeightSizable | kCALayerWidthSizable;
    // fMetalLayer.contentsGravity = kCAGravityTopLeft;
    // fMetalLayer.magnificationFilter = kCAFilterNearest;
    // fMetalLayer.framebufferOnly = false;
    // NSColorSpace* cs = fMainView.window.colorSpace;
    // fMetalLayer.colorspace = cs.CGColorSpace;

    // fMainView.layer = fMetalLayer;
    // fMainView.wantsLayer = YES;

    return true;
}

void GraphiteDawnMetalWindowContext_mac::onDestroyContext() {
    // fMainView.layer = nil;
    // fMainView.wantsLayer = NO;
}

void GraphiteDawnMetalWindowContext_mac::onSwapBuffers() {
}

DawnSwapChainImplementation GraphiteDawnMetalWindowContext_mac::createSwapChainImplementation(
    int width, int height, const DisplayParams& params) {
    return {};
}

void GraphiteDawnMetalWindowContext_mac::resize(int w, int h) {
    // CGFloat backingScaleFactor = sk_app::GetBackingScaleFactor(fMainView);
    // CGSize backingSize = fMainView.bounds.size;
    // backingSize.width *= backingScaleFactor;
    // backingSize.height *= backingScaleFactor;

    // fMetalLayer.drawableSize = backingSize;
    // fMetalLayer.contentsScale = backingScaleFactor;

    // fWidth = backingSize.width;
    // fHeight = backingSize.height;
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

std::unique_ptr<WindowContext> MakeGraphiteDawnMetalForMac(const MacWindowInfo& info,
                                                       const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new GraphiteDawnMetalWindowContext_mac(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
