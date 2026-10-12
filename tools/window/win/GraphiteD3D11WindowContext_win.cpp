/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/win/WindowContextFactory_win.h"

#include "include/core/SkSurface.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/d3d11/D3D11BackendContext.h"
#include "include/gpu/graphite/d3d11/D3D11GraphiteTypes.h"
#include "tools/graphite/GraphiteToolUtils.h"
#include "tools/window/DisplayParams.h"
#include "tools/window/WindowContext.h"

#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <algorithm>

using namespace Microsoft::WRL;
using skwindow::DisplayParams;
using skwindow::WindowContext;

namespace {

class GraphiteD3D11WindowContext_win : public WindowContext {
public:
    GraphiteD3D11WindowContext_win(HWND hwnd, std::unique_ptr<const DisplayParams> params);
    ~GraphiteD3D11WindowContext_win() override;

    bool isValid() override { return fDevice != nullptr && fSwapChain != nullptr; }

    sk_sp<SkSurface> getBackbufferSurface() override;

    void resize(int w, int h) override;
    void setDisplayParams(std::unique_ptr<const DisplayParams> params) override;

protected:
    void onSwapBuffers() override;

private:
    void initializeContext();
    void destroyContext();
    void setupSurfaces(int width, int height);

    static constexpr int kNumBuffers = 2;

    HWND fWindow;
    ComPtr<ID3D11Device1> fDevice;
    ComPtr<ID3D11DeviceContext1> fDeviceContext;
    ComPtr<IDXGISwapChain1> fSwapChain;
    ComPtr<IDXGISwapChain3> fSwapChain3;
    ComPtr<ID3D11Texture2D> fBuffers[kNumBuffers];
    sk_sp<SkSurface> fSurfaces[kNumBuffers];
    int fBufferIndex = 0;
};

GraphiteD3D11WindowContext_win::GraphiteD3D11WindowContext_win(
        HWND hwnd, std::unique_ptr<const DisplayParams> params)
        : WindowContext(std::move(params))
        , fWindow(hwnd) {
    this->initializeContext();
}

GraphiteD3D11WindowContext_win::~GraphiteD3D11WindowContext_win() {
    this->destroyContext();
}

void GraphiteD3D11WindowContext_win::initializeContext() {
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(SK_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    D3D_FEATURE_LEVEL featureLevel;

    ComPtr<ID3D11Device> baseDevice;
    ComPtr<ID3D11DeviceContext> baseContext;

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &baseDevice,
        &featureLevel,
        &baseContext
    );

    if (FAILED(hr)) {
        creationFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &baseDevice,
            &featureLevel,
            &baseContext
        );
    }

    if (FAILED(hr)) {
        return;
    }

    if (FAILED(baseDevice.As(&fDevice)) || FAILED(baseContext.As(&fDeviceContext))) {
        return;
    }

    ComPtr<IDXGIDevice> dxgiDevice;
    ComPtr<IDXGIAdapter> dxgiAdapter;
    ComPtr<IDXGIFactory2> dxgiFactory;
    if (FAILED(baseDevice.As(&dxgiDevice)) ||
        FAILED(dxgiDevice->GetAdapter(&dxgiAdapter)) ||
        FAILED(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)))) {
        return;
    }

    RECT windowRect;
    GetClientRect(fWindow, &windowRect);
    int width = std::max(1, static_cast<int>(windowRect.right - windowRect.left));
    int height = std::max(1, static_cast<int>(windowRect.bottom - windowRect.top));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = kNumBuffers;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    hr = dxgiFactory->CreateSwapChainForHwnd(
        fDevice.Get(),
        fWindow,
        &swapChainDesc,
        nullptr,
        nullptr,
        &fSwapChain
    );

    if (FAILED(hr)) {
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        hr = dxgiFactory->CreateSwapChainForHwnd(
            fDevice.Get(),
            fWindow,
            &swapChainDesc,
            nullptr,
            nullptr,
            &fSwapChain
        );
        if (FAILED(hr)) {
            return;
        }
    }

    dxgiFactory->MakeWindowAssociation(fWindow, DXGI_MWA_NO_ALT_ENTER);
    fSwapChain.As(&fSwapChain3);

    ComPtr<IDXGIAdapter1> adapter1;
    dxgiAdapter.As(&adapter1);

    skgpu::graphite::D3D11BackendContext backendContext;
    backendContext.fAdapter = adapter1;
    backendContext.fDevice = fDevice;
    backendContext.fDeviceContext = fDeviceContext;

    skgpu::graphite::ContextOptions contextOptions;
    contextOptions.fRequireOrderedRecordings = true;

    fGraphiteContext = skgpu::graphite::ContextFactory::MakeD3D11(backendContext, contextOptions);
    if (!fGraphiteContext) {
        return;
    }
    fGraphiteRecorder = fGraphiteContext->makeRecorder(ToolUtils::CreateTestingRecorderOptions());

    fWidth = width;
    fHeight = height;

    this->setupSurfaces(width, height);
}

