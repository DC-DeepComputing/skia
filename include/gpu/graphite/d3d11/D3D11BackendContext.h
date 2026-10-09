/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_D3D11BackendContext_DEFINED
#define skgpu_graphite_D3D11BackendContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkAPI.h"

#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <wrl/client.h>
#include <memory>

namespace skgpu::graphite {

class Context;
struct ContextOptions;

struct SK_API D3D11BackendContext {
    Microsoft::WRL::ComPtr<IDXGIAdapter1> fAdapter;
    Microsoft::WRL::ComPtr<ID3D11Device1> fDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> fDeviceContext;
};

namespace ContextFactory {
SK_API std::unique_ptr<Context> MakeD3D11(const D3D11BackendContext&, const ContextOptions&);
} // namespace ContextFactory

} // namespace skgpu::graphite

#endif // skgpu_graphite_D3D11BackendContext_DEFINED
