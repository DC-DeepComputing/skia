/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnSharedContext.h"

#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

namespace skgpu::graphite {
namespace {

wgpu::ShaderModule CreateNoopFragment(const wgpu::Device& device) {
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.code =
            "@fragment\n"
            "fn main() {}\n";
    wgpu::ShaderModuleDescriptor smDesc;
    smDesc.nextInChain = &wgslDesc;
    auto fsModule = device.CreateShaderModule(&smDesc);
    return fsModule;
}

wgpu::BindGroupLayout CreateUniformBuffersBindGroupLayout(const wgpu::Device& device,
                                                          const Caps& caps) {
    std::array<wgpu::BindGroupLayoutEntry, 3> entries;
    entries[0].binding = DawnGraphicsPipeline::kIntrinsicUniformBufferIndex;
    entries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[0].buffer.type = wgpu::BufferBindingType::Uniform;
    entries[0].buffer.hasDynamicOffset = true;
    entries[0].buffer.minBindingSize = 0;

    entries[1].binding = DawnGraphicsPipeline::kRenderStepUniformBufferIndex;
    entries[1].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entries[1].buffer.type = caps.storageBufferPreferred()
                                     ? wgpu::BufferBindingType::ReadOnlyStorage
                                     : wgpu::BufferBindingType::Uniform;
    entries[1].buffer.hasDynamicOffset = true;
    entries[1].buffer.minBindingSize = 0;

    entries[2].binding = DawnGraphicsPipeline::kPaintUniformBufferIndex;
    entries[2].visibility = wgpu::ShaderStage::Fragment;
    entries[2].buffer.type = caps.storageBufferPreferred()
                                     ? wgpu::BufferBindingType::ReadOnlyStorage
                                     : wgpu::BufferBindingType::Uniform;
    entries[2].buffer.hasDynamicOffset = true;
    entries[2].buffer.minBindingSize = 0;

    wgpu::BindGroupLayoutDescriptor groupLayoutDesc;
#if defined(SK_DEBUG)
    groupLayoutDesc.label = "Uniform buffers bind group layout";
#endif

    groupLayoutDesc.entryCount = entries.size();
    groupLayoutDesc.entries = entries.data();
    return device.CreateBindGroupLayout(&groupLayoutDesc);
}

wgpu::BindGroupLayout CreateSingleTextureSamplerBindGroupLayout(const wgpu::Device& device) {
    std::array<wgpu::BindGroupLayoutEntry, 2> entries;

    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Fragment;
    entries[0].sampler.type = wgpu::SamplerBindingType::Filtering;

    entries[1].binding = 1;
    entries[1].visibility = wgpu::ShaderStage::Fragment;
    entries[1].texture.sampleType = wgpu::TextureSampleType::Float;
    entries[1].texture.viewDimension = wgpu::TextureViewDimension::e2D;
    entries[1].texture.multisampled = false;

    wgpu::BindGroupLayoutDescriptor groupLayoutDesc;
#if defined(SK_DEBUG)
    groupLayoutDesc.label = "Single texture + sampler bind group layout";
#endif

    groupLayoutDesc.entryCount = entries.size();
    groupLayoutDesc.entries = entries.data();
    return device.CreateBindGroupLayout(&groupLayoutDesc);
}

wgpu::Buffer CreateNullBuffer(const wgpu::Device& device) {
    wgpu::BufferDescriptor desc;
#if defined(SK_DEBUG)
    desc.label = "UnusedBufferSlot";
#endif
    desc.usage =
            wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage;
    desc.size = DawnCaps::kBufferBindingSizeAlignment;
    desc.mappedAtCreation = false;

    return device.CreateBuffer(&desc);
}
}

sk_sp<SharedContext> DawnSharedContext::Make(const DawnBackendContext& backendContext,
                                             const ContextOptions& options) {
    if (!backendContext.fDevice || !backendContext.fQueue) {
        return {};
    }

    auto noopFragment = CreateNoopFragment(backendContext.fDevice);
    if (!noopFragment) {
        return {};
    }

    auto caps = std::make_unique<const DawnCaps>(backendContext, options);

    return sk_sp<SharedContext>(new DawnSharedContext(backendContext,
                                                      std::move(caps),
                                                      std::move(noopFragment)));
}

DawnSharedContext::DawnSharedContext(const DawnBackendContext& backendContext,
                                     std::unique_ptr<const DawnCaps> caps,
                                     wgpu::ShaderModule noopFragment)
        : skgpu::graphite::SharedContext(std::move(caps), BackendApi::kDawn)
        , fInstance(backendContext.fInstance)
        , fDevice(backendContext.fDevice)
        , fQueue(backendContext.fQueue)
        , fTick(backendContext.fTick)
        , fNoopFragment(std::move(noopFragment)) {
    fNullBuffer = CreateNullBuffer(fDevice);
    fUniformBuffersBindGroupLayout = CreateUniformBuffersBindGroupLayout(fDevice, *this->caps());
    fSingleTextureSamplerBindGroupLayout = CreateSingleTextureSamplerBindGroupLayout(fDevice);
}

DawnSharedContext::~DawnSharedContext() {
    // need to clear out resources before any allocator is removed
    this->globalCache()->deleteResources();
}

std::unique_ptr<ResourceProvider> DawnSharedContext::makeResourceProvider(
        SingleOwner* singleOwner,
        uint32_t recorderID,
        size_t resourceBudget) {
    return std::unique_ptr<ResourceProvider>(new DawnResourceProvider(this,
                                                                      singleOwner,
                                                                      recorderID,
                                                                      resourceBudget));
}

} // namespace skgpu::graphite
