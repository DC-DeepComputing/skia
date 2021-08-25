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

bool GrMtlMSAALoadManager::createMSAALoadProgram(GrMtlGpu* gpu) {
    TRACE_EVENT0("skia", TRACE_FUNC);

    SkSL::String shaderText;
    shaderText.append(
            "#include <metal_stdlib>\n"
            "#include <simd/simd.h>\n"
            "using namespace metal;\n"
            "\n"
            "typedef struct {\n"
            "    float4 position [[position]];\n"
            "    uint2 texCoord;\n"
            "} VertexOutput;\n"
            "\n"
            "typedef struct {\n"
            "    float4 uPosXform;\n"
            "    uint2 uTextureSize;\n"
            "} VertexUniforms;\n"
            "\n"
            "vertex VertexOutput vertexMain(constant VertexUniforms& uniforms [[buffer(0)]],\n"
            "                               uint vertexID [[vertex_id]]) {\n"
            "    VertexOutput out;\n"
            "    float2 position = float2(float(vertexID >> 1), float(vertexID & 1));\n"
            "    out.position.xy = position * uniforms.uPosXform.xy + uniforms.uPosXform.zw;\n"
            "    out.position.zw = float2(0.0, 1.0);\n"
            "    out.texCoord = uint2((vertexID >> 1)*uniforms.uTextureSize.x,\n"
            "                        (vertexID & 1)*uniforms.uTextureSize.y);\n"
            "    return out;\n"
            "}\n"
            "\n"
            "fragment float4 fragmentMain(VertexOutput in [[stage_in]],\n"
            "                             texture2d<half> colorMap [[texture(0)]]) {\n"
            "    half4 colorSample   = colorMap.read(in.texCoord.xy);\n"
            "    return float4(colorSample);\n"
            "}"
    );

    auto errorHandler = gpu->getContext()->priv().getShaderErrorHandler();
    fShaderLibrary = GrCompileMtlShaderLibrary(gpu, shaderText, errorHandler);
    if (!fShaderLibrary) {
        this->destroyResources();
        return false;
    }
//
//
//
//
//    MtlDescriptorSetLayout dsLayout[GrMtlUniformHandler::kDescSetCount];
//
//    GrMtlResourceProvider& resourceProvider = gpu->resourceProvider();
//
//    dsLayout[GrMtlUniformHandler::kUniformBufferDescSet] = resourceProvider.getUniformDSLayout();
//
//    // Even though we don't have a sampler we need to put a valid handle here (of zero samplers)
//    // since we set up our descriptor layout to be uniform, sampler, input.
//    //
//    // TODO: We should have a more general way for different pipelines to describe their descriptor
//    // layouts so that we don't have to use the compile time constants for the sets.
//    GrMtlDescriptorSetManager::Handle samplerHandle;
//    resourceProvider.getZeroSamplerDescriptorSetHandle(&samplerHandle);
//
//    dsLayout[GrMtlUniformHandler::kSamplerDescSet] =
//            resourceProvider.getSamplerDSLayout(samplerHandle);
//
//    dsLayout[GrMtlUniformHandler::kInputDescSet] = resourceProvider.getInputDSLayout();
//
//    // Create the MtlPipelineLayout
//    MtlPipelineLayoutCreateInfo layoutCreateInfo;
//    memset(&layoutCreateInfo, 0, sizeof(MtlPipelineLayoutCreateFlags));
//    layoutCreateInfo.sType = Mtl_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    layoutCreateInfo.pNext = nullptr;
//    layoutCreateInfo.flags = 0;
//    layoutCreateInfo.setLayoutCount = GrMtlUniformHandler::kDescSetCount;
//    layoutCreateInfo.pSetLayouts = dsLayout;
//    layoutCreateInfo.pushConstantRangeCount = 0;
//    layoutCreateInfo.pPushConstantRanges = nullptr;
//
//    MtlResult err = GR_Mtl_CALL(
//            gpu->MtlInterface(),
//            CreatePipelineLayout(gpu->device(), &layoutCreateInfo, nullptr, &fPipelineLayout));
//    if (err) {
//        this->destroyResources(gpu);
//        return false;
//    }
//
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
        if (!this->createMSAALoadProgram(gpu)) {
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

    pipelineDescriptor.vertexDescriptor = nil;

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
    } uniData = {{dx1 - dx0, dy1 - dy0, dx0, dy0}, {dw, dh}};

    constexpr size_t uniformSize = 32;
    if (@available(macOS 10.11, iOS 8.3, *)) {
        SkASSERT(uniformSize <= gpu->caps()->maxPushConstantsSize());
        renderCmdEncoder->setVertexBytes(&uniData, uniformSize,
                                         GrMtlUniformHandler::kUniformBinding);
    } else {
        // upload the data
        GrRingBuffer::Slice slice = gpu->uniformsRingBuffer()->suballocate(uniformSize);
        GrMtlBuffer* buffer = (GrMtlBuffer*) slice.fBuffer;
        char* destPtr = static_cast<char*>(slice.fBuffer->map()) + slice.fOffset;
        memcpy(destPtr, &uniData, uniformSize);

        renderCmdEncoder->setVertexBuffer(buffer->mtlBuffer(), slice.fOffset,
                                          GrMtlUniformHandler::kUniformBinding);
    }

    renderCmdEncoder->drawPrimitives(MTLPrimitiveTypeTriangle, 4, 1);

    return true;
}

void GrMtlMSAALoadManager::destroyResources() {
    fShaderLibrary = nil;
}

