/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/src/ContextPriv.h"

#include "experimental/graphite/include/mtl/MtlTypes.h"
#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/DrawBufferManager.h"
#include "experimental/graphite/src/DrawWriter.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/GraphicsPipeline.h"
#include "experimental/graphite/src/Renderer.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/Texture.h"
#include "experimental/graphite/src/TextureProxy.h"
#include "experimental/graphite/src/geom/Shape.h"

#if GRAPHITE_TEST_UTILS
// set to 1 if you want to do GPU capture of the commandBuffer
#define CAPTURE_COMMANDBUFFER 0
#endif

using namespace skgpu;

namespace {

class FullscreenRectDraw final : public RenderStep {
public:
    ~FullscreenRectDraw() override {}

    static const RenderStep* Singleton() {
        static const FullscreenRectDraw kSingleton;
        return &kSingleton;
    }

    const char* name() const override { return "fullscreen-rect"; }

    SkString getVertexMSL() const override {
        // FIXME take from mtl graphics pipeline
    }

    void writeVertices(DrawWriter* writer, ResourceProvider* resourceProvider,
                       const Shape& shape) const override {
        SkASSERT(shape.isEmpty()); // this render step ignores shape
        writer->draw({}, {}, 4);
    }

    // FIXME writeUniforms() ...

private:
    FullscreenRectDraw() : RenderStep(Flags::kPerformsShading, PrimitiveType::kTriangleStrip,
                                      nullptr, 0, nullptr, 0) {}
};

class UniformRectDraw final : public RenderStep {
public:
    ~UniformRectDraw() override {}

    static const RenderStep* Singleton() {
        static const UniformRectDraw kSingleton;
        return &kSingleton;
    }

    const char* name() const override { return "fullscreen-rect"; }

    SkString getVertexMSL() const override {
        // FIXME take from mtl graphics pipeline
    }

    void writeVertices(DrawWriter* writer, ResourceProvider* resourceProvider,
                       const Shape& shape) const override {
        SkASSERT(shape.isRect());
        writer->draw({}, {}, 4);
    }

    // FIXME writeUniforms() ...

private:
    UniformRectDraw() : RenderStep(Flags::kPerformsShading, PrimitiveType::kTriangleStrip,
                                      nullptr, 0, nullptr, 0) {}
};

class TriangleRectDraw final : public RenderStep {
            skgpu::GraphicsPipelineDesc::Attribute vertexAttributes[1] = {
            { "position", VertexAttribType::kFloat2, SLType::kFloat2 }
        };
public:
    ~TriangleRectDraw() override {}

    static const RenderStep* Singleton() {
        static const TriangleRectDraw kSingleton;
        return &kSingleton;
    }

    const char* name() const override { return "fullscreen-rect"; }

    SkString getVertexMSL() const override {
        // FIXME take from mtl graphics pipeline
    }

    void writeVertices(DrawWriter* writer, ResourceProvider* resourceProvider,
                       const Shape& shape) const override {

        auto [vertexWriter, vertices] = bufferMgr.getVertexWriter(4 * pipelineDesc.vertexStride());
        vertexWriter << SkPoint{0.25f, 0.25f}
                     << SkPoint{0.25f, 0.75f}
                     << SkPoint{0.75f, 0.25f}
                     << SkPoint{0.75f, 0.75f};
        auto [indexWriter, indices] = bufferMgr.getIndexWriter(6 * sizeof(uint16_t));
        indexWriter << 0 << 1 << 2
                    << 2 << 1 << 3;

        drawWriter.draw(vertices, indices, 6);
    }

    // FIXME writeUniforms() ...

private:
    TriangleRectDraw() : RenderStep(Flags::kPerformsShading, PrimitiveType::kTriangles,
                                      nullptr, 0, nullptr, 0) {}
};

class InstanceRectDraw final : public RenderStep {
    skgpu::GraphicsPipelineDesc::Attribute instanceAttributes[2] = {
            { "position", VertexAttribType::kFloat2, SLType::kFloat2 },
            { "dims", VertexAttribType::kFloat2, SLType::kFloat2 },
        };
public:
    ~InstanceRectDraw() override {}

