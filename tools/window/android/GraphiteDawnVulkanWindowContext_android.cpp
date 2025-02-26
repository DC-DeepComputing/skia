/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
// Important to put this first because webgpu_cpp.h and X.h don't get along.
// Include these first, before X11 defines None, Success, Status etc.
#include "dawn/native/DawnNative.h"  // NO_G3_REWRITE
#include "webgpu/webgpu_cpp.h"       // NO_G3_REWRITE

#include "tools/window/android/WindowContextFactory_android.h"

#include "tools/window/GraphiteDawnWindowContext.h"

using skwindow::DisplayParams;
using skwindow::internal::GraphiteDawnWindowContext;

namespace {

class GraphiteDawnVulkanWindowContext_android : public GraphiteDawnWindowContext {
public:
    GraphiteDawnVulkanWindowContext_android(ANativeWindow* window,
                                            std::unique_ptr<const DisplayParams> params);

    ~GraphiteDawnVulkanWindowContext_android() override;

    bool onInitializeContext() override;
    void onDestroyContext() override;
    void resize(int w, int h) override;

private:
    ANativeWindow* fWindow;
};

GraphiteDawnVulkanWindowContext_android::GraphiteDawnVulkanWindowContext_android(
        ANativeWindow* window, std::unique_ptr<const DisplayParams> params)
        : GraphiteDawnWindowContext(std::move(params), wgpu::TextureFormat::RGBA8Unorm)
        , fWindow(window) {
    SkDebugf("ANativeWindow_getFormat(window)=%u\n", ANativeWindow_getFormat(window));
    SkASSERT(ANativeWindow_getFormat(window) == AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM);
    this->initializeContext(ANativeWindow_getWidth(window), ANativeWindow_getHeight(window));
}

GraphiteDawnVulkanWindowContext_android::~GraphiteDawnVulkanWindowContext_android() {
    this->destroyContext();
}

bool GraphiteDawnVulkanWindowContext_android::onInitializeContext() {
    SkASSERT(!!fWindow);

    auto device = this->createDevice(wgpu::BackendType::Vulkan);
    if (!device) {
        SkASSERT(device);
        return false;
    }

    wgpu::SurfaceDescriptorFromAndroidNativeWindow surfaceChainedDesc;
    surfaceChainedDesc.window = fWindow;

    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &surfaceChainedDesc;

    auto surface = wgpu::Instance(fInstance->Get()).CreateSurface(&surfaceDesc);
    if (!surface) {
        SkASSERT(false);
        return false;
    }

    fDevice = std::move(device);
    fSurface = std::move(surface);
    configureSurface();

    return true;
}

void GraphiteDawnVulkanWindowContext_android::onDestroyContext() {}

void GraphiteDawnVulkanWindowContext_android::resize(int w, int h) { configureSurface(); }

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeGraphiteDawnVulkanForAndroid(
        ANativeWindow* window, std::unique_ptr<const DisplayParams> params) {
    std::unique_ptr<WindowContext> ctx(
            new GraphiteDawnVulkanWindowContext_android(window, std::move(params)));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow
