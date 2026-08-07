/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/MeshRenderStep.h"

#include "src/gpu/graphite/PaintParamsKey.h"

#include "include/private/SkAssert.h"
#include "src/core/SkMeshPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Transform.h"

#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

#include <cstdint>

namespace skgpu::graphite {

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
                     /*appendAttrs=*/{{"ssboIndex", VertexAttribType::kUInt, SkSLType::kUInt}}) {}

MeshRenderStep::~MeshRenderStep() {}

std::string MeshRenderStep::vertexSkSL(const RootNodesInfo& roots) const {
    return "";
}

std::string MeshRenderStep::fragmentColorSkSL(const RootNodesInfo& roots) const {
    return "";
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

    auto [bufferWriter, instanceBindInfo, instanceAlloc] =
        bufferManager->getMappedVertexBuffer(1, kInstanceStride, /*reservedCount=*/0, /*alignment=*/4);
    VertexWriter instanceWriter(std::move(bufferWriter));
    instanceWriter << ssboIndex;

    if (mesh.indexBuffer()) {
        writer->drawIndexedInstanced(vertexBindInfo, indexBindInfo, mesh.indexCount(),
                                     instanceBindInfo, 1);
    } else {
        writer->drawInstanced(vertexBindInfo, mesh.vertexCount(), instanceBindInfo, 1);
    }
}

void MeshRenderStep::writeUniformsAndTextures(const DrawParams& params,
                                              PipelineDataGatherer* gatherer) const {
    // TODO (nathanasanchez): Implement uniform writing.
}

}  // namespace skgpu::graphite
