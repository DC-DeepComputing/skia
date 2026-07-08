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
#include "include/private/SkTo.h"
#include "src/core/SkMeshPriv.h"
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

MeshRenderStep::MeshRenderStep(RenderStepID renderStepId, Layout layout,
                               sk_sp<SkMeshSpecification> spec, SkMesh::Mode mode,
                               std::vector<Attribute> attrs, std::vector<Varying> varyings,
                               std::vector<Uniform> uniforms, std::string vsTopLevelSkSL,
                               std::string fsTopLevelSkSL, bool hasColorOutput,
                               bool needsColorConversion)
        : RenderStep(layout,
                     renderStepId,
                     Flags::kPerformsShading
                        | Flags::kAppendInstances
                        | (hasColorOutput ? Flags::kEmitsPrimitiveColor : Flags::kNone),
                     /*uniforms=*/{},
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
        , fVSTopLevel(vsTopLevelSkSL)
        , fFSTopLevel(fsTopLevelSkSL)
        , fFSHasColorOutput(hasColorOutput)
        , fFSColorConversion(needsColorConversion) {}

MeshRenderStep::~MeshRenderStep() {}

std::string MeshRenderStep::vertexSkSL() const {
    std::string attrs = "Attributes attributes;\n";
    for (const Attribute& attr: fAttributes) {
        attrs = attrs.append(SkSL::String::printf("attributes.%s = %s;\n",
                             attr.name(), attr.name()));
    }

    std::string varyingAssignments;
    for (const Varying& v: this->varyings()) {
        varyingAssignments += SkSL::String::printf("%s = varyings.%s;\n", v.name(), v.name());
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

std::string MeshRenderStep::vertexSkSLTopLevel() const {
    return fVSTopLevel;
}

std::string MeshRenderStep::fragmentColorSkSL() const {
    std::string s = "Varyings varyings;\n";
    for (const Varying& v: this->varyings()) {
        s += SkSL::String::printf("varyings.%s = %s;\n", v.name(), v.name());
    }

    std::string outColorName = "primitiveColor";
    if (fFSColorConversion) {
        outColorName = "primitiveColorFloat4";
        s += "float4 " + outColorName + ";\n";
    }

    std::string methodCall = fFSHasColorOutput ? SkSL::String::printf("%s(varyings, %s)",
                                                                      kMeshFSMainName,
                                                                      outColorName.c_str())
                                               : SkSL::String::printf("%s(varyings)",
                                                                      kMeshFSMainName);

    s += SkSL::String::printf("float2 %s = %s;\n",
                              kMeshFSLocalCoordsName,
                              methodCall.c_str());
    if (fFSColorConversion) {
        s += "primitiveColor = half4(" + outColorName + ");\n";
    }
    return s;
}

std::string MeshRenderStep::fragmentColorSkSLTopLevel() const {
    return fFSTopLevel;
}

const char* MeshRenderStep::fragmentColorSkSLLocalCoordsVariable() const {
    return kMeshFSLocalCoordsName;
}

void MeshRenderStep::writeVertices(DrawWriter* writer,
                                   const DrawParams& params,
                                   uint32_t ssboIndex) const {
    SkASSERT(params.geometry().isMesh());
    const SkMesh& mesh = params.geometry().mesh();
    DrawBufferManager* bufferManager = writer->bufferManager();

    BindBufferInfo vertexBindInfo;
    if (mesh.vertexBuffer()) {
        auto* cpuVertexBuffer = static_cast<const SkMeshPriv::CpuVertexBuffer*>(mesh.vertexBuffer());
        size_t vertexStride = mesh.spec()->stride();
        size_t vertexCount = mesh.vertexCount();
        const void* vertexData = static_cast<const void*>(static_cast<const uint8_t*>(
                                    cpuVertexBuffer->peek()) + mesh.vertexOffset());

        // TODO(nathanasanchez) figure out the alignment in a better way.
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
    //SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)
    //gatherer->write(params.transform().matrix());
}

MeshRenderStep::HashKey::HashKey(const SkMesh& mesh)
    : fPrimitiveType(PrimitiveTypeFromSkMeshMode(mesh.mode()))
    , fMeshSpec(mesh.refSpec()) {}

}  // namespace skgpu::graphite