    static const RenderStep* Singleton() {
        static const InstanceRectDraw kSingleton;
        return &kSingleton;
    }

    const char* name() const override { return "fullscreen-rect"; }

    SkString getVertexMSL() const override {
        // FIXME take from mtl graphics pipeline
    }

    void writeVertices(DrawWriter* writer, ResourceProvider* resourceProvider,
                       const Shape& shape) const override {
        auto [indexWriter, indices] = bufferMgr.getIndexWriter(6 * sizeof(uint16_t));
        indexWriter << 0 << 1 << 2
                    << 2 << 1 << 3;

        drawWriter.setInstanceTemplate({}, indices, 6);
        auto instanceWriter = drawWriter.appendInstances(2);
        instanceWriter << SkPoint{-0.4f, -0.4f}  << SkPoint{0.4f, 0.4f}   << SkColors::kGreen
                       << SkPoint{0.f, 0.f}      << SkPoint{0.25f, 0.25f} << SkColors::kCyan;


        drawWriter.draw(vertices, indices, 6);
    }

    // FIXME writeUniforms() ...

private:
    InstanceRectDraw() : RenderStep(Flags::kPerformsShading, PrimitiveType::kTriangles,
                                      nullptr, 0, nullptr, 0) {}
};

};

/*
 * This is to test the various pieces of the CommandBuffer interface.
 */
