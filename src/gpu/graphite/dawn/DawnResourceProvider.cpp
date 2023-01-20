/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/dawn/DawnBuffer.h"
#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnSampler.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnTexture.h"

namespace skgpu::graphite {

namespace {
wgpu::ShaderModule create_shader_module(const wgpu::Device& device, const char* source) {
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.source = source;
    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &wgslDesc;
    return device.CreateShaderModule(&descriptor);
}

wgpu::RenderPipeline create_blit_render_pipeline(const wgpu::Device& device,
                                                 const char* label,
                                                 wgpu::ShaderModule vsModule,
                                                 wgpu::ShaderModule fsModule,
                                                 wgpu::TextureFormat renderPassColorFormat,
                                                 wgpu::TextureFormat renderPassDepthStencilFormat,
                                                 int numSamples) {
    wgpu::RenderPipelineDescriptor descriptor;
#if defined(SK_DEBUG)
    descriptor.label = label;
#endif
    descriptor.layout = nullptr;

    wgpu::ColorTargetState colorTarget;
    colorTarget.format = renderPassColorFormat;
    colorTarget.blend = nullptr;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::DepthStencilState depthStencil;
    if (renderPassDepthStencilFormat != wgpu::TextureFormat::Undefined) {
        depthStencil.format = renderPassDepthStencilFormat;
        depthStencil.depthWriteEnabled = false;
        depthStencil.depthCompare = wgpu::CompareFunction::Always;

        descriptor.depthStencil = &depthStencil;
    }

    wgpu::FragmentState fragment;
    fragment.module = std::move(fsModule);
    fragment.entryPoint = "main";
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;
    descriptor.fragment = &fragment;

    descriptor.vertex.module = std::move(vsModule);
    descriptor.vertex.entryPoint = "main";
    descriptor.vertex.constantCount = 0;
    descriptor.vertex.constants = nullptr;
    descriptor.vertex.bufferCount = 0;
    descriptor.vertex.buffers = nullptr;

    descriptor.primitive.frontFace = wgpu::FrontFace::CCW;
    descriptor.primitive.cullMode = wgpu::CullMode::None;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
    descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;

    descriptor.multisample.count = numSamples;
    descriptor.multisample.mask = 0xFFFFFFFF;
    descriptor.multisample.alphaToCoverageEnabled = false;

    return device.CreateRenderPipeline(&descriptor);
}
}

DawnResourceProvider::DawnResourceProvider(SharedContext* sharedContext,
                                           SingleOwner* singleOwner)
        : ResourceProvider(sharedContext, singleOwner) {}

DawnResourceProvider::~DawnResourceProvider() = default;

wgpu::RenderPipeline DawnResourceProvider::findOrCreateBlitWithDrawPipeline(
        const RenderPassDesc& renderPassDesc) {
    uint64_t renderPassKey =
            this->dawnSharedContext()->dawnCaps()->getRenderPassDescKey(renderPassDesc);
    wgpu::RenderPipeline pipeline = fBlitWithDrawPipelines[renderPassKey];
    if (!pipeline) {
        static constexpr char kVertexShaderText[] = R"(
            struct Rectangle {
                x : f32,
                y : f32,
                width : f32,
                height : f32,
            }
            struct VertexOutput {
                @builtin(position) position : vec4<f32>,
                @location(0) srcTexelCoord : vec2<f32>,
            }

            @group(0) @binding(0) var<uniform> srcRect: Rectangle;

            //
            // NDC space (Y up)
            //    (-1,3)
            //      |\
            //      | \
            //      |  \
            //(-1,1)|___\ (1,1)
            //      |   |\
            //      |   | \
            //      |   |  \
            //      |___|___\
            //(-1,-1) (1,-1) (3,-1)
            //

            //
            // texture space (Y down)
            //    (0,-1)
            //      |\
            //      | \
            //      |  \
            //(0,0) |___\ (1,0)
            //      |   |\
            //      |   | \
            //      |   |  \
            //      |___|___\
            //(0,1)   (1,1)  (2,1)
            //

            var<private> fullscreenTriPositions : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                vec2(-1.0, -1.0), vec2(-1.0, 3.0), vec2(3.0, -1.0));
            var<private> fullscreenTriTexcoords : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                vec2(0.0, 1.0), vec2(0.0, -1.0), vec2(2.0, 1.0));

