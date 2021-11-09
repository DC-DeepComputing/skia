/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlGraphicsPipeline.h"

#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/Uniform.h"

#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "experimental/graphite/src/mtl/MtlUtils.h"

#include "include/private/SkSLString.h"

namespace skgpu::mtl {

namespace {

/** Describes a vertex or instance attribute. */
// TODO: There is a good chance this will be lifted to a more general location as program
// representation is further fleshed out. For now, keep it next to where it's used.
class Attribute {
public:
    constexpr Attribute() = default;
    constexpr Attribute(const char* name,
                        VertexAttribType cpuType,
                        SLType gpuType)
            : fName(name), fCPUType(cpuType), fGPUType(gpuType) {
        SkASSERT(name && gpuType != SLType::kVoid);
    }
    constexpr Attribute(const Attribute&) = default;

    Attribute& operator=(const Attribute&) = default;

    constexpr bool isInitialized() const { return fGPUType != SLType::kVoid; }

    constexpr const char* name() const { return fName; }
    constexpr VertexAttribType cpuType() const { return fCPUType; }
    constexpr SLType           gpuType() const { return fGPUType; }

    constexpr size_t size()       const { return VertexAttribTypeSize(this->cpuType()); }
    constexpr size_t sizeAlign4() const { return SkAlign4(this->size()); }

private:
    const char* fName = nullptr;
    VertexAttribType fCPUType = VertexAttribType::kFloat;
    SLType fGPUType = SLType::kVoid;
};

class Iter {
public:
    Iter() : fCurr(nullptr), fRemaining(0) {}
    Iter(const Iter& iter) : fCurr(iter.fCurr), fRemaining(iter.fRemaining) {}
    Iter& operator= (const Iter& iter) {
        fCurr = iter.fCurr;
        fRemaining = iter.fRemaining;
        return *this;
    }
    Iter(const Attribute* attrs, int count) : fCurr(attrs), fRemaining(count) {
        this->skipUninitialized();
    }

    bool operator!=(const Iter& that) const { return fCurr != that.fCurr; }
    const Attribute& operator*() const { return *fCurr; }
    void operator++() {
        if (fRemaining) {
            fRemaining--;
            fCurr++;
            this->skipUninitialized();
        }
    }

private:
    void skipUninitialized() {
        if (!fRemaining) {
            fCurr = nullptr;
        } else {
            while (!fCurr->isInitialized()) {
                ++fCurr;
            }
        }
    }

    const Attribute* fCurr;
    int fRemaining;
};

class AttributeSet {
public:
    constexpr AttributeSet(const Attribute* attrs, int count)
            : fAttrs(attrs)
            , fRawCount(count) {}

    Iter begin() const { return Iter(fAttrs, this->count()); }
    Iter end() const { return Iter(); }

    // TODO: These are computed so that the ctor can be constexpr for convenience with hard-coded
    // definitions but that can be relaxed when the program system is expanded
    int count() const {
        int count = 0;
        for (int i = 0; i < fRawCount; ++i) {
            if (fAttrs[i].isInitialized()) {
                count++;
            }
        }
        return count;
     }

    size_t stride() const {
        size_t stride = 0;
        for (int i = 0; i < fRawCount; ++i) {
            if (fAttrs[i].isInitialized()) {
                stride += fAttrs[i].sizeAlign4();
            }
        }
        return stride;
     }

private:
    const Attribute* fAttrs;
    int fRawCount;
};

// Totally temporary, but aggregates MSL, attribute sets, and uniform definitions for the hard-coded
// shaders for both stages.
struct VertexShader {
    constexpr VertexShader(const char* msl,
                           const Attribute* vertexAttrs, int vertexAttrCount,
                           const Attribute* instanceAttrs, int instanceAttrCount)
            : fMSL(msl)
            , fVertexAttribs(vertexAttrs, vertexAttrCount)
            , fInstanceAttribs(instanceAttrs, instanceAttrCount) {}

