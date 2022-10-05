/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GraphiteDawnWindowContext_DEFINED
#define GraphiteDawnWindowContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

#include "tools/sk_app/WindowContext.h"
#include "webgpu/webgpu_cpp.h"
#include "dawn/native/DawnNative.h"
#include "dawn/dawn_wsi.h"

namespace sk_app {

class GraphiteDawnWindowContext : public WindowContext {
public:
    GraphiteDawnWindowContext(const DisplayParams&, wgpu::TextureFormat swapChainFormat);
    ~GraphiteDawnWindowContext() override;
    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;
    bool isValid() override { return SkToBool(fDevice.Get()); }

    void resize(int w, int h) override;

    void setDisplayParams(const DisplayParams& params) override;

protected:
    bool isGpuContext() override { return true; }
    void initializeContext(int width, int height);
    wgpu::Device createDevice(wgpu::BackendType type);
    void destroyContext();

    virtual bool onInitializeContext() = 0;
    virtual void onDestroyContext() = 0;
    virtual void onSwapBuffers() = 0;
    virtual GrSurfaceOrigin getRTOrigin() const { return kTopLeft_GrSurfaceOrigin; }
    virtual DawnSwapChainImplementation createSwapChainImplementation( int width, int height,
        const DisplayParams& params) = 0;

    sk_sp<SkSurface>              fSurface;
    DawnSwapChainImplementation   fSwapChainImplementation;
    wgpu::TextureFormat           fSwapChainFormat;
    wgpu::SwapChain               fSwapChain;
    wgpu::Device                  fDevice;
    std::unique_ptr<dawn::native::Instance> fInstance;
};

}   // namespace sk_app

#endif
