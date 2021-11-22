/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlGraphicsPipeline.h"

#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "experimental/graphite/src/mtl/MtlUtils.h"
#include "include/private/SkSLString.h"

namespace skgpu::mtl {

namespace {

SkSL::String emit_MSL_uniform_struct(const char* structName, SkSpan<const Uniform> uniforms) {
    SkSL::String result;

    result.appendf("struct %s {\n", structName);
    for (auto u : uniforms) {
        // TODO: this is sufficient for the sprint but should be changed to use SkSL's
        // machinery
        switch (u.type()) {
            case SLType::kFloat4:
                result.append("float4");
                break;
            case SLType::kFloat2:
                result.append("float2");
                break;
            case SLType::kFloat:
                result.append("float");
                break;
            case SLType::kHalf4:
                result.append("half4");
                break;
            default:
                SkASSERT(0);
        }

        result.append(" ");
        result.append(u.name());
        if (u.count()) {
            result.append("[");
            result.append(std::to_string(u.count()));
            result.append("]");
        }
        result.append(";\n");
    }
    result.append("};\n");
    return result;
}

SkSL::String emit_MSL_vertex_struct(const char* structName, SkSpan<const Attribute> attrs, int* attr) {
    SkSL::String result;

    result.append("typedef struct {\n");
    for (auto a : attrs) {
        // TODO: this is sufficient for the sprint but should be changed to use SkSL's
        // machinery
        switch (a.gpuType()) {
            case SLType::kFloat4:
                result.append("float4");
                break;
            case SLType::kFloat2:
                result.append("float2");
                break;
            case SLType::kFloat:
                result.append("float");
                break;
            case SLType::kHalf4:
                result.append("half4");
                break;
            default:
                SkASSERT(0);
        }

        result.appendf(" %s [[attribute(%d)]]\n", a.name(), (*attr)++);
    }

    result.appendf("} %s;", structName);
    return result;
}

SkSL::String get_msl(const GraphicsPipelineDesc& desc) {
    const RenderStep* step = desc.renderStep();
    // TODO: To more completely support end-to-end rendering, this will need to be updated so that
    // the RenderStep shader snippet can produce a device coord, a local coord, and depth.
    // If the paint combination doesn't need the local coord it can be ignored, otherwise we need
    // a varying for it. The fragment function's output will need to be updated to have a color and
    // the depth, or when there's no combination, just the depth. Lastly, we also should add the
    // static/intrinsic uniform binding point so that we can handle normalizing the device position
    // produced by the RenderStep automatically.

    // Fixed program header
    SkSL::String msl =
            "#include <metal_stdlib>\n"
            "#include <simd/simd.h>\n"
            "using namespace metal;\n"
            "\n"
            "typedef struct {\n"
            "    float4 position [[position]];\n"
            "} VertexOutput;\n"
            "\n";

    // Typedefs needed by RenderStep
    if (step->numUniforms() > 0) {
        msl += emit_MSL_uniform_struct("StepUniforms", step->uniforms());
    }
    int attrIndex = 0;
    if (step->numVertexAttributes() > 0) {
        msl += emit_MSL_vertex_struct("VertexAttrs", step->vertexAttributes(), &attrIndex);
    }
    if (step->numInstanceAttributes() > 0) {
        msl += emit_MSL_vertex_struct("InstanceAttrs", step->instanceAttributes(), &attrIndex);
    }

    // Vertex shader function declaration
    msl += "vertex VertexOutput vertexMain(uint vertexID [[vertex_id]],\n"
           "                               uint instanceID [[instance_id]]";
    if (step->numVertexAttributes() > 0) {
        msl += "\n,                        VertexAttrs vertex [[stage_in]]";
    }
    if (step->numInstanceAttributes() > 0) {
        msl += "\n,                        InstanceAttrs instance [[stage_in]]";
    }
    if (step->numUniforms() > 0) {
        msl += "\n,                        const StepUniforms& uniforms [[buffer(0)]]";
    }
    msl += ") {\n"

    // Vertex shader body
    msl += "    VertexOutput out;\n";
    msl += step->vertexMSL();
    msl += "}\n"

    // Typedefs needed for painting
    // TODO: Right now hardcoded float4 color uniform but will come from Combination once that is
    // stored on the GraphicsPipelineDesc.
    msl += "struct PaintUniforms {\n"
           "    float4 color;\n"
           "};\n";
    msl += "fragment float4 fragmentMain(VertexOutput in [[stage_in]],\n"
           "                             const PaintUniforms& uniforms [[buffer(1)]]) {\n"
           "    return uniforms.color;\n"
           "}\n";

    return msl;
}

// static constexpr NSString* kTestingOnlyShaderLabels[]  = {
//     @"Clear viewport to blue",
//     @"Clear rect with uniforms",
//     @"Draw triangles with uniform color",
//     @"Draw triangles with instance buffer"
// };

inline MTLVertexFormat attribute_type_to_mtlformat(VertexAttribType type) {
    switch (type) {
        case VertexAttribType::kFloat:
            return MTLVertexFormatFloat;
        case VertexAttribType::kFloat2:
            return MTLVertexFormatFloat2;
        case VertexAttribType::kFloat3:
            return MTLVertexFormatFloat3;
        case VertexAttribType::kFloat4:
            return MTLVertexFormatFloat4;
        case VertexAttribType::kHalf:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatHalf;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kHalf2:
            return MTLVertexFormatHalf2;
        case VertexAttribType::kHalf4:
            return MTLVertexFormatHalf4;
        case VertexAttribType::kInt2:
            return MTLVertexFormatInt2;
        case VertexAttribType::kInt3:
            return MTLVertexFormatInt3;
        case VertexAttribType::kInt4:
            return MTLVertexFormatInt4;
        case VertexAttribType::kByte:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatChar;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kByte2:
            return MTLVertexFormatChar2;
        case VertexAttribType::kByte4:
            return MTLVertexFormatChar4;
        case VertexAttribType::kUByte:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatUChar;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kUByte2:
            return MTLVertexFormatUChar2;
        case VertexAttribType::kUByte4:
            return MTLVertexFormatUChar4;
        case VertexAttribType::kUByte_norm:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatUCharNormalized;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kUByte4_norm:
            return MTLVertexFormatUChar4Normalized;
        case VertexAttribType::kShort2:
            return MTLVertexFormatShort2;
        case VertexAttribType::kShort4:
            return MTLVertexFormatShort4;
        case VertexAttribType::kUShort2:
            return MTLVertexFormatUShort2;
        case VertexAttribType::kUShort2_norm:
            return MTLVertexFormatUShort2Normalized;
        case VertexAttribType::kInt:
            return MTLVertexFormatInt;
        case VertexAttribType::kUInt:
            return MTLVertexFormatUInt;
        case VertexAttribType::kUShort_norm:
            if (@available(macOS 10.13, iOS 11.0, *)) {
                return MTLVertexFormatUShortNormalized;
            } else {
                return MTLVertexFormatInvalid;
            }
        case VertexAttribType::kUShort4_norm:
            return MTLVertexFormatUShort4Normalized;
    }
    SK_ABORT("Unknown vertex attribute type");
}

MTLVertexDescriptor* create_vertex_descriptor(const GraphicsPipelineDesc& desc) {
    auto vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    int attributeIndex = 0;

    int vertexAttributeCount = desc.numVertexAttributes();
    size_t vertexAttributeOffset = 0;
    for (const auto& attribute : desc.vertexAttributes()) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        MTLVertexFormat format = attribute_type_to_mtlformat(attribute.cpuType());
        SkASSERT(MTLVertexFormatInvalid != format);
        mtlAttribute.format = format;
        mtlAttribute.offset = vertexAttributeOffset;
        mtlAttribute.bufferIndex = GraphicsPipeline::kVertexBufferIndex;

        vertexAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }
    SkASSERT(vertexAttributeOffset == desc.vertexStride());