    const char*  fMSL;
    AttributeSet fVertexAttribs;
    AttributeSet fInstanceAttribs;
};

const VertexShader& get_vertex_shader(TestVertexShader vs) {
    static constexpr Attribute kInstanceRectAttrs[3] = {
        { "position", VertexAttribType::kFloat2, SLType::kFloat2 },
        { "dims",     VertexAttribType::kFloat2, SLType::kFloat2 }
    };
    static constexpr Attribute kVertexTriAttrs[1] = {
        { "position", VertexAttribType::kFloat2, SLType::kFloat2 }
    };

    static constexpr VertexShader kVertexShaders[4] = {
        // kUnitRectangle
        { "typedef struct {\n"
          "    float4 position [[position]];\n"
          "} VertexOutput;\n"
          "\n"
          "vertex VertexOutput vertexMain(uint vertexID [[vertex_id]]) {\n"
          "    VertexOutput out;\n"
          "    float2 position = float2(float(vertexID >> 1), float(vertexID & 1));\n"
          "    out.position.xy = position * 2 - 1;\n"
          "    out.position.zw = float2(0.0, 1.0);\n"
          "    return out;\n"
          "}\n", nullptr, 0, nullptr, 0 },

        // kRectangle
        { "typedef struct {\n"
          "    float4 position [[position]];\n"
          "} VertexOutput;\n"
          "\n"
          "typedef struct {\n"
          "    float4 uPosXform;\n"
          "} VertexUniforms;\n"
          "\n"
          "vertex VertexOutput vertexMain(constant VertexUniforms& uniforms [[buffer(0)]],\n"
          "                               uint vertexID [[vertex_id]]) {\n"
          "    VertexOutput out;\n"
          "    float2 position = float2(float(vertexID >> 1), float(vertexID & 1));\n"
          "    out.position.xy = position * uniforms.uPosXform.xy + uniforms.uPosXform.zw;\n"
          "    out.position.zw = float2(0.0, 1.0);\n"
          "    return out;\n"
          "}\n", nullptr, 0, nullptr, 0 },

        // kInstanceRectangle
        { "typedef struct {\n"
          "    float2 position [[attribute(0)]];\n"
          "    float2 dims [[attribute(1)]];\n"
          "} InstanceInput;\n"
          "\n"
          "typedef struct {\n"
          "    float4 position [[position]];\n"
          "} VertexOutput;\n"
          "\n"
          "vertex VertexOutput vertexMain(InstanceInput in [[stage_in]],\n"
          "                               uint vertexID [[vertex_id]]) {\n"
          "    VertexOutput out;\n"
          "    float2 position = float2(float(vertexID >> 1), float(vertexID & 1));\n"
          "    out.position.xy = position * in.dims + in.position;\n"
          "    out.position.zw = float2(0.0, 1.0);\n"
          "    return out;\n"
          "}\n", nullptr, 0, kInstanceRectAttrs, 2 },

        // kVertexTris
        { "typedef struct {\n"
          "    float2 position [[attribute(0)]];\n"
          "} VertexInput;\n"
          "\n"
          "typedef struct {\n"
          "    float4 position [[position]];\n"
          "} VertexOutput;\n"
          "\n"
          "typedef struct {\n"
          "    float4 uPosXform;\n"
          "} VertexUniforms;\n"
          "\n"
          "vertex VertexOutput vertexMain(VertexInput in [[stage_in]],\n"
          "                               constant VertexUniforms& uniforms [[buffer(0)]],\n"
          "                               uint vertexID [[vertex_id]]) {\n"
          "    VertexOutput out;\n"
          "    float2 position = in.position;\n"
          "    out.position.xy = position * uniforms.uPosXform.xy + uniforms.uPosXform.zw;\n"
          "    out.position.zw = float2(0.0, 1.0);\n"
          "    return out;\n"
          "}\n", kVertexTriAttrs, 1, nullptr, 0 }
    };

    return kVertexShaders[static_cast<int>(vs)];
}


// TODO: use a SkSpan for the parameters
std::string emit_MSL_uniform_struct(const Uniform *uniforms, int numUniforms) {
    std::string result;

    result.append("struct FragmentUniforms {\n");
    for (int i = 0; i < numUniforms; ++i) {
        // TODO: this is sufficient for the sprint but should be changed to use SkSL's
        // machinery
        switch (uniforms[i].type()) {
            case SLType::kFloat4:
                result.append("vector_float4");
                break;
            case SLType::kFloat2:
                result.append("vector_float2");
                break;
            case SLType::kFloat:
                result.append("float");
                break;
            case SLType::kHalf4:
                result.append("vector_half4");
                break;
            default:
                SkASSERT(0);
        }

        result.append(" ");
        result.append(uniforms[i].name());
        if (uniforms[i].count()) {
            result.append("[");
            result.append(std::to_string(uniforms[i].count()));
            result.append("]");
        }
        result.append(";\n");
    }
    result.append("};\n");
    return result;
}

static SkSL::String get_msl(TestVertexShader vertexShader, ShaderCombo::ShaderType fragmentShader) {
    SkSL::String msl =
            "#include <metal_stdlib>\n"
            "#include <simd/simd.h>\n"
            "using namespace metal;\n"
            "\n";

    msl += get_vertex_shader(vertexShader).fMSL;

    auto fsUniforms = GetUniforms(fragmentShader);
    msl += emit_MSL_uniform_struct(fsUniforms.data(), fsUniforms.size());

    switch (fragmentShader) {
        case ShaderCombo::ShaderType::kLinearGradient:
            // TODO: this MSL uses a 'rtSize' uniform that, presumably, we'll be getting from the
            // vertex shader side of things (still somewhat TBD)
            // TODO: All of these fragmentMain funcs will need to also output depth.
            msl +=
            "fragment float4 fragmentMain(VertexOutput interpolated [[stage_in]],\n"
            "                             constant FragmentUniforms &uniforms [[buffer(1)]]) {\n"
            "    float2 screenPos = float2(2*interpolated.pos.x/uniforms.rtSize[0] - 1,\n"
            "                              2*interpolated.pos.y/uniforms.rtSize[1] - 1);\n"
            "    float2 delta = uniforms.point1 - uniforms.point0;\n"
            "    float2 pt = screenPos - uniforms.point0;\n"
            "    float t = dot(pt, delta) / dot(delta, delta);\n"
            "    float4 result = uniforms.colors[0];\n"
            "    result = mix(result, uniforms.colors[1],\n"
            "            clamp((t-uniforms.offsets[0])/(uniforms.offsets[1]-uniforms.offsets[0]),\n"
            "                  0, 1));\n"
            "    result = mix(result, uniforms.colors[2],\n"
            "            clamp((t-uniforms.offsets[1])/(uniforms.offsets[2]-uniforms.offsets[1]),\n"
            "                  0, 1));\n"
            "    result = mix(result, uniforms.colors[3],\n"
            "            clamp((t-uniforms.offsets[2])/(uniforms.offsets[3]-uniforms.offsets[2]),\n"
            "                  0, 1));\n"
            "    return result;\n"
            "}\n";
            break;
        case ShaderCombo::ShaderType::kNone:
            // TODO: kNone is for depth-only draws, so should actually have a fragment output type
            // that only defines a [[depth]] attribute but no color calculation.
            msl +=
            "fragment float4 fragmentMain(VertexOutput interpolated [[stage_in]]) {\n"
            "    return float4(0.0, 0.0, 1.0, 1.0);\n"
            "}\n";
            break;
        case ShaderCombo::ShaderType::kRadialGradient:
        case ShaderCombo::ShaderType::kSweepGradient:
        case ShaderCombo::ShaderType::kConicalGradient:
        case ShaderCombo::ShaderType::kSolidColor:
        default:
            msl +=
            "fragment float4 fragmentMain(VertexOutput interpolated [[stage_in]],\n"
            "                             constant FragmentUniforms &uniforms [[buffer(1)]]) {\n"
            "    return float4(uniforms.color);\n"
            "}\n";
            break;
    }

    return msl;
}

} // anonymous

static inline MTLVertexFormat attribute_type_to_mtlformat(VertexAttribType type) {
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

static MTLVertexDescriptor* create_vertex_descriptor(const GraphicsPipelineDesc& desc) {
    auto vertexDescriptor = [[MTLVertexDescriptor alloc] init];

    const VertexShader& shader = get_vertex_shader(desc.vertexShader());

    int attributeIndex = 0;

    int vertexAttributeCount = shader.fVertexAttribs.count();
    size_t vertexAttributeOffset = 0;
    for (const auto& attribute : shader.fVertexAttribs) {
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

    int instanceAttributeCount = shader.fInstanceAttribs.count();
    size_t instanceAttributeOffset = 0;
    for (const auto& attribute : shader.fInstanceAttribs) {
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

sk_sp<GraphicsPipeline> GraphicsPipeline::Make(const Gpu* gpu,
                                               const skgpu::GraphicsPipelineDesc& desc) {
    sk_cfp<MTLRenderPipelineDescriptor*> psoDescriptor([[MTLRenderPipelineDescriptor alloc] init]);

    // Temp pipeline for now that combines hard-coded vertex shaders and frag shaders
    SkSL::String shaderText = get_msl(desc.vertexShader(), desc.combo().fShaderType);

    auto metallib = CompileShaderLibrary(gpu, shaderText);
    if (!metallib) {
        return nullptr;
    }

    // TODO: Generate or include some useful label based on RenderStep and Combination.
    // (*psoDescriptor).label = kTestingOnlyShaderLabels[shaderIndex];

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
