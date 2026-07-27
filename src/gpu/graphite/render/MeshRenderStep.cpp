/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/MeshRenderStep.h"

#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

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

static constexpr char kMeshFSLocalCoordsName[] = "meshLocalCoordsOverride";

RenderStep::RenderStepID variant_id(PrimitiveType type) {
    if (type == PrimitiveType::kTriangles) {
        return RenderStep::RenderStepID::kMesh_Tri;
    } else {
        SkASSERT(type == PrimitiveType::kTriangleStrip);
        return RenderStep::RenderStepID::kMesh_Tristrips;
    }
}

MeshRenderStep::MeshRenderStep(Layout layout, PrimitiveType primType)
        : RenderStep(layout,
                     variant_id(primType),
                     Flags::kPerformsShading | Flags::kAppendInstances
                                             | Flags::kEmitsPrimitiveColor,
                     /*uniforms=*/{},
                     primType,
                     kDirectDepthLEqualPass,
                     /*staticAttrs=*/{},
                     /*appendAttrs=*/{
                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}
                     },
                     {}) {}

MeshRenderStep::~MeshRenderStep() {}

std::string MeshRenderStep::vertexSkSL(const RootNodesInfo& roots) const {
    if (!roots.meshSpec) {
        return "";
    }
    const SkMeshSpecification* spec = roots.meshSpec;

    std::string attrs = "Attributes attributes;\n";
    for (const auto& attr : spec->attributes()) {
        attrs += SkSL::String::printf("attributes.%s = %s;\n",
                                      attr.name.c_str(), attr.name.c_str());
    }

    std::string varyingAssignments;
    for (const auto& v : SkMeshSpecificationPriv::Varyings(*spec)) {
        varyingAssignments += SkSL::String::printf("%s%s = varyings.%s;\n",
                                                   v.name.c_str(), kMeshVaryingMangleSuffix,
                                                   v.name.c_str());
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

std::string MeshRenderStep::fragmentColorSkSL(const RootNodesInfo& roots) const {
    if (!roots.meshSpec) {
        return "";
    }
    const SkMeshSpecification* spec = roots.meshSpec;

    std::string s = "Varyings varyings;\n";
    for (const auto& v : SkMeshSpecificationPriv::Varyings(*spec)) {
        s += SkSL::String::printf("varyings.%s = %s%s;\n",
                                  v.name.c_str(), v.name.c_str(),
                                  kMeshVaryingMangleSuffix);
    }

    bool needsColorConversion = SkMeshSpecificationPriv::GetColorType(*spec)
                                         == SkMeshSpecificationPriv::ColorType::kFloat4;
    std::string outColorName = "primitiveColor";
    if (needsColorConversion) {
        outColorName = "primitiveColorFloat4";
        s += "float4 " + outColorName + ";\n";
    }

    // Check if the mesh FS has a color output parameter
    bool hasColorOutput = SkMeshSpecificationPriv::HasColors(*spec);
    std::string methodCall = hasColorOutput ? SkSL::String::printf("%s(varyings, %s)",
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
    const SkMesh& mesh = params.geometry().mesh();
    const SkMeshSpecification* spec = mesh.spec();

    std::vector<std::string> uniformNames;
    std::vector<Uniform> uniforms;
    for (const auto& u : spec->uniforms()) {
        uniformNames.push_back(std::string(u.name));
        uniforms.push_back(Uniform(uniformNames.back().c_str(),
                                   ShaderCodeDictionary::UniformTypeToSkSLType(u),
                                   u.isArray() ? u.count : Uniform::kNonArray));
    }
    SkDEBUGCODE(gatherer->checkRewind());
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, uniforms));

    sk_sp<const SkData> transformedUniforms = SkRuntimeEffectPriv::TransformUniforms(
            spec->uniforms(), mesh.refUniforms(), SkMeshSpecificationPriv::ColorSpace(*spec));
    for (size_t i = 0; i < uniforms.size(); i++) {
        // Since we are doing pointer arithmetic, make sure we are operating on bytes.
        static_assert(std::is_same_v<decltype(transformedUniforms->bytes()), const uint8_t*>);
        gatherer->write(uniforms[i],
                        transformedUniforms->bytes() + spec->uniforms()[i].offset);
    }
}

size_t MeshRenderStep::staticDataStride(const DrawParams& params) const {
    return params.geometry().mesh().spec()->stride();
}

}  // namespace skgpu::graphite
