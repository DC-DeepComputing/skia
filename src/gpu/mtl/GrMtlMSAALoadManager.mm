/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlMSAALoadManager.h"

#include "include/gpu/GrDirectContext.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/mtl/GrMtlGpu.h"

#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlCommandBuffer.h"
//#include "src/gpu/GrResourceProvider.h"
//#include "src/gpu/Mtl/GrMtlImageView.h"
#include "src/gpu/mtl/GrMtlPipeline.h"
#include "src/gpu/mtl/GrMtlRenderCommandEncoder.h"
//#include "src/gpu/mtl/GrMtlRenderTarget.h"
//#include "src/gpu/mtl/GrMtlResourceProvider.h"
//#include "src/gpu/mtl/GrMtlUtil.h"

GrMtlMSAALoadManager::GrMtlMSAALoadManager() {}

GrMtlMSAALoadManager::~GrMtlMSAALoadManager() {}

bool GrMtlMSAALoadManager::createMSAALoadProgram(GrMtlGpu* gpu, GrMtlAttachment* src) {
    TRACE_EVENT0("skia", TRACE_FUNC);

    SkSL::String shaderText;
    shaderText.append(
            "#include <metal_stdlib>\n"
            "#include <simd/simd.h>\n"
            "using namespace metal;\n"
            "\n"
            "typedef struct {\n"
            "    float2 position;\n"
            "} VertexInput;\n"
            "\n"
            "typedef struct {\n"
            "    float4 position [[position]];\n"
            "} VertexOutput;\n"
            "\n"
            "typedef struct {\n"
            "    float4 uPosXform;\n"
            "    uint2 uTextureSize;\n"
            "} VertexUniforms;\n"
            "\n"
            "vertex VertexOutput vertexMain(constant VertexUniforms& uniforms [[buffer(0)]],\n"
            "                               constant VertexInput* vertices [[buffer(1)]],\n"
            "                               uint vertexID [[vertex_id]]) {\n"
            "    VertexOutput out;\n"
            "    float2 position = vertices[vertexID].position;\n"
            "    out.position.xy = position * uniforms.uPosXform.xy + uniforms.uPosXform.zw;\n"
            "    out.position.zw = float2(0.0, 1.0);\n"
            "    return out;\n"
            "}\n"
            "\n"
            "fragment float4 fragmentMain(VertexOutput in [[stage_in]],\n"
            "                             texture2d<half> colorMap [[texture(0)]]) {\n"
            "    uint2 coords = uint2(in.position.x, in.position.y);"
            "    half4 colorSample   = colorMap.read(coords);\n"
            "    return float4(colorSample);\n"
            "}"
    );

    auto errorHandler = gpu->getContext()->priv().getShaderErrorHandler();
    fShaderLibrary = GrCompileMtlShaderLibrary(gpu, shaderText, errorHandler);
    if (!fShaderLibrary) {
        this->destroyResources();
        return false;
    }

    SkPoint vertices[6] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 0},
        {0, 1},
        {1, 1},
    };
    fVertexBuffer = gpu->createBuffer(
            sizeof(vertices), GrGpuBufferType::kVertex, kStatic_GrAccessPattern, vertices);

    fVertexDescriptor = [[MTLVertexDescriptor alloc] init];
    auto mtlAttribute = fVertexDescriptor.attributes[0];
    mtlAttribute.format = MTLVertexFormatFloat2;
    mtlAttribute.offset = 0;
    mtlAttribute.bufferIndex = 1;

    auto vertexBufferLayout = fVertexDescriptor.layouts[1];
    vertexBufferLayout.stepFunction = MTLVertexStepFunctionPerVertex;
    vertexBufferLayout.stepRate = 1;
    vertexBufferLayout.stride = sizeof(SkPoint);

    auto desc = [[MTLTextureDescriptor alloc] init];
    desc.textureType = MTLTextureType2D;
    desc.pixelFormat = MTLPixelFormatRGBA8Unorm;
    desc.width = src->width();
    desc.height = src->height();
    desc.depth = 1;
    desc.mipmapLevelCount = 1;
    desc.sampleCount = 1;
    desc.arrayLength = 1;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        desc.usage = MTLTextureUsageShaderRead;
        desc.storageMode = MTLStorageModeShared;
    }
    fTestTexture = [gpu->device() newTextureWithDescriptor:desc];

    unsigned int width = src->width();
    unsigned int height = src->height();
    size_t datasize = width * height * sizeof(uint32_t);
    uint32_t* data = new uint32_t[datasize];
    for (size_t j = 0; j < height; ++j) {
        for (size_t i = 0; i < width; ++i) {
            if ((i/16) & 0x1) {
                data[i + j*width] = 0xff0000ff;
            } else {
                data[i + j*width] = 0x00ff00ff;
            }
        }
    }
    [fTestTexture replaceRegion:MTLRegionMake2D(0, 0, width, height)
                    mipmapLevel:0
                      withBytes:data
                    bytesPerRow:width*sizeof(uint32_t)];

    delete [] data;

    return true;
}