            @vertex
            fn main(@builtin(vertex_index) vertexIndex : u32) -> VertexOutput {
                var output : VertexOutput;
                output.position = vec4(fullscreenTriPositions[vertexIndex], 1.0, 1.0);

                // For texel coords, we need to offset by srcRect's origin
                // and scale with srcRect's size.
                output.srcTexelCoord = vec2(srcRect.x, srcRect.y)
                    + fullscreenTriTexcoords[vertexIndex] * vec2(srcRect.width, srcRect.height);
                return output;
            }
        )";

        static constexpr char kFragmentShaderText[] = R"(
            @group(0) @binding(1) var colorMap: texture_2d<f32>;

            @fragment
            fn main(@location(0) srcTexelCoord : vec2<f32>) -> @location(0) vec4<f32> {
                var coords : vec2<i32> = vec2<i32>(i32(srcTexelCoord.x), i32(srcTexelCoord.y));
                return textureLoad(colorMap, coords, 0);
            }
        )";

        auto vsModule = create_shader_module(dawnSharedContext()->device(), kVertexShaderText);
        auto fsModule = create_shader_module(dawnSharedContext()->device(), kFragmentShaderText);

        pipeline = create_blit_render_pipeline(
                dawnSharedContext()->device(),
                /*label=*/"BlitWithDraw",
                std::move(vsModule),
                std::move(fsModule),
                /*renderPassColorFormat=*/
                renderPassDesc.fColorAttachment.fTextureInfo.dawnTextureSpec().fFormat,
                /*renderPassDepthStencilFormat=*/
                        renderPassDesc.fDepthStencilAttachment.fTextureInfo.isValid()
                        ? renderPassDesc.fDepthStencilAttachment.fTextureInfo.dawnTextureSpec()
                                  .fFormat
                        : wgpu::TextureFormat::Undefined,
                /*numSamples=*/renderPassDesc.fColorAttachment.fTextureInfo.numSamples());

        if (pipeline) {
            fBlitWithDrawPipelines.set(renderPassKey, pipeline);
        }
    }

    return pipeline;
}

sk_sp<Texture> DawnResourceProvider::createWrappedTexture(const BackendTexture& texture) {
    wgpu::Texture dawnTexture         = texture.getDawnTexture();
    wgpu::TextureView dawnTextureView = texture.getDawnTextureView();
    SkASSERT(!dawnTexture || !dawnTextureView);

    if (!dawnTexture && !dawnTextureView) {
        return {};
    }

    if (dawnTexture) {
        return DawnTexture::MakeWrapped(this->dawnSharedContext(),
                                        texture.dimensions(),
                                        texture.info(),
                                        std::move(dawnTexture));
    } else {
        return DawnTexture::MakeWrapped(this->dawnSharedContext(),
                                        texture.dimensions(),
                                        texture.info(),
                                        std::move(dawnTextureView));
    }
}

sk_sp<GraphicsPipeline> DawnResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    return DawnGraphicsPipeline::Make(this->dawnSharedContext(),
                                      this->skslCompiler(),
                                      runtimeDict,
                                      pipelineDesc,
                                      renderPassDesc);
}

sk_sp<ComputePipeline> DawnResourceProvider::createComputePipeline(const ComputePipelineDesc&) {
    SkASSERT(false);
    return nullptr;
}

sk_sp<Texture> DawnResourceProvider::createTexture(SkISize dimensions,
                                                   const TextureInfo& info,
                                                   skgpu::Budgeted budgeted) {
    return DawnTexture::Make(this->dawnSharedContext(), dimensions, info, budgeted);
}

sk_sp<Buffer> DawnResourceProvider::createBuffer(size_t size,
                                                 BufferType type,
                                                 PrioritizeGpuReads prioritizeGpuReads) {
    return DawnBuffer::Make(dawnSharedContext(), size, type, prioritizeGpuReads);
}

sk_sp<Sampler> DawnResourceProvider::createSampler(const SkSamplingOptions& options,
                                                   SkTileMode xTileMode,
                                                   SkTileMode yTileMode) {
    return DawnSampler::Make(dawnSharedContext(), options, xTileMode, yTileMode);
}

BackendTexture DawnResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                            const TextureInfo& info) {
    wgpu::Texture texture = DawnTexture::MakeDawnTexture(dawnSharedContext(), dimensions, info);
    if (!texture) {
        return {};
    }

    return BackendTexture(std::move(texture));
}

void DawnResourceProvider::onDeleteBackendTexture(BackendTexture& texture) {
    SkASSERT(texture.isValid());
    SkASSERT(texture.backend() == BackendApi::kDawn);

    // Nothing to be done here as all the the cleanup of Dawn's resources will be done inside
    // BackendTexture::~BackendTexture().
}

const DawnSharedContext* DawnResourceProvider::dawnSharedContext() const {
    return static_cast<const DawnSharedContext*>(fSharedContext);
}

} // namespace skgpu::graphite
