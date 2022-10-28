/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/dawn/GraphiteDawnTestContext.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"

#include "dawn/dawn_proc.h"

namespace skiatest::graphite {

std::unique_ptr<GraphiteTestContext> DawnTestContext::Make() {
    auto instance = std::make_unique<dawn::native::Instance>();

    instance->DiscoverDefaultAdapters();
    DawnProcTable backendProcs = dawn::native::GetProcs();
    dawnProcSetProcs(&backendProcs);

    std::vector<dawn::native::Adapter> adapters = instance->GetAdapters();
    SkASSERT(!adapters.empty());
    // Sort adapters by adapterType(DiscreteGPU, IntegratedGPU, CPU) and
    // backendType(WebGPU, D3D11, D3D12, Metal, Vulkan, OpenGL, OpenGLES).
    std::sort(
        adapters.begin(), adapters.end(),
        [](dawn::native::Adapter a, dawn::native::Adapter b) {
            wgpu::AdapterProperties propA;
            wgpu::AdapterProperties propB;
            a.GetProperties(&propA);
            b.GetProperties(&propB);
            return std::tuple(propA.adapterType, propA.backendType) <
                   std::tuple(propB.adapterType, propB.backendType);
        });

    auto adapter = adapters.front();
#if defined(SK_DEBUG)
    wgpu::AdapterProperties properties;
    adapter.GetProperties(&properties);
    SkDebugf("GPU: %s\nDriver: %s\n", properties.name, properties.driverDescription);
#endif

    std::array<wgpu::FeatureName, 2> features = {
        wgpu::FeatureName::DepthClipControl,
        wgpu::FeatureName::Depth32FloatStencil8,
    };
    wgpu::DeviceDescriptor desc;
    desc.requiredFeaturesCount = features.size();
    desc.requiredFeatures = features.data();

#if !defined(SK_DEBUG)
        wgpu::DawnTogglesDeviceDescriptor togglesDeviceDesc;
        std::array<const char*, 1> toggles = {
            "skip_validation",
        };
        togglesDeviceDesc.forceEnabledTogglesCount = toggles.size();
        togglesDeviceDesc.forceEnabledToggles = toggles.data();
        desc.nextInChain = &togglesDeviceDesc;
#endif
    auto device = wgpu::Device::Acquire(adapter.CreateDevice(&desc));
    SkASSERT(device);
    device.SetUncapturedErrorCallback(
            [](WGPUErrorType type, const char* message, void*) {
                SkDebugf("Device error: %s\n", message);
                SkASSERT(false);
            },
            0);

    skgpu::graphite::DawnBackendContext backendContext;
    backendContext.fDevice = device;
    backendContext.fQueue = device.GetQueue();
    return std::unique_ptr<GraphiteTestContext>(new DawnTestContext(backendContext, std::move(instance)));
}

std::unique_ptr<skgpu::graphite::Context> DawnTestContext::makeContext() {
    return skgpu::graphite::Context::MakeDawn(fBackendContext,
                                              skgpu::graphite::ContextOptions{});
}

}  // namespace skiatest::graphite