bool GrMtlMSAALoadManager::loadMSAAFromResolve(GrMtlGpu* gpu,
                                               GrAttachment* dst,
                                               GrMtlAttachment* src,
                                               const SkIRect& rect) {
    if (!dst) {
        return false;
    }
//    if (!src || src->framebufferOnly()) {
//        return false;
//    }

    if (!fShaderLibrary) {
        if (!this->createMSAALoadProgram(gpu, src)) {
            SkDebugf("Failed to create copy program.\n");
            return false;
        }
    }

    GrMtlAttachment* mtlDst = static_cast<GrMtlAttachment*>(dst);

    // Set up pipeline
    //*** use resource provider to create this
    auto pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];

    pipelineDescriptor.vertexFunction =
            [fShaderLibrary newFunctionWithName: @"vertexMain"];
    pipelineDescriptor.fragmentFunction =
            [fShaderLibrary newFunctionWithName: @"fragmentMain"];

    pipelineDescriptor.vertexDescriptor = fVertexDescriptor;

    auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

    mtlColorAttachment.pixelFormat = mtlDst->mtlFormat();
    mtlColorAttachment.blendingEnabled = FALSE;
    mtlColorAttachment.writeMask = MTLColorWriteMaskAll;

    pipelineDescriptor.colorAttachments[0] = mtlColorAttachment;

    pipelineDescriptor.sampleCount = dst->numSamples();
    // probably don't need this
    pipelineDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;

    NSError* error;
    auto pso =
            [gpu->device() newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                                          error: &error];
    if (!pso) {
        SkDebugf("Error creating pipeline: %s\n",
                 [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
    }

    auto renderPipeline = GrMtlRenderPipeline::Make(pso);

    // Set up rendercommandencoder
    auto renderPassDesc = [MTLRenderPassDescriptor new];
    auto colorAttachment = renderPassDesc.colorAttachments[0];
    colorAttachment.texture = mtlDst->mtlTexture();
    colorAttachment.loadAction = MTLLoadActionDontCare;
    //*** is this what we want?
    colorAttachment.storeAction = MTLStoreActionMultisampleResolve;
    colorAttachment.resolveTexture = src->mtlTexture();

    auto renderCmdEncoder =
                gpu->commandBuffer()->getRenderCommandEncoder(renderPassDesc, nullptr, nullptr);

    // Bind pipeline
    renderCmdEncoder->setRenderPipelineState(pso);
    gpu->commandBuffer()->addResource(std::move(renderPipeline));

    // Bind src as input texture
    renderCmdEncoder->setFragmentTexture(src->mtlTexture(), 0);
    // No sampler needed
    gpu->commandBuffer()->addGrSurface(sk_ref_sp<GrSurface>(src));

    // Scissor and viewport should default to size of color attachment

    // Update and bind uniform data
    int w = rect.width();
    int h = rect.height();

    // dst rect edges in NDC (-1 to 1)
    int dw = dst->width();
    int dh = dst->height();
    float dx0 = 2.f * rect.fLeft / dw - 1.f;
    float dx1 = 2.f * (rect.fLeft + w) / dw - 1.f;
    float dy0 = 2.f * rect.fTop / dh - 1.f;
    float dy1 = 2.f * (rect.fTop + h) / dh - 1.f;

    struct {
        float posXform[4];
        int textureSize[2];
        int pad[2];
    } uniData = {{2, 2, -1, -1}, {dw, dh}, {0, 0}};
    //{{dx1 - dx0, dy1 - dy0, dx0, dy0}, {dw, dh}, {0, 0}};

    constexpr size_t uniformSize = 32;
    if (@available(macOS 10.11, iOS 8.3, *)) {
        SkASSERT(uniformSize <= gpu->caps()->maxPushConstantsSize());
        renderCmdEncoder->setVertexBytes(&uniData, uniformSize, 0);
    } else {
        // upload the data
        GrRingBuffer::Slice slice = gpu->uniformsRingBuffer()->suballocate(uniformSize);
        GrMtlBuffer* buffer = (GrMtlBuffer*) slice.fBuffer;
        char* destPtr = static_cast<char*>(slice.fBuffer->map()) + slice.fOffset;
        memcpy(destPtr, &uniData, uniformSize);

        renderCmdEncoder->setVertexBuffer(buffer->mtlBuffer(), slice.fOffset, 0);
    }
    // set vertex buffer
    auto mtlVertexBuffer = (GrMtlBuffer*)(fVertexBuffer.get());
    renderCmdEncoder->setVertexBuffer(mtlVertexBuffer->mtlBuffer(), 0, 1);

    renderCmdEncoder->drawPrimitives(MTLPrimitiveTypeTriangle, 0, 6, 1, 0);

    return true;
}

void GrMtlMSAALoadManager::destroyResources() {
    fShaderLibrary = nil;
    fVertexBuffer.reset();
    fVertexDescriptor = nil;
}

