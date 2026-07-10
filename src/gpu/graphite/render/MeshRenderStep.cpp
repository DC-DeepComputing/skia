/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/MeshRenderStep.h"

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSpan.h"
#include "include/core/SkVertices.h"
#include "include/private/SkAssert.h"
#include "include/private/SkDebug.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTo.h"
#include "src/core/SkMeshPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/core/SkVertState.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLProgram.h"

#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

#include <cstdint>

namespace skgpu::graphite {

class MeshProgramCallbacks : public SkSL::PipelineStage::Callbacks {
public:
    MeshProgramCallbacks(std::string mainFn,
                         const ShaderInfo* shaderInfo,
                         SkSpan<const ShaderNode*> childNodes)
            : fMainFn(mainFn), fShaderInfo(shaderInfo), fChildNodes(childNodes) {}

    std::string getMainName() override { return fMainFn; }
    void defineFunction(const char* declaration, const char* body, bool isMain) override {
        fPreamble = fPreamble.append(SkSL::String::printf("%s {\n %s }\n", declaration, body));
    }
    void declareFunction(const char* declaration) override {}
    void defineStruct(const char* definition) override {
        fPreamble = fPreamble.append(definition).append("\n");
    }
    void declareGlobal(const char* declaration) override {
        fPreamble = fPreamble.append(declaration).append("\n");
    }

    std::string declareUniform(const SkSL::VarDeclaration* decl) override {
        const std::string uniformName = std::string(decl->var()->name());
        if (fShaderInfo->uniformSsboIndex()) {
            return SkSL::String::printf("combinedUniformData[%s].%s",
                                        fShaderInfo->uniformSsboIndex(),
                                        uniformName.c_str());
        }
        return uniformName;
    }

    std::string sampleShader(int index, std::string coords) override {
        ShaderSnippet::Args args = ShaderSnippet::kDefaultArgs;
        args.fFragCoord = coords;
        return fChildNodes[index]->invoke(*fShaderInfo, args);
    }

    std::string sampleColorFilter(int index, std::string color) override {
        ShaderSnippet::Args args = ShaderSnippet::kDefaultArgs;
        args.fPriorStageOutput = color;
        return fChildNodes[index]->invoke(*fShaderInfo, args);
    }

    std::string sampleBlender(int index, std::string src, std::string dst) override {
        ShaderSnippet::Args args = ShaderSnippet::kDefaultArgs;
        args.fPriorStageOutput = src;
        args.fBlenderDstColor = dst;
        return fChildNodes[index]->invoke(*fShaderInfo, args);
    }

    std::string toLinearSrgb(std::string color) override {
        SK_ABORT("Color transform intrinsics not allowed.");
    }
    std::string fromLinearSrgb(std::string color) override {
        SK_ABORT("Color transform intrinsics not allowed.");
    }

    std::string fPreamble;
    std::string fMainFn;
    const ShaderInfo* fShaderInfo;
    SkSpan<const ShaderNode*> fChildNodes;
};

static constexpr char kMeshFSLocalCoordsName[] = "meshLocalCoordsOverride";

MeshRenderStep::MeshRenderStep(RenderStepID renderStepId, Layout layout,
                               sk_sp<SkMeshSpecification> spec, SkMesh::Mode mode,
                               std::vector<Attribute> attrs, std::vector<Varying> varyings,
                               std::unordered_map<std::string, std::string> mangledVaryingsMap,
                               std::vector<Uniform> uniforms, bool hasColorOutput)
        : RenderStep(layout,
                     renderStepId,
                     Flags::kPerformsShading
                        | Flags::kAppendInstances
                        | (hasColorOutput ? Flags::kEmitsPrimitiveColor : Flags::kNone),
                     /*uniforms=*/uniforms,
                     PrimitiveTypeFromSkMeshMode(mode),
                     kDirectDepthLEqualPass,
                     /*staticAttrs=*/attrs,
                     /*appendAttrs=*/{
                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}
                     },
                     varyings)
        , fSpec(spec)
        , fAttributes(attrs)
        , fMangledVaryingsMap(mangledVaryingsMap)
        , fHasColorOutput(hasColorOutput) {
    setStaticDataStride(spec->stride());
}

MeshRenderStep::~MeshRenderStep() {}

