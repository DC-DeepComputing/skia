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
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/GraphicsPipeline.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/Texture.h"

#if GRAPHITE_TEST_UTILS
// set to 1 if you want to do GPU capture of the commandBuffer
#define CAPTURE_COMMANDBUFFER 0
#endif

using namespace skgpu;

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
    };
    TextureInfo textureInfo(mtlTextureInfo);
#else
    TextureInfo textureInfo;
#endif

    sk_sp<Texture> texture = gpu->resourceProvider()->findOrCreateTexture(textureSize,
                                                                          textureInfo);
    REPORTER_ASSERT(reporter, texture);



    RenderPassDesc renderPassDesc = {};
    renderPassDesc.fColorAttachment.fTexture = texture;
    renderPassDesc.fColorAttachment.fLoadOp = LoadOp::kClear;
    renderPassDesc.fColorAttachment.fStoreOp = StoreOp::kStore;
    renderPassDesc.fClearColor = { 1, 0, 0, 1 }; // red

    commandBuffer->beginRenderPass(renderPassDesc);

    // Uniform buffer for kRectangle and kVertexTris vertex shaders
    struct VertexUniformData {
        SkPoint fScale;
        SkPoint fTranslate;
    };
    sk_sp<Buffer> vertexUniformBuffer = gpu->resourceProvider()->findOrCreateBuffer(
        2*sizeof(VertexUniformData), BufferType::kUniform, PrioritizeGpuReads::kNo);
    size_t vertexUniformOffset = 0;
    VertexUniformData* vertexUniforms = (VertexUniformData*) vertexUniformBuffer->map();

    // Uniform buffer for kSolidColor shader combination
    // TODO: This is simple enough I've hard-coded it, but should be using ExtractCombo?
    struct ShadingUniformData {
        SkColor4f fColor;
    };
    sk_sp<Buffer> shadingUniformBuffer = gpu->resourceProvider()->findOrCreateBuffer(
        4*sizeof(ShadingUniformData), BufferType::kUniform, PrioritizeGpuReads::kNo);
    size_t shadingUniformOffset = 0;
    ShadingUniformData* shadingUniforms = (ShadingUniformData*) shadingUniformBuffer->map();


    // Draw blue rectangle over entire rendertarget (which was red)
    GraphicsPipelineDesc pipelineDesc;
    pipelineDesc.setProgram(TestVertexShader::kUnitRectangle, {ShaderCombo::ShaderType::kSolidColor});
    auto graphicsPipeline = gpu->resourceProvider()->findOrCreateGraphicsPipeline(pipelineDesc);
    commandBuffer->bindGraphicsPipeline(std::move(graphicsPipeline));
    commandBuffer->bindUniformBuffer(kFirstSet, nullptr, 0);
    shadingUniforms->fColor = SkColors::kBlue;
    commandBuffer->bindUniformBuffer(kSecondSet, shadingUniformBuffer, shadingUniformOffset);
    shadingUniformOffset += sizeof(ShadingUniformData);
    shadingUniforms++;
    commandBuffer->draw(PrimitiveType::kTriangleStrip, 0, 4);

    // Draw inset yellow rectangle using uniforms
    pipelineDesc.setProgram(TestVertexShader::kRectangle, {ShaderCombo::ShaderType::kSolidColor});
    graphicsPipeline = gpu->resourceProvider()->findOrCreateGraphicsPipeline(pipelineDesc);
    commandBuffer->bindGraphicsPipeline(std::move(graphicsPipeline));
    vertexUniforms->fScale = SkPoint({1.8, 1.8});
    vertexUniforms->fTranslate = SkPoint({-0.9, -0.9});
    commandBuffer->bindUniformBuffer(kFirstSet, vertexUniforms, vertexUniformOffset);
    vertexUniformOffset += sizeof(VertexUniformData);
    vertexUniforms++;
    shadingUniforms->fColor = SkColors::kYellow;
    commandBuffer->bindUniformBuffer(kSecondSet, shadingUniformBuffer, shadingUniformOffset);
    shadingUniformOffset += sizeof(ShadingUniformData);
    shadingUniforms++;
    commandBuffer->draw(PrimitiveType::kTriangleStrip, 0, 4);

    // Draw inset magenta rectangle with triangles in vertex buffer
    pipelineDesc.setProgram(TestVertexShader::kVertexTris, {ShaderCombo::ShaderType::kSolidColor});
    graphicsPipeline = gpu->resourceProvider()->findOrCreateGraphicsPipeline(pipelineDesc);
    commandBuffer->bindGraphicsPipeline(std::move(graphicsPipeline));

    struct VertexData {
        SkPoint fPosition;
    };
    sk_sp<Buffer> vertexBuffer = gpu->resourceProvider()->findOrCreateBuffer(
            4*sizeof(VertexData), BufferType::kVertex, PrioritizeGpuReads::kNo);
    sk_sp<Buffer> indexBuffer = gpu->resourceProvider()->findOrCreateBuffer(
            6*sizeof(uint16_t), BufferType::kIndex, PrioritizeGpuReads::kNo);
    auto vertices = (VertexData*)vertexBuffer->map();
    vertices[0].fPosition = SkPoint({0.25f, 0.25f});
    vertices[1].fPosition = SkPoint({0.25f, 0.75f});
    vertices[2].fPosition = SkPoint({0.75f, 0.25f});
    vertices[3].fPosition = SkPoint({0.75f, 0.75f});
    vertexBuffer->unmap();
    auto indices = (uint16_t*)indexBuffer->map();
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 2;
    indices[4] = 1;
    indices[5] = 3;
    indexBuffer->unmap();
    commandBuffer->bindVertexBuffers(vertexBuffer, nullptr);
    commandBuffer->bindIndexBuffer(indexBuffer, 0);
    commandBuffer->bindUniformBuffer(kFirstSet, nullptr, 0);
    shadingUniforms->fColor = SkColors::kMagenta;
    commandBuffer->bindUniformBuffer(kSecondSet, shadingUniformBuffer, shadingUniformOffset);
    shadingUniformOffset += sizeof(ShadingUniformData);
    shadingUniforms++;
    commandBuffer->drawIndexed(PrimitiveType::kTriangles, 0, 6, 0);

    // draw rects using instance buffer
    pipelineDesc.setProgram(TestVertexShader::kInstanceRectangle, {ShaderCombo::ShaderType::kSolidColor});
    graphicsPipeline = gpu->resourceProvider()->findOrCreateGraphicsPipeline(pipelineDesc);
    commandBuffer->bindGraphicsPipeline(std::move(graphicsPipeline));

    struct InstanceData {
        SkPoint fPosition;
        SkPoint fDims;
    };
    sk_sp<Buffer> instanceBuffer = gpu->resourceProvider()->findOrCreateBuffer(
            2*sizeof(InstanceData), BufferType::kVertex, PrioritizeGpuReads::kNo);
    auto instances = (InstanceData*)instanceBuffer->map();
    instances[0].fPosition = SkPoint({-0.4, -0.4});
    instances[0].fDims = SkPoint({0.4, 0.4});
    instances[1].fPosition = SkPoint({0, 0});
    instances[1].fDims = SkPoint({0.25, 0.25});
    instanceBuffer->unmap();
    commandBuffer->bindVertexBuffers(nullptr, instanceBuffer);
