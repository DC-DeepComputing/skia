/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlGraphicsPipeline.h"

#include "experimental/graphite/include/TextureInfo.h"
#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/Renderer.h"
#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "experimental/graphite/src/mtl/MtlResourceProvider.h"
#include "experimental/graphite/src/mtl/MtlUtils.h"
#include "include/core/SkSpan.h"
#include "include/private/SkSLString.h"

namespace skgpu::mtl {

namespace {

SkSL::String emit_SKSL_uniforms(int bufferID, const char* name, SkSpan<const Uniform> uniforms) {
    SkSL::String result;

    result.appendf("layout (binding=%d) uniform %sUniforms {\n", bufferID, name);

    int offset = 0;
    for (auto u : uniforms) {
        // TODO: this is sufficient for the sprint but should be changed to use SkSL's
        // machinery
        result.appendf("    layout(offset=%d) ", offset);
        switch (u.type()) {
            case SLType::kFloat4:
                result.append("float4");
                offset += 16 * u.count();
                break;
            case SLType::kFloat2:
                result.append("float2");
                offset += 8 * u.count();
                break;
            case SLType::kFloat:
                result.append("float");
                offset += 4 * u.count();
                break;
            case SLType::kFloat4x4:
                result.append("float4x4");
                offset += 64 * u.count();
                break;
            case SLType::kHalf4:
                result.append("half4");
                offset += 8 * u.count();
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
    result.append("};\n\n");
    return result;
}

SkSL::String emit_MSL_vertex_struct(const char* structName,
                                    SkSpan<const Attribute> vertexAttrs,
                                    SkSpan<const Attribute> instanceAttrs) {
    SkSL::String result;

    int attr = 0;
    auto add_attrs = [&](SkSpan<const Attribute> attrs) {
        for (auto a : attrs) {
            // TODO: this is sufficient for the sprint but should be changed to use SkSL's
            // machinery
            result.append("    ");
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

            result.appendf(" %s [[attribute(%d)]];\n", a.name(), attr++);
        }
    };

    result.appendf("struct %s {\n", structName);
    if (!vertexAttrs.empty()) {
        result.append("    // vertex attrs\n");
        add_attrs(vertexAttrs);
    }
    if (!instanceAttrs.empty()) {
        result.append("    // instance attrs\n");
        add_attrs(instanceAttrs);
    }

    result.append("};\n\n");
    return result;
}

// Gradient
#if 0
---- Vertex shader ----------------------------------------------------
SKSL:
1	layout (binding=0) uniform uniformBuffer
2	{
3		layout(offset=0) float4 sk_RTAdjust;
4		layout(offset=16) half4 ustart_S1_c0_c0;
5		layout(offset=24) half4 uend_S1_c0_c0;
6		layout(offset=32) float3x3 umatrix_S1_c0_c1;
7		layout(offset=80) half4 uleftBorderColor_S1_c0;
8		layout(offset=88) half4 urightBorderColor_S1_c0;
9	}
10	;
11	layout(location = 0) in float2 position;
12	layout(location = 1) in half4 color;
13	layout(location = 2) in float2 localCoord;
14	layout(location = 0) flat out half4 vcolor_S0;
15	layout(location = 1) out float2 vTransformedCoords_5_S0;
16	void main()
17	{
18		// Primitive Processor QuadPerEdgeAAGeometryProcessor
19		vcolor_S0 = color;
20		sk_Position = position.xy01;
21		{
22			vTransformedCoords_5_S0 = float3x2(umatrix_S1_c0_c1) * localCoord.xy1;
23		}
24	}
25
---- Fragment shader ----------------------------------------------------
SKSL:
1	layout (binding=0) uniform uniformBuffer
2	{
3		layout(offset=0) float4 sk_RTAdjust;
4		layout(offset=16) half4 ustart_S1_c0_c0;
5		layout(offset=24) half4 uend_S1_c0_c0;
6		layout(offset=32) float3x3 umatrix_S1_c0_c1;
7		layout(offset=80) half4 uleftBorderColor_S1_c0;
8		layout(offset=88) half4 urightBorderColor_S1_c0;
9	}
10	;
11	layout(location = 0) flat in half4 vcolor_S0;
12	layout(location = 1) in float2 vTransformedCoords_5_S0;
13	layout(location = 0, index = 0) out half4 sk_FragColor;
14	half4 SingleIntervalColorizer_S1_c0_c0(half4 _input, float2 _coords)
15	{
16		half4 _tmp_0_inColor = _input;
17		float2 _tmp_1_coords = _coords;
18		return half4(mix(ustart_S1_c0_c0, uend_S1_c0_c0, half(_tmp_1_coords.x)));
19	}
20	half4 LinearLayout_S1_c0_c1_c0(half4 _input)
21	{
22		half4 _tmp_2_inColor = _input;
23		float2 _tmp_3_coords = vTransformedCoords_5_S0;
24		return half4(half4(half(_tmp_3_coords.x) + 9.9999997473787516e-06, 1.0, 0.0, 0.0));
25	}
26	half4 MatrixEffect_S1_c0_c1(half4 _input)
27	{
28		return LinearLayout_S1_c0_c1_c0(_input);
29	}
30	half4 ClampedGradient_S1_c0(half4 _input)
31	{
32		half4 _tmp_4_inColor = _input;
33		half4 t = MatrixEffect_S1_c0_c1(_tmp_4_inColor);
34		half4 outColor;
35		if (!bool(int(1)) && t.y < 0.0)
36		{
37			outColor = half4(0.0);
38		}
39		else if (t.x < 0.0)
40		{
41			outColor = uleftBorderColor_S1_c0;
42		}
43		else if (t.x > 1.0)
44		{
45			outColor = urightBorderColor_S1_c0;
46		}
47		else
48		{
49			outColor = SingleIntervalColorizer_S1_c0_c0(_tmp_4_inColor, float2(half2(t.x, 0.0)));
50		}
51		if (bool(int(0)))
52		{
53			outColor.xyz *= outColor.w;
54		}
55		return half4(outColor);
56	}
57	half4 DisableCoverageAsAlpha_S1(half4 _input)
58	{
59		_input = ClampedGradient_S1_c0(_input);
60		half4 _tmp_5_inColor = _input;
61		return half4(_input);
62	}
63	void main()
64	{
65		// Stage 0, QuadPerEdgeAAGeometryProcessor
66		half4 outputColor_S0;
67		outputColor_S0 = vcolor_S0;
68		const half4 outputCoverage_S0 = half4(1);
69		half4 output_S1;
70		output_S1 = DisableCoverageAsAlpha_S1(outputColor_S0);
71		{
72			// Xfer Processor: Porter Duff
73			sk_FragColor = output_S1 * outputCoverage_S0;
74		}
75	}
#endif
// Solid
#if 0
---- Vertex shader ----------------------------------------------------
SKSL:
1	layout (binding=0) uniform uniformBuffer
2	{
3		layout(offset=0) float4 sk_RTAdjust;
4	}
5	;
6	layout(location = 0) in float2 position;
7	layout(location = 1) in half4 color;
8	layout(location = 0) flat out half4 vcolor_S0;
9	void main()
10	{
11		// Primitive Processor QuadPerEdgeAAGeometryProcessor
12		vcolor_S0 = color;
13		sk_Position = position.xy01;
14	}
15
---- Fragment shader ----------------------------------------------------
SKSL:
1	layout (binding=0) uniform uniformBuffer
2	{
3		layout(offset=0) float4 sk_RTAdjust;
4	}
5	;
6	layout(location = 0) flat in half4 vcolor_S0;
7	layout(location = 0, index = 0) out half4 sk_FragColor;
8	void main()
9	{
10		// Stage 0, QuadPerEdgeAAGeometryProcessor
11		half4 outputColor_S0;
12		outputColor_S0 = vcolor_S0;
13		const half4 outputCoverage_S0 = half4(1);
14		{
15			// Xfer Processor: Porter Duff
16			sk_FragColor = outputColor_S0 * outputCoverage_S0;
17		}
18	}
#endif

SkSL::String get_sksl_vs(const GraphicsPipelineDesc& desc) {
    const RenderStep *step = desc.renderStep();
    // TODO: To more completely support end-to-end rendering, this will need to be updated so that
    // the RenderStep shader snippet can produce a device coord, a local coord, and depth.
    // If the paint combination doesn't need the local coord it can be ignored, otherwise we need
    // a varying for it. The fragment function's output will need to be updated to have a color and
    // the depth, or when there's no combination, just the depth. Lastly, we also should add the
    // static/intrinsic uniform binding point so that we can handle normalizing the device position
    // produced by the RenderStep automatically.

    // Fixed program header
    SkSL::String sksl =
        "layout (binding=0) uniform intrinsicUniforms {\n"
        "    layout(offset=0) float4 rtAdjust;\n"
        "};\n"
        "layout(location = 0) in float2 position;\n"
        "\n";

#if 0
    "typedef struct {\n"
    "    float4 rtAdjust;\n"
    "} Intrinsics;\n"
    "typedef struct {\n"
    "    float4 position [[position]];\n"
    "} VertexOutput;\n"
    "typedef struct {\n"
    "    float4 color [[color(0)]];\n"
    "} FragmentOutput;\n"
    "\n";
#endif

    // Typedefs needed by RenderStep
    if (step->numUniforms() > 0) {
        sksl += emit_SKSL_uniforms(1, "VS", step->uniforms());
    }
//    if (step->numVertexAttributes() > 0 || step->numInstanceAttributes() > 0) {
//        sksl += emit_MSL_vertex_struct("VertexAttrs",
//                                       step->vertexAttributes(),
//                                       step->instanceAttributes());
//    }

    // Vertex shader function declaration
    sksl += "void main() {\n";
//    if (step->numVertexAttributes() > 0 || step->numInstanceAttributes() > 0) {
//        sksl += ",\n                        VertexAttrs vtx [[stage_in]]";
//    }
//    if (step->numUniforms() > 0) {
//        sksl += ",\n                        constant StepUniforms& uniforms [[buffer(1)]]";
//    }
//    sksl += ") {\n";

    // Vertex shader body
    //sksl += "    VertexOutput out;\n";
    sksl += step->vertexMSL();
    sksl += "    sk_Position = float4(stepPosition.xy * rtAdjust.xy + rtAdjust.zw, stepPosition.zw);\n"
            "}\n";

    return sksl;
}

SkSL::String get_sksl_fs(const GraphicsPipelineDesc& desc) {
    SkSL::String sksl;

    // Typedefs needed for painting
    auto paintUniforms = GetUniforms(desc.shaderCombo().fShaderType);
    if (!paintUniforms.empty()) {
        sksl += emit_SKSL_uniforms(2, "FS", paintUniforms);
    }

    sksl += "half4 main() {\n"
            "    half4 outColor;\n";
    sksl += GetShaderSkSL(desc.shaderCombo().fShaderType);
    sksl += "    return outColor;\n"
            "}\n";

    return sksl;
}

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

MTLVertexDescriptor* create_vertex_descriptor(const RenderStep* step) {
    auto vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    int attributeIndex = 0;

    int vertexAttributeCount = step->numVertexAttributes();
    size_t vertexAttributeOffset = 0;
    for (const auto& attribute : step->vertexAttributes()) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        MTLVertexFormat format = attribute_type_to_mtlformat(attribute.cpuType());
        SkASSERT(MTLVertexFormatInvalid != format);
        mtlAttribute.format = format;
        mtlAttribute.offset = vertexAttributeOffset;
        mtlAttribute.bufferIndex = GraphicsPipeline::kVertexBufferIndex;

        vertexAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }
    SkASSERT(vertexAttributeOffset == step->vertexStride());

    if (vertexAttributeCount) {
        MTLVertexBufferLayoutDescriptor* vertexBufferLayout =
                vertexDescriptor.layouts[GraphicsPipeline::kVertexBufferIndex];
        vertexBufferLayout.stepFunction = MTLVertexStepFunctionPerVertex;
        vertexBufferLayout.stepRate = 1;
        vertexBufferLayout.stride = vertexAttributeOffset;
    }

    int instanceAttributeCount = step->numInstanceAttributes();
    size_t instanceAttributeOffset = 0;
    for (const auto& attribute : step->instanceAttributes()) {
        MTLVertexAttributeDescriptor* mtlAttribute = vertexDescriptor.attributes[attributeIndex];
        MTLVertexFormat format = attribute_type_to_mtlformat(attribute.cpuType());
        SkASSERT(MTLVertexFormatInvalid != format);
        mtlAttribute.format = format;
        mtlAttribute.offset = instanceAttributeOffset;
        mtlAttribute.bufferIndex = GraphicsPipeline::kInstanceBufferIndex;

        instanceAttributeOffset += attribute.sizeAlign4();
        attributeIndex++;
    }
    SkASSERT(instanceAttributeOffset == step->instanceStride());

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

    //auto errorHandler = gpu->getContext()->priv().getShaderErrorHandler();
    SkSL::String msl[kGrShaderTypeCount];
    SkSL::Program::Inputs inputs[kGrShaderTypeCount];
    SkSL::Program::Settings settings;

    if (!SkSLToMSL(gpu,
                   get_sksl_vs(desc),
                   SkSL::ProgramKind::kVertex,
                   settings,
                   &msl[kVertex_GrShaderType],
                   &inputs[kVertex_GrShaderType])) {
        return nullptr;
    }

    if (!SkSLToMSL(gpu,
                   get_sksl_fs(desc),
                   SkSL::ProgramKind::kFragment,
                   settings,
                   &msl[kFragment_GrShaderType],
                   &inputs[kFragment_GrShaderType])) {
        return nullptr;
    }

    sk_cfp<id<MTLLibrary>> shaderLibraries[kGrShaderTypeCount];

    shaderLibraries[kVertex_GrShaderType] = CompileShaderLibrary(gpu, msl[kVertex_GrShaderType]);
    shaderLibraries[kFragment_GrShaderType] = CompileShaderLibrary(gpu, msl[kFragment_GrShaderType]);
    if (!shaderLibraries[kVertex_GrShaderType] || !shaderLibraries[kFragment_GrShaderType]) {
        return nullptr;
    }

    (*psoDescriptor).label = @(desc.renderStep()->name());

    (*psoDescriptor).vertexFunction =
            [shaderLibraries[kVertex_GrShaderType].get() newFunctionWithName: @"vertexMain"];
    (*psoDescriptor).fragmentFunction =
            [shaderLibraries[kFragment_GrShaderType].get() newFunctionWithName: @"fragmentMain"];

    // TODO: I *think* this gets cleaned up by the pipelineDescriptor?
    (*psoDescriptor).vertexDescriptor = create_vertex_descriptor(desc.renderStep());

    // TODO: I *think* this gets cleaned up by the pipelineDescriptor as well?
    auto mtlColorAttachment = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];

    mtlColorAttachment.pixelFormat = MTLPixelFormatRGBA8Unorm;
    mtlColorAttachment.blendingEnabled = FALSE;
    mtlColorAttachment.writeMask = MTLColorWriteMaskAll;

    (*psoDescriptor).colorAttachments[0] = mtlColorAttachment;

    DepthStencilType depthStencilType = desc.renderStep()->requiresStencil()
                                      ? DepthStencilType::kDepthStencil
                                      : DepthStencilType::kDepthOnly;
    skgpu::TextureInfo texInfo = gpu->caps()->getDefaultDepthStencilTextureInfo(depthStencilType,
                                                                                1 /*sampleCount*/,
                                                                                Protected::kNo);
    mtl::TextureInfo mtlTexInfo;
    texInfo.getMtlTextureInfo(&mtlTexInfo);
    if (depthStencilType != DepthStencilType::kDepthOnly) {
        (*psoDescriptor).stencilAttachmentPixelFormat = (MTLPixelFormat)mtlTexInfo.fFormat;
    }
    (*psoDescriptor).depthAttachmentPixelFormat = (MTLPixelFormat)mtlTexInfo.fFormat;

    NSError* error;
    sk_cfp<id<MTLRenderPipelineState>> pso(
            [gpu->device() newRenderPipelineStateWithDescriptor:psoDescriptor.get()
                                                          error:&error]);
    if (!pso) {
        SkDebugf("Pipeline creation failure\n");
        SkDebugf("Errors:\n%s", error.debugDescription.UTF8String);
        return nullptr;
    }

    auto resourceProvider = (skgpu::mtl::ResourceProvider*) gpu->resourceProvider();
    id<MTLDepthStencilState> dss = resourceProvider->findOrCreateCompatibleDepthStencilState(
            desc.renderStep()->depthStencilSettings());

    return sk_sp<GraphicsPipeline>(new GraphicsPipeline(std::move(pso),
                                                        dss,
                                                        desc.renderStep()->vertexStride(),
                                                        desc.renderStep()->instanceStride()));
}

} // namespace skgpu::mtl
