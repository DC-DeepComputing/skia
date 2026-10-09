/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/d3d11/GraphiteD3D11TestContext.h"

#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "tools/gpu/ContextType.h"
#include "tools/graphite/TestOptions.h"

#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>

namespace skiatest::graphite {

D3D11TestContext::~D3D11TestContext() = default;

std::unique_ptr<GraphiteTestContext> D3D11TestContext::Make() {
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(SK_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    D3D_FEATURE_LEVEL featureLevel;

    Microsoft::WRL::ComPtr<ID3D11Device> baseDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> baseContext;

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

#if defined(SK_DEBUG)
    if (FAILED(hr)) {
        // Fall back to creating without debug flag if debug layer is not installed.
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
#endif

    if (FAILED(hr)) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D11Device1> device1;
    hr = baseDevice.As(&device1);
    if (FAILED(hr)) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> context1;
    hr = baseContext.As(&context1);
    if (FAILED(hr)) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    hr = baseDevice.As(&dxgiDevice);
    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    if (SUCCEEDED(hr)) {
        dxgiDevice->GetAdapter(&dxgiAdapter);
    }
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter1;
    if (dxgiAdapter) {
        dxgiAdapter.As(&adapter1);
    }

    skgpu::graphite::D3D11BackendContext backendContext;
    backendContext.fAdapter = adapter1;
    backendContext.fDevice = device1;
    backendContext.fDeviceContext = context1;

    return std::unique_ptr<GraphiteTestContext>(new D3D11TestContext(backendContext));
}

skgpu::ContextType D3D11TestContext::contextType() {
    return skgpu::ContextType::kD3D11;
}

std::unique_ptr<skgpu::graphite::Context> D3D11TestContext::makeContext(const TestOptions& options) {
    skiatest::graphite::TestOptions revisedOptions = options;
    revisedOptions.fOptionsPriv.fStoreContextRefInRecorder = true;
    return skgpu::graphite::ContextFactory::MakeD3D11(fBackendContext, revisedOptions.fContextOptions);
}

}  // namespace skiatest::graphite