//    commandBuffer->drawInstanced(PrimitiveType::kTriangleStrip, 0, 4, 0, 2);

    commandBuffer->bindUniformBuffer(kFirstSet, nullptr, 0);
    shadingUniforms->fColor = SkColors::kCyan;
    commandBuffer->bindUniformBuffer(kSecondSet, shadingUniformBuffer, shadingUniformOffset);
    shadingUniformOffset += sizeof(ShadingUniformData);
    shadingUniforms++;

    commandBuffer->drawIndexedInstanced(PrimitiveType::kTriangles, 0, 6, 0, 0, 2);

    commandBuffer->endRenderPass();

    vertexUniformBuffer->unmap();
    shadingUniformBuffer->unmap();

    // Do readback

    // TODO: add 4-byte transfer buffer alignment for Mac to Caps
    //       add bpp to Caps
    size_t rowBytes = 4*kTextureWidth;
    size_t bufferSize = rowBytes*kTextureHeight;
    sk_sp<Buffer> copyBuffer = gpu->resourceProvider()->findOrCreateBuffer(
            bufferSize, BufferType::kXferGpuToCpu, PrioritizeGpuReads::kNo);
    REPORTER_ASSERT(reporter, copyBuffer);
    SkIRect srcRect = { 0, 0, kTextureWidth, kTextureHeight };
    commandBuffer->copyTextureToBuffer(texture, srcRect, copyBuffer, 0, rowBytes);

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
