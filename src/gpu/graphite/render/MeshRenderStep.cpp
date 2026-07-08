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

static constexpr char kMeshVSMainName[] = "drawMeshVSMain";
static constexpr char kMeshFSMainName[] = "drawMeshFSMain";

MeshRenderStep::MeshRenderStep(RenderStepID renderStepId, Layout layout,
                               sk_sp<SkMeshSpecification> spec, SkMesh::Mode mode,
                               std::vector<Attribute> attrs, std::vector<Varying> varyings,
                               std::vector<Uniform> uniforms)
        : RenderStep(layout,
                     renderStepId,
                     Flags::kPerformsShading | Flags::kEmitsPrimitiveColor | Flags::kAppendInstances,
                     /*uniforms=*/uniforms,
                     PrimitiveTypeFromSkMeshMode(mode),
                     kDirectDepthLEqualPass,
                     /*staticAttrs=*/{},
                     /*appendAttrs=*/{
                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3}
                     },
                     varyings)
        , fSpec(spec)
        , fAttributes(attrs) {
    const SkSL::Program* vsProgram = SkMeshSpecificationPriv::VS(*fSpec);
    MeshProgramCallbacks vsCallbacks(kMeshVSMainName);
    SkSL::PipelineStage::ConvertProgram(*vsProgram,
                                        /*sampleCoords=*/nullptr,
                                        /*inputColor=*/  nullptr,
                                        /*destColor=*/   nullptr,
                                        &vsCallbacks);
    fVSTopLevel = vsCallbacks.fPreamble;

    const SkSL::Program* fsProgram = SkMeshSpecificationPriv::FS(*fSpec);
    for (const std::unique_ptr<SkSL::ProgramElement>& e: fsProgram->fOwnedElements) {
        if (e->is<SkSL::FunctionDefinition>()) {
            const SkSL::FunctionDeclaration& f = e->as<SkSL::FunctionDefinition>().declaration();
            if (f.isMain()) {
                for (const SkSL::Variable* v: f.parameters()) {
                    if (v->name() == "color" && v->type().name() == "float4") {
                        fFSColorConversion = true;
                    }
                }
            }
        }
    }

    MeshProgramCallbacks fsCallbacks(kMeshFSMainName);
    SkSL::PipelineStage::ConvertProgram(*fsProgram,
                                        /*sampleCoords=*/nullptr,
                                        /*inputColor=*/  nullptr,
                                        /*destColor=*/   nullptr,
                                        &fsCallbacks);
    fFSTopLevel = fsCallbacks.fPreamble;
}

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
    s += SkSL::String::printf("%s(varyings, %s);\n", kMeshFSMainName, outColorName.c_str());
    if (fFSColorConversion) {
        s += "primitiveColor = half4(" + outColorName + ");\n";
    }
    return s;
}

std::string MeshRenderStep::fragmentColorSkSLTopLevel() const {
    return fFSTopLevel;
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

        // TODO(nathanasanchez) figure out the alignment in a better way.
        auto [indexWriter, ibBindInfo, ibAlloc] = bufferManager->getMappedIndexBuffer(indexCount);
        indexWriter.write(indexData, indexCount * sizeof(uint16_t));
        indexBindInfo = ibBindInfo;
    }

    const SkM44& m = params.transform().matrix();
    if (mesh.indexBuffer()) {
        DrawWriter::Instances instance(*writer, vertexBindInfo, indexBindInfo, mesh.indexCount());
        auto vw = instance.append(1);
        vw << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)  // mat0
           << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)  // mat1
           << m.rc(0,3) << m.rc(1,3) << m.rc(3,3); // mat2
        //writer->drawIndexed(
        //        vertexBindInfo,
        //        indexBindInfo,
        //        mesh.indexCount());
    } else {
        DrawWriter::Instances instance(*writer, vertexBindInfo, {}, mesh.vertexCount());
        auto vw = instance.append(1);
        vw << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)  // mat0
           << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)  // mat1
           << m.rc(0,3) << m.rc(1,3) << m.rc(3,3); // mat2
        //writer->draw(
        //        vertexBindInfo,
        //        mesh.vertexCount());
    }

    // Need to call or assert at DrawWriter.h:299 cause different static buffer, cause of
    // DrawWriter::Instances constructor?
    //writer->flush();
}

void MeshRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                              PipelineDataGatherer* gatherer) const {
    SkDEBUGCODE(gatherer->checkRewind());
    SkDEBUGCODE(UniformExpectationsValidator uev(gatherer, this->uniforms());)
    gatherer->write(params.transform().matrix());
}

MeshRenderStep::HashKey::HashKey(const SkMesh& mesh)
    : fPrimitiveType(PrimitiveTypeFromSkMeshMode(mesh.mode()))
    , fMeshSpec(mesh.refSpec()) {}

}  // namespace skgpu::graphite