    if (vertexAttributeCount) {
        MTLVertexBufferLayoutDescriptor* vertexBufferLayout =
                vertexDescriptor.layouts[GraphicsPipeline::kVertexBufferIndex];
        vertexBufferLayout.stepFunction = MTLVertexStepFunctionPerVertex;
        vertexBufferLayout.stepRate = 1;
        vertexBufferLayout.stride = vertexAttributeOffset;
    }

    int instanceAttributeCount = desc.numInstanceAttributes();
    size_t instanceAttributeOffset = 0;
    for (const auto& attribute : desc.instanceAttributes()) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        MTLVertexFormat format = attribute_type_to_mtlformat(attribute.cpuType());
        SkASSERT(MTLVertexFormatInvalid != format);
        mtlAttribute.format = format;
        mtlAttribute.offset = instanceAttributeOffset;
        mtlAttribute.bufferIndex = GraphicsPipeline::kInstanceBufferIndex;

        instanceAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }
    SkASSERT(instanceAttributeOffset == desc.instanceStride());

    if (instanceAttributeCount) {
        MTLVertexBufferLayoutDescriptor* instanceBufferLayout =
                vertexDescriptor.layouts[GraphicsPipeline::kInstanceBufferIndex];
        instanceBufferLayout.stepFunction = MTLVertexStepFunctionPerInstance;
        instanceBufferLayout.stepRate = 1;
        instanceBufferLayout.stride = instanceAttributeOffset;
    }
    return vertexDescriptor;
}

} // anonymous namespace

sk_sp<GraphicsPipeline> GraphicsPipeline::Make(const Gpu* gpu,
                                               const skgpu::GraphicsPipelineDesc& desc) {
    sk_cfp<MTLRenderPipelineDescriptor*> psoDescriptor([[MTLRenderPipelineDescriptor alloc] init]);

    auto metallib = CompileShaderLibrary(gpu, get_msl(desc));
    if (!metallib) {
        return nullptr;
    }

    (*psoDescriptor).label = @(desc.renderStep()->name());

    (*psoDescriptor).vertexFunction =
            [*metallib newFunctionWithName: @"vertexMain"];
    (*psoDescriptor).fragmentFunction =
            [*metallib newFunctionWithName: @"fragmentMain"];

    // TODO: I *think* this gets cleaned up by the pipelineDescriptor?
    (*psoDescriptor).vertexDescriptor = create_vertex_descriptor(desc);

    // TODO: I *think* this gets cleaned up by the pipelineDescriptor as well?
    auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

    mtlColorAttachment.pixelFormat = MTLPixelFormatRGBA8Unorm;
    mtlColorAttachment.blendingEnabled = FALSE;
    mtlColorAttachment.writeMask = MTLColorWriteMaskAll;

    (*psoDescriptor).colorAttachments[0] = mtlColorAttachment;
    (*psoDescriptor).sampleCount = 1;

    NSError* error;
    sk_cfp<id<MTLRenderPipelineState>> pso(
            [gpu->device() newRenderPipelineStateWithDescriptor:psoDescriptor.get()
                                                          error:&error]);
    if (!pso) {
        SkDebugf("Pipeline creation failure\n");
        SkDebugf("Errors:\n%s", error.debugDescription.UTF8String);
        return nullptr;
    }
    return sk_sp<GraphicsPipeline>(new GraphicsPipeline(std::move(pso), desc.vertexStride(),
                                                        desc.instanceStride()));
}

} // namespace skgpu::mtl