std::unique_ptr<MeshRenderStep> MeshRenderStep::Make(RenderStepID renderStepId, Layout layout,
                                                     const SkMesh& mesh) {
    sk_sp<SkMeshSpecification> spec = mesh.refSpec();

    size_t attributeCount = spec->attributes().size();
    std::vector<Attribute> attributes(attributeCount);
    for (size_t i = 0; i < attributeCount; i++) {
        attributes[i] = Attribute::MakeFromSkMeshAttribute(spec->attributes()[i]);
    }

    auto meshVaryings =
        SkMeshSpecificationPriv::Varyings(*spec);
    std::vector<Varying> varyings(meshVaryings.size());
    std::unordered_map<std::string, std::string> mangledVaryingsMap;
    for (size_t i = 0; i < meshVaryings.size(); i++) {
        const SkMeshSpecification::Varying& v = meshVaryings[i];
        std::string mangledName = std::string(v.name.c_str()) + "_SkMeshSpecificationVaryingName";
        mangledVaryingsMap.emplace(mangledName, std::string(v.name.c_str()));
        varyings[i] = Varying(mangledName, SkMeshSpecificationPriv::VaryingTypeAsSLType(v.type));
    }

    std::vector<Uniform> uniforms;
    for (const SkMeshSpecification::Uniform& u: spec->uniforms()) {
        uniforms.emplace_back(std::string(u.name),
                              ShaderCodeDictionary::UniformTypeToSkSLType(u),
                              u.isArray() ? u.count : Uniform::kNonArray);
    }
    bool hasColorOutput =
        SkMeshSpecificationPriv::GetColorType(*spec) != SkMeshSpecificationPriv::ColorType::kNone;

    return std::unique_ptr<MeshRenderStep>(
                new MeshRenderStep(renderStepId, layout, spec, mesh.mode(), attributes,
                                   varyings, mangledVaryingsMap, uniforms, hasColorOutput));
}

std::string MeshRenderStep::vertexSkSL() const {
    std::string attrs = "Attributes attributes;\n";
    for (const Attribute& attr: fAttributes) {
        attrs = attrs.append(SkSL::String::printf("attributes.%s = %s;\n",
                             attr.name(), attr.name()));
    }

    std::string varyingAssignments;
    for (const Varying& v: this->varyings()) {
        const std::string& unmangledName = fMangledVaryingsMap.at(v.name());
        varyingAssignments += SkSL::String::printf("%s = varyings.%s;\n",
                                                   v.name(), unmangledName.c_str());
    }

    return SkSL::String::printf("%s\n"
                                "Varyings varyings = %s(attributes);\n"
                                "float3 devPosition3 = float3x3(mat0, mat1, mat2)\n"
                                "                   * float3(varyings.position, 1.0);\n"
                                "float4 devPosition = float4(devPosition3.xy, 0.0, 1.0);\n"
                                "stepLocalCoords = varyings.position;\n"
                                "%s", attrs.c_str(), kMeshVSMainName,
                                varyingAssignments.c_str());
}

std::string MeshRenderStep::vertexSkSLTopLevel(const ShaderInfo* shaderInfo) const {
    const SkSL::Program* vsProgram = SkMeshSpecificationPriv::VS(*fSpec);
    MeshProgramCallbacks vsCallbacks(kMeshVSMainName, shaderInfo, {});

    SkSL::PipelineStage::ConvertProgram(*vsProgram,
                                        /*sampleCoords=*/nullptr,
                                        /*inputColor=*/  nullptr,
                                        /*destColor=*/   nullptr,
                                        &vsCallbacks);
    return vsCallbacks.fPreamble;
}

std::string MeshRenderStep::fragmentColorSkSL() const {
    std::string s = "Varyings varyings;\n";
    for (const Varying& v: this->varyings()) {
        const std::string& unmangledName = fMangledVaryingsMap.at(v.name());
        s += SkSL::String::printf("varyings.%s = %s;\n", unmangledName.c_str(), v.name());
    }

    bool needsColorConversion = SkMeshSpecificationPriv::GetColorType(*fSpec)
                                        == SkMeshSpecificationPriv::ColorType::kFloat4;
    std::string outColorName = "primitiveColor";
    if (needsColorConversion) {
        outColorName = "primitiveColorFloat4";
        s += "float4 " + outColorName + ";\n";
    }

    std::string methodCall = fHasColorOutput ? SkSL::String::printf("%s(varyings, %s)",
                                                                   kMeshFSMainName,
                                                                   outColorName.c_str())
                                            : SkSL::String::printf("%s(varyings)",
                                                                   kMeshFSMainName);

    s += SkSL::String::printf("float2 %s = %s;\n",
                              kMeshFSLocalCoordsName,
                              methodCall.c_str());
    if (needsColorConversion) {
        s += "primitiveColor = half4(" + outColorName + ");\n";
    }
    return s;
}

