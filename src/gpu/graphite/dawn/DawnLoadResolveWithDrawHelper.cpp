/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnLoadResolveWithDrawHelper.h"

#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"
#include "src/gpu/graphite/dawn/DawnErrorChecker.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnTexture.h"

#include <limits>
#include <optional>
#include <sstream>

namespace skgpu::graphite {

namespace {

static constexpr char kCommonShaderText[] =
    "struct VertexOutput {"
        "@builtin(position) position: vec4f,"
    "};";

static constexpr char kVertexShaderVSText[] =
    "var<private> fullscreenTriPositions : array<vec2<f32>, 3> = array<vec2<f32>, 3>("
    "vec2(-1.0, -1.0), vec2(-1.0, 3.0), vec2(3.0, -1.0));"
    ""
    "@vertex\n"
    "fn main(@builtin(vertex_index) vertexIndex : u32,"
            "@builtin(instance_index) instanceIndex : u32) -> VertexOutput"
    "{"
        "var out: VertexOutput;"
        "out.position = vec4(fullscreenTriPositions[vertexIndex], 1.0, 1.0);"
        "return out;"
    "}";

static constexpr char kLoadSingleSampledFSText[] =
    "@group(0) @binding(0) var colorMap: texture_2d<f32>;"
    "@fragment\n"
    "fn main(input: VertexOutput) -> @location(0) vec4<f32> {"
        "let coords = vec2<i32>(i32(input.position.x), i32(input.position.y));"
        "return textureLoad(colorMap, coords, 0);"
    "}";

static constexpr char kLoadMultisampledFSText[] =
    "@group(0) @binding(0) var colorMap: texture_multisampled_2d<f32>;"
    "@fragment\n"
    "fn main(input: VertexOutput) -> @location(0) vec4<f32> {"
        "let coords = vec2<i32>(i32(input.position.x), i32(input.position.y));"
        "var sum = vec4f(0.0);"
        "let sampleCount = textureNumSamples(colorMap);"
        "for (var i: u32 = 0; i < sampleCount; i = i + 1) {"
            "sum += textureLoad(colorMap, coords, i);"
        "}"
        "return sum / f32(sampleCount);"
    "}";

std::string get_vs_src() {
    std::ostringstream ss;
    ss << kCommonShaderText;
    ss << kVertexShaderVSText;
    return ss.str();
}

std::string get_fs_src(int srcSampleCount) {
    std::ostringstream ss;
    ss << kCommonShaderText;

    if (srcSampleCount <= 1) {
        ss << kLoadSingleSampledFSText;
    } else {
        ss << kLoadMultisampledFSText;
    }
    return ss.str();
}

uint32_t make_blit_render_pipeline_key(DawnResourceProvider* resourceProvider,
                                       wgpu::TextureFormat dstFromat,
                                       int dstSampleCount,
                                       int srcSampleCount) {
    uint32_t key = 0;

    key = resourceProvider->dawnSharedContext()->dawnCaps()->makeDawnFormatKey(dstFromat);

    SkASSERT(srcSampleCount <= 8);
    SkASSERT(dstSampleCount <= 8);
    key |= srcSampleCount << 16;
    key |= dstSampleCount << 20;

    return key;
}

wgpu::ShaderModule create_shader_module(const wgpu::Device& device, const std::string& source) {
#if defined(__EMSCRIPTEN__)
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
#else
    wgpu::ShaderSourceWGSL wgslDesc;
#endif
    wgslDesc.code = source.c_str();
    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &wgslDesc;
    return device.CreateShaderModule(&descriptor);
}

wgpu::RenderPipeline find_or_create_blit_render_pipeline(DawnResourceProvider* resourceProvider,
                                                         wgpu::TextureFormat dstFormat,
                                                         int dstSampleCount,
                                                         int srcSampleCount) {
    uint32_t key = make_blit_render_pipeline_key(
            resourceProvider, dstFormat, dstSampleCount, srcSampleCount);

    auto pipeline = resourceProvider->findBlitWithDrawPipeline(key);
    if (pipeline) {
        return pipeline;
    }

    auto sharedContext = resourceProvider->dawnSharedContext();

    auto vsModule = create_shader_module(sharedContext->device(), get_vs_src());
    auto fsModule = create_shader_module(sharedContext->device(), get_fs_src(srcSampleCount));

    wgpu::RenderPipelineDescriptor descriptor = {};
    descriptor.label = "BlitWithDraw";
    descriptor.layout = nullptr;

    wgpu::ColorTargetState colorTarget;
    colorTarget.format = dstFormat;
    colorTarget.blend = nullptr;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragment;
    fragment.module = std::move(fsModule);
    fragment.entryPoint = "main";
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;
    descriptor.fragment = &fragment;

    descriptor.vertex.module = std::move(vsModule);
    descriptor.vertex.entryPoint = "main";

    descriptor.primitive.cullMode = wgpu::CullMode::None;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
    descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;

    descriptor.multisample.count = dstSampleCount;
    descriptor.multisample.mask = 0xFFFFFFFF;
    descriptor.multisample.alphaToCoverageEnabled = false;

    std::optional<DawnErrorChecker> errorChecker;
    if (sharedContext->dawnCaps()->allowScopedErrorChecks()) {
        errorChecker.emplace(sharedContext);
    }
    pipeline = sharedContext->device().CreateRenderPipeline(&descriptor);
    if (errorChecker.has_value() && errorChecker->popErrorScopes() != DawnErrorType::kNoError) {
        return nullptr;
    }

    resourceProvider->cacheBlitWithDrawPipeline(key, pipeline);

    return pipeline;
}

}  // namespace

DawnLoadResolveWithDrawHelper::DawnLoadResolveWithDrawHelper(const DawnTexture* msaaTexture,
                                                             const DawnTexture* resolveTexture,
                                                             SkIRect bounds)
        : fMsaaTexture(msaaTexture)
        , fResolveTexture(resolveTexture)
        , fBounds(std::move(bounds)) {}

bool DawnLoadResolveWithDrawHelper::loadFromResolveToMSAATexture(
        DawnResourceProvider* resourceProvider, const wgpu::CommandEncoder& encoder) {
    return this->doBlitWithDraw(resourceProvider,
                                encoder,
                                /*dstTexture=*/fMsaaTexture,
                                /*srcTexture=*/fResolveTexture,
                                fBounds);
}

bool DawnLoadResolveWithDrawHelper::storeFromMSAAToResolveTexture(
        DawnResourceProvider* resourceProvider, const wgpu::CommandEncoder& encoder) {
    return this->doBlitWithDraw(resourceProvider,
                                encoder,
                                /*dstTexture=*/fResolveTexture,
                                /*srcTexture=*/fMsaaTexture,
                                fBounds);
}

bool DawnLoadResolveWithDrawHelper::doBlitWithDraw(DawnResourceProvider* resourceProvider,
                                                   const wgpu::CommandEncoder& encoder,
                                                   const DawnTexture* dstTexture,
                                                   const DawnTexture* srcTexture,
                                                   const SkIRect& bounds) {
    const auto& srcTexInfo = srcTexture->textureInfo();
    const auto& dstDawnTexInfo = dstTexture->dawnTextureInfo();

    auto pipeline = find_or_create_blit_render_pipeline(resourceProvider,
                                                        dstDawnTexInfo.fFormat,
                                                        dstDawnTexInfo.fSampleCount,
                                                        srcTexInfo.numSamples());
    if (!pipeline) {
        SKGPU_LOG_E("Unable to create pipeline to blit with draw");
        return false;
    }

    wgpu::RenderPassColorAttachment wgpuIntermediateColorAttachment;
    wgpuIntermediateColorAttachment.loadOp = wgpu::LoadOp::Load;
    wgpuIntermediateColorAttachment.clearValue = {1, 1, 1, 1};
    wgpuIntermediateColorAttachment.storeOp = wgpu::StoreOp::Store;
    wgpuIntermediateColorAttachment.view = dstTexture->renderTextureView();

    wgpu::RenderPassDescriptor wgpuIntermediateRenderPassDesc;
    wgpuIntermediateRenderPassDesc.colorAttachmentCount = 1;
    wgpuIntermediateRenderPassDesc.colorAttachments = &wgpuIntermediateColorAttachment;

    auto renderEncoder = encoder.BeginRenderPass(&wgpuIntermediateRenderPassDesc);

    renderEncoder.SetPipeline(pipeline);

    // The load msaa pipeline takes no uniforms, no vertex/instance attributes and only uses
    // one texture that does not require a sampler.

    // TODO: b/260368758
    // cache single texture's bind group creation.
    wgpu::BindGroupEntry entry;
    entry.binding = 0;
    entry.textureView = srcTexture->renderTextureView();

    wgpu::BindGroupDescriptor desc;
    desc.layout = pipeline.GetBindGroupLayout(0);
    desc.entryCount = 1;
    desc.entries = &entry;

    auto bindGroup = resourceProvider->dawnSharedContext()->device().CreateBindGroup(&desc);

    renderEncoder.SetBindGroup(0, bindGroup);

    renderEncoder.SetScissorRect(bounds.x(), bounds.y(), bounds.width(), bounds.height());
    renderEncoder.SetViewport(bounds.x(), bounds.y(), bounds.width(), bounds.height(), 0, 1);

    // Fullscreen triangle.
    renderEncoder.Draw(3);

    renderEncoder.End();

    return true;
}

}  // namespace skgpu::graphite