DEF_GRAPHITE_TEST_FOR_CONTEXTS(CommandBufferTest, reporter, context) {
    constexpr int kTextureWidth = 1024;
    constexpr int kTextureHeight = 768;

    auto gpu = context->priv().gpu();
    REPORTER_ASSERT(reporter, gpu);

#if GRAPHITE_TEST_UTILS && CAPTURE_COMMANDBUFFER
    gpu->testingOnly_startCapture();
#endif
    auto commandBuffer = gpu->resourceProvider()->createCommandBuffer();

    SkISize textureSize = { kTextureWidth, kTextureHeight };
#ifdef SK_METAL
    skgpu::mtl::TextureInfo mtlTextureInfo = {
        1,
        1,
        70,     // MTLPixelFormatRGBA8Unorm
        0x0005, // MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead
        2,      // MTLStorageModePrivate
        false,  // framebufferOnly
    };
    TextureInfo textureInfo(mtlTextureInfo);
#else
    TextureInfo textureInfo;
#endif

    auto target = sk_sp<TextureProxy>(new TextureProxy(textureSize, textureInfo));
    REPORTER_ASSERT(reporter, target);

    RenderPassDesc renderPassDesc = {};
    renderPassDesc.fColorAttachment.fTextureProxy = target;
    renderPassDesc.fColorAttachment.fLoadOp = LoadOp::kClear;
    renderPassDesc.fColorAttachment.fStoreOp = StoreOp::kStore;
    renderPassDesc.fClearColor = { 1, 0, 0, 1 }; // red

    target->instantiate(gpu->resourceProvider());
    DrawBufferManager bufferMgr(gpu->resourceProvider(), 4);


    commandBuffer->beginRenderPass(renderPassDesc);
    {
        DrawWriter drawWriter(commandBuffer->asDrawDispatcher(), &bufferMgr);

        struct RectAndColor {
            SkRect    fRect;
            SkColor4f fColor;
        };

        auto draw = [&](const RenderStep* step, std::vector<RectAndColor> draws) {
            GraphicsPipelineDesc pipelineDesc;
            pipelineDesc.setRenderStep(step);
            drawWriter = drawWriter.forNewPipeline(step->primitiveType(), step->vertexStride(), step->instanceStride());
            auto pipeline = gpu->resourceProvider()->findOrCreateGraphicsPipeline(pipelineDesc);
            commandBuffer->bindGraphicsPipeline(std::move(pipeline));

            for (auto d : draws) {
                drawWriter = drawWriter.forDynamicStateChange();
                Shape shape(d.fRect);
                // have step write uniforms based on shape
                // bind uniforms to kRenderStep slot
                // write and bind solid color uniforms to kPaint slot
                step->writeVertices(&drawWriter, gpu->resourceProvider(), shape);
            }
        };

        // Shared uniform buffer
        // struct UniformData {
        //     SkPoint fScale;
        //     SkPoint fTranslate;
        //     SkColor4f fColor;
        // };
        // sk_sp<Buffer> uniformBuffer = gpu->resourceProvider()->findOrCreateBuffer(
        //         2*sizeof(UniformData), BufferType::kUniform, PrioritizeGpuReads::kNo);
        // size_t uniformOffset = 0;

        // Draw blue rectangle over entire rendertarget (which was red)
        draw(FullscreenRectDraw::Singleton(), {{SkRect::MakeEmpty(), SkColors::kBlue}});

        // Draw inset yellow rectangle using uniforms
        draw(UniformRectDraw::Singleton(), {{{-0.9f, -0.9f, 0.9f, 0.9f}, SkColors::kYellow}});
        // UniformData* uniforms = (UniformData*)uniformBuffer->map();
        // uniforms->fScale = SkPoint({1.8, 1.8});
        // uniforms->fTranslate = SkPoint({-0.9, -0.9});
        // uniforms->fColor = SkColors::kYellow;
        // commandBuffer->bindUniformBuffer(UniformSlot::kRenderStep, uniformBuffer, uniformOffset);


        // Draw inset magenta rectangle with triangles in vertex buffer
        draw(TriangleRectDraw::Singleton(), {{{-.5f, -.5f, .5f, .5f}, SkColors::kMagenta}});

        // Draw green and cyan rects using instance buffer
        draw(InstanceRectDraw::Singleton(),
             { {{0.5f, -0.5f, 0.5f, 0.5f}, SkColors::kGreen},
               {{0.f, 0.f, 0.25f, 0.25f}, SkColors::kCyan} });

        // uniformBuffer->unmap();
    }

    bufferMgr.transferToCommandBuffer(commandBuffer.get());
    commandBuffer->endRenderPass();

    // Do readback

    // TODO: add 4-byte transfer buffer alignment for Mac to Caps
    //       add bpp to Caps
    size_t rowBytes = 4*kTextureWidth;
    size_t bufferSize = rowBytes*kTextureHeight;
    sk_sp<Buffer> copyBuffer = gpu->resourceProvider()->findOrCreateBuffer(
            bufferSize, BufferType::kXferGpuToCpu, PrioritizeGpuReads::kNo);
    REPORTER_ASSERT(reporter, copyBuffer);
    SkIRect srcRect = { 0, 0, kTextureWidth, kTextureHeight };
    commandBuffer->copyTextureToBuffer(target->refTexture(), srcRect, copyBuffer, 0, rowBytes);

    bool result = gpu->submit(commandBuffer);
    REPORTER_ASSERT(reporter, result);

    gpu->checkForFinishedWork(skgpu::SyncToCpu::kYes);
    uint32_t* pixels = (uint32_t*)(copyBuffer->map());
    REPORTER_ASSERT(reporter, pixels[0] == 0xffff0000);
    REPORTER_ASSERT(reporter, pixels[51 + 38*kTextureWidth] == 0xff00ffff);
    REPORTER_ASSERT(reporter, pixels[256 + 192*kTextureWidth] == 0xffff00ff);
    copyBuffer->unmap();

#if GRAPHITE_TEST_UTILS && CAPTURE_COMMANDBUFFER
    gpu->testingOnly_endCapture();
#endif
}