std::string MeshRenderStep::fragmentColorSkSLTopLevel(const ShaderInfo* shaderInfo,
                                                      SkSpan<const ShaderNode*> rootNodes) const {
    const SkSL::Program* fsProgram = SkMeshSpecificationPriv::FS(*fSpec);
    MeshProgramCallbacks fsCallbacks(kMeshFSMainName, shaderInfo, rootNodes);
    SkSL::PipelineStage::ConvertProgram(*fsProgram,
                                        /*sampleCoords=*/nullptr,
                                        /*inputColor=*/  nullptr,
                                        /*destColor=*/   nullptr,
                                        &fsCallbacks);
    return fsCallbacks.fPreamble;
}

const char* MeshRenderStep::fragmentColorSkSLLocalCoordsVariable() const {
    return kMeshFSLocalCoordsName;
}

void MeshRenderStep::writeVertices(DrawWriter* writer,
                                   const DrawParams& params,
                                   uint32_t ssboIndex) const {
    const SkMesh& mesh = params.geometry().mesh();
    DrawBufferManager* bufferManager = writer->bufferManager();

    BindBufferInfo vertexBindInfo;
    if (mesh.vertexBuffer()) {
        auto* cpuVertexBuffer = static_cast<const SkMeshPriv::CpuVertexBuffer*>(mesh.vertexBuffer());
        size_t vertexStride = mesh.spec()->stride();
        size_t vertexCount = mesh.vertexCount();
        const void* vertexData = static_cast<const void*>(static_cast<const uint8_t*>(
                                    cpuVertexBuffer->peek()) + mesh.vertexOffset());

        auto [vertexWriter, vbBindInfo, vbAlloc] =
            bufferManager->getMappedVertexBuffer(vertexCount, vertexStride,
                                                 /*reservedCount=*/0, /*alignment=*/4);
        vertexWriter.write(vertexData, vertexCount * vertexStride);
        vertexBindInfo = vbBindInfo;
    }

    BindBufferInfo indexBindInfo;
    if (mesh.indexBuffer()) {
        auto* cpuIndexBuffer = static_cast<const SkMeshPriv::CpuIndexBuffer*>(mesh.indexBuffer());
        size_t indexCount = mesh.indexCount();
        const void* indexData = static_cast<const void*>(static_cast<const uint8_t*>(
                                    cpuIndexBuffer->peek()) + mesh.indexOffset());

        auto [indexWriter, ibBindInfo, ibAlloc] = bufferManager->getMappedIndexBuffer(indexCount);
        indexWriter.write(indexData, indexCount * sizeof(uint16_t));
        indexBindInfo = ibBindInfo;
    }

    constexpr size_t kInstanceStride = 3 * 3 * sizeof(float) + sizeof(uint32_t);

    const SkM44& m = params.transform().matrix();
    auto [bufferWriter, instanceBindInfo, instanceAlloc] =
        bufferManager->getMappedVertexBuffer(1, kInstanceStride, /*reservedCount=*/0, /*alignment=*/4);
    VertexWriter instanceWriter(std::move(bufferWriter));
    instanceWriter << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)  // mat0
                   << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)  // mat1
                   << m.rc(0,3) << m.rc(1,3) << m.rc(3,3)  // mat2
                   << ssboIndex;

    if (mesh.indexBuffer()) {
        writer->drawIndexedInstanced(vertexBindInfo, indexBindInfo, mesh.indexCount(),
                                     instanceBindInfo, 1);
    } else {
        writer->drawInstanced(vertexBindInfo, mesh.vertexCount(), instanceBindInfo, 1);
    }
}

void MeshRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                              PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(gatherer->checkRewind());
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)
    const SkMesh& mesh = params.geometry().mesh();
    sk_sp<const SkData> transformedUniforms = SkRuntimeEffectPriv::TransformUniforms(
            fSpec->uniforms(), mesh.refUniforms(), SkMeshSpecificationPriv::ColorSpace(*fSpec));
    for (const Uniform& u: this->uniforms()) {
        gatherer->write(u, transformedUniforms->bytes() + fSpec->findUniform(u.name())->offset);
    }
}

MeshRenderStep::HashKey::HashKey(const SkMesh& mesh)
    : fPrimitiveType(PrimitiveTypeFromSkMeshMode(mesh.mode()))
    , fMeshSpec(mesh.refSpec()) {}

}  // namespace skgpu::graphite