void GraphiteD3D11WindowContext_win::setupSurfaces(int width, int height) {
    skgpu::graphite::D3D11TextureInfo info;
    info.fFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    info.fViewFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    info.fBindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    info.fSampleCount = skgpu::graphite::SampleCount::k1;
    info.fMipmapped = skgpu::Mipmapped::kNo;
    info.fUsage = D3D11_USAGE_DEFAULT;

    for (int i = 0; i < kNumBuffers; ++i) {
        HRESULT hr = fSwapChain->GetBuffer(i, IID_PPV_ARGS(&fBuffers[i]));
        if (FAILED(hr)) {
            if (i > 0) {
                fBuffers[i] = fBuffers[0];
                fSurfaces[i] = fSurfaces[0];
                continue;
            }
            return;
        }

        auto backendTex = skgpu::graphite::BackendTextures::MakeD3D11(
            SkISize::Make(width, height), info, fBuffers[i].Get());

        fSurfaces[i] = SkSurfaces::WrapBackendTexture(
            this->graphiteRecorder(),
            backendTex,
            kRGBA_8888_SkColorType,
            fDisplayParams->colorSpace(),
            &fDisplayParams->surfaceProps());
    }

    if (fSwapChain3) {
        fBufferIndex = fSwapChain3->GetCurrentBackBufferIndex();
    } else {
        fBufferIndex = 0;
    }
}

void GraphiteD3D11WindowContext_win::destroyContext() {
    if (fGraphiteContext) {
        fGraphiteRecorder.reset();
        fGraphiteContext.reset();
    }
    for (int i = 0; i < kNumBuffers; ++i) {
        fSurfaces[i].reset();
        fBuffers[i].Reset();
    }
    fSwapChain3.Reset();
    fSwapChain.Reset();
    fDeviceContext.Reset();
    fDevice.Reset();
}

sk_sp<SkSurface> GraphiteD3D11WindowContext_win::getBackbufferSurface() {
    if (fSwapChain3) {
        fBufferIndex = fSwapChain3->GetCurrentBackBufferIndex();
    }
    if (fBufferIndex >= 0 && fBufferIndex < kNumBuffers) {
        return fSurfaces[fBufferIndex];
    }
    return fSurfaces[0];
}

void GraphiteD3D11WindowContext_win::onSwapBuffers() {
    this->submitToGpu();

    UINT syncInterval = fDisplayParams->disableVsync() ? 0 : 1;
    fSwapChain->Present(syncInterval, 0);

    if (fSwapChain3) {
        fBufferIndex = fSwapChain3->GetCurrentBackBufferIndex();
    } else {
        fBufferIndex = (fBufferIndex + 1) % kNumBuffers;
    }
}

void GraphiteD3D11WindowContext_win::resize(int w, int h) {
    if (!this->isValid()) {
        return;
    }

    this->submitToGpu();
    if (fGraphiteContext) {
        fGraphiteContext->submit(skgpu::graphite::SyncToCpu::kYes);
    }

    for (int i = 0; i < kNumBuffers; ++i) {
        fSurfaces[i].reset();
        fBuffers[i].Reset();
    }

    fSwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);

    this->setupSurfaces(w, h);
    fWidth = w;
    fHeight = h;
}

void GraphiteD3D11WindowContext_win::setDisplayParams(std::unique_ptr<const DisplayParams> params) {
    fDisplayParams = std::move(params);
    this->destroyContext();
    this->initializeContext();
}

}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeGraphiteD3D11ForWin(
        HWND hwnd, std::unique_ptr<const DisplayParams> params) {
    auto ctx = std::make_unique<GraphiteD3D11WindowContext_win>(hwnd, std::move(params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow
