/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawWriter.h"

#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/DrawCommands.h"

namespace skgpu::graphite {

DrawWriter::DrawWriter(DrawPassCommands::List* commandList, DrawBufferManager* bufferManager)
        : fCommandList(commandList)
        , fManager(bufferManager)
        , fRenderState(RenderStateFlags::kNone)
        , fPrimitiveType(PrimitiveType::kTriangles)
        , fVertexStride(0)
        , fInstanceStride(0)
        , fVertices()
        , fIndices()
        , fInstances()
        , fBoundVertices()
        , fBoundIndices()
        , fBoundInstances()
        , fTemplateCount(0)
        , fPendingCount(0) {
    SkASSERT(commandList && bufferManager);
}

void DrawWriter::setTemplate(BindBufferInfo vertices,
                             BindBufferInfo indices,
                             BindBufferInfo instances,
                             unsigned int templateCount) {
    bool isDynamic = SkToBool(fRenderState & RenderStateFlags::kAppendDynamicInstances);
    bool willBind = vertices != fVertices || instances != fInstances || indices != fIndices;
    // Flush pending data if incompatible with the new template settings.
    if (fPendingCount && (willBind || (templateCount != fTemplateCount && !isDynamic))) {
        this->flush();
    }

    // Update the writer's template state.
    fVertices = vertices;
    fInstances = instances;
    fIndices = indices;

    // For dynamic instances, fTemplateCount is finalized during flush/destruction.
    if (!isDynamic) {
        fTemplateCount = templateCount;
    }
}

void DrawWriter::flush() {
    // Skip flush if no items appended, or dynamic instances resolved to zero count.
    if (fPendingCount == 0 ||
        ((fRenderState & RenderStateFlags::kAppendDynamicInstances) && fTemplateCount == 0)) {
        return;
    }

    // Calculate base offsets from buffer info for the draw commands.
    uint32_t pendingBaseIndices = 0;
    uint32_t pendingBaseVertex = 0;
    uint32_t pendingBaseInstance = 0;

    if (fVertices.fBuffer) {
        if (fVertices.fOffset % fVertexStride == 0) {
            pendingBaseVertex = fVertices.fOffset / fVertexStride;
            fVertices.fOffset = 0;
            fVertices.fSize = fVertices.fBuffer->size();
        } // else: Offset isn't stride-aligned, draw from start (base=0).
    }

    if (fInstances.fBuffer) {
        if (fInstances.fOffset % fInstanceStride == 0) {
            pendingBaseInstance = fInstances.fOffset / fInstanceStride;
            fInstances.fOffset = 0; // Reset offset/size for comparison against fBoundInstances.
            fInstances.fSize = fInstances.fBuffer->size();
        } // else: Offset isn't stride-aligned, draw from start (base=0).
    }

    if (fIndices.fBuffer) {
        if (fIndices.fOffset % sizeof(uint16_t) == 0) {
            pendingBaseIndices = fIndices.fOffset / sizeof(uint16_t);
            fIndices.fOffset = 0;
            fIndices.fSize = fIndices.fBuffer->size();
        } // else: Offset isn't aligned, draw from start (base=0).
    }

    // Bind buffers only if they differ from the last known bound state.
    if ((fVertices.fBuffer && fBoundVertices != fVertices) ||
        (fInstances.fBuffer && fBoundInstances != fInstances) ||
        (fIndices.fBuffer && fIndices != fBoundIndices)) {
        if (fVertices.fBuffer) { fBoundVertices = fVertices; }
        if (fInstances.fBuffer) { fBoundInstances = fInstances; }
        if (fIndices.fBuffer) { fBoundIndices = fIndices; }
        fCommandList->bindDrawBuffers(fBoundVertices, fInstances, fIndices, {});
    }

    // Issue the appropriate draw call (instanced vs. non-instanced) based on renderState.
    if (fRenderState &
        (RenderStateFlags::kAppendInstances | RenderStateFlags::kAppendDynamicInstances)) {
        SkASSERT((pendingBaseInstance + fPendingCount)*fInstanceStride <= fInstances.fSize);
        if (fIndices) {
            // It's not possible to validate that the indices stored in fIndices access only valid
            // data within fVertices. Simply validate that fIndices holds enough data for the
            // vertex count that's drawn.
            SkASSERT(fTemplateCount*sizeof(uint16_t) <= fIndices.fSize);
            fCommandList->drawIndexedInstanced(fPrimitiveType,
                                               pendingBaseIndices,
                                               fTemplateCount,
                                               pendingBaseVertex,
                                               pendingBaseInstance,
                                               fPendingCount);

        } else {
            SkASSERT(fTemplateCount*fVertexStride <= fVertices.fSize);
            fCommandList->drawInstanced(fPrimitiveType, pendingBaseVertex, fTemplateCount,
                                        pendingBaseInstance, fPendingCount);
        }

        // Clear instancing template state after the draw is recorded.
        fInstances = {};
        fTemplateCount = 0;
    } else {
        // Issue non-instanced draw call (indexed or non-indexed).
        SkASSERT(!fInstances); // Should not have an instance buffer in non-instanced mode.
        if (fIndices) {
            // As before, just validate there is sufficient index data
            SkASSERT(fPendingCount*sizeof(uint16_t) <= fIndices.fSize);
            fCommandList->drawIndexed(fPrimitiveType,
                                      pendingBaseIndices,
                                      fPendingCount,
                                      pendingBaseVertex);
        } else {
            SkASSERT((pendingBaseVertex + fPendingCount)*fVertexStride <= fVertices.fSize);
            fCommandList->draw(fPrimitiveType, pendingBaseVertex, fPendingCount);
        }

        // Clear vertex template state after the draw is recorded.
        fVertices = {};
    }

    // Mark all appended items as drawn.
    fPendingCount = 0;
}

} // namespace skgpu::graphite
