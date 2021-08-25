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

//#include "src/gpu/GrResourceProvider.h"
//#include "src/gpu/mtl/GrMtlBuffer.h"
//#include "src/gpu/mtl/GrMtlCommandBuffer.h"
//#include "src/gpu/Mtl/GrMtlDescriptorSet.h"
//#include "src/gpu/Mtl/GrMtlImageView.h"
//#include "src/gpu/mtl/GrMtlPipeline.h"
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
//        SkASSERT(fPipelineLayout == Mtl_NULL_HANDLE);
        if (!this->createMSAALoadProgram(gpu)) {
            SkDebugf("Failed to create copy program.\n");
            return false;
        }
    }

    GrMtlAttachment* mtlDst = static_cast<GrMtlAttachment*>(dst);

    auto pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];

    pipelineDescriptor.vertexFunction =
            [fShaderLibrary newFunctionWithName: @"vertexMain"];
    pipelineDescriptor.fragmentFunction =
            [fShaderLibrary newFunctionWithName: @"fragmentMain"];

    // ?
    pipelineDescriptor.vertexDescriptor = [[MTLVertexDescriptor alloc] init];

    auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

    mtlColorAttachment.pixelFormat = mtlDst->mtlFormat();
    mtlColorAttachment.blendingEnabled = FALSE;
    mtlColorAttachment.writeMask = MTLColorWriteMaskAll;

    pipelineDescriptor.colorAttachments[0] = mtlColorAttachment;

    pipelineDescriptor.sampleCount = dst->numSamples();
    // probably don't need this
    pipelineDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;

    NSError* error;
    auto pipelineState =
            [gpu->device() newRenderPipelineStateWithDescriptor: pipelineDescriptor
                                                          error: &error];


//    SkASSERT(fPipelineLayout != Mtl_NULL_HANDLE);
//
//    GrMtlResourceProvider& resourceProv = gpu->resourceProvider();
//
//    sk_sp<const GrMtlPipeline> pipeline =
//            resourceProv.findOrCreateMSAALoadPipeline(renderPass, dst->numSamples(),
//                                                      fShaderStageInfo, fPipelineLayout);
//    if (!pipeline) {
//        return false;
//    }
//    commandBuffer->bindPipeline(gpu, std::move(pipeline));
//
//    // Set Dynamic viewport and stencil
//    // We always use one viewport the size of the RT
//    MtlViewport viewport;
//    viewport.x = 0.0f;
//    viewport.y = 0.0f;
//    viewport.width = SkIntToScalar(dst->width());
//    viewport.height = SkIntToScalar(dst->height());
//    viewport.minDepth = 0.0f;
//    viewport.maxDepth = 1.0f;
//    commandBuffer->setViewport(gpu, 0, 1, &viewport);
//
//    // We assume the scissor is not enabled so just set it to the whole RT
//    MtlRect2D scissor;
//    scissor.extent.width = dst->width();
//    scissor.extent.height = dst->height();
//    scissor.offset.x = 0;
//    scissor.offset.y = 0;
//    commandBuffer->setScissor(gpu, 0, 1, &scissor);
//
//    // Update and bind uniform descriptor set
//    int w = rect.width();
//    int h = rect.height();
//
//    // dst rect edges in NDC (-1 to 1)
//    int dw = dst->width();
//    int dh = dst->height();
//    float dx0 = 2.f * rect.fLeft / dw - 1.f;
//    float dx1 = 2.f * (rect.fLeft + w) / dw - 1.f;
//    float dy0 = 2.f * rect.fTop / dh - 1.f;
//    float dy1 = 2.f * (rect.fTop + h) / dh - 1.f;
//
//    float uniData[] = {dx1 - dx0, dy1 - dy0, dx0, dy0};  // posXform
//
//    GrResourceProvider* resourceProvider = gpu->getContext()->priv().resourceProvider();
//    // TODO: Is it worth holding onto the last used uniform buffer and tracking the width, height,
//    // dst width, and dst height so that we can use the buffer again without having to update the
//    // data?
//    sk_sp<GrGpuBuffer> uniformBuffer = resourceProvider->createBuffer(
//            4 * sizeof(float), GrGpuBufferType::kUniform, kDynamic_GrAccessPattern, uniData);
//    if (!uniformBuffer) {
//        return false;
//    }
//    GrMtlBuffer* MtlUniformBuffer = static_cast<GrMtlBuffer*>(uniformBuffer.get());
//    static_assert(GrMtlUniformHandler::kUniformBufferDescSet < GrMtlUniformHandler::kInputDescSet);
//    commandBuffer->bindDescriptorSets(gpu, fPipelineLayout,
//                                      GrMtlUniformHandler::kUniformBufferDescSet,
//                                      /*setCount=*/1, MtlUniformBuffer->uniformDescriptorSet(),
//                                      /*dynamicOffsetCount=*/0, /*dynamicOffsets=*/nullptr);
//    commandBuffer->addGrBuffer(std::move(uniformBuffer));
//
//    // Update the input descriptor set
//    gr_rp<const GrMtlDescriptorSet> inputDS = src->inputDescSetForMSAALoad(gpu);
//    if (!inputDS) {
//        return false;
//    }
//    commandBuffer->bindDescriptorSets(gpu, fPipelineLayout,
//                                      GrMtlUniformHandler::kInputDescSet, /*setCount=*/1,
//                                      inputDS->descriptorSet(),
//                                      /*dynamicOffsetCount=*/0, /*dynamicOffsets=*/nullptr);
//
//    // We don't need to add the src and dst resources here since those are all tracked by the main
//    // render pass code out in GrMtlOpsRenderPass and GrMtlRenderTarget::adResources.
//    commandBuffer->addRecycledResource(std::move(inputDS));
//
//    commandBuffer->draw(gpu, 4, 1, 0, 0);

    return true;
}

void GrMtlMSAALoadManager::destroyResources() {
    fShaderLibrary = nil;

//    if (fPipelineLayout != Mtl_NULL_HANDLE) {
//        GR_Mtl_CALL(gpu->MtlInterface(),
//                   DestroyPipelineLayout(gpu->device(), fPipelineLayout, nullptr));
//        fPipelineLayout = Mtl_NULL_HANDLE;
//    }
}

