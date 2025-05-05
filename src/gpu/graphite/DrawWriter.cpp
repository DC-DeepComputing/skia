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
        , fStaticStride(0)
        , fAppendStride(0)
        , fAppend()
        , fPendingStaticBaseVertices(0)
        , fPendingStaticBaseIndices(0)
        , fBoundAppend()
        , fPendingIsIndexed(false)
        , fTemplateCount(0)
        , fPendingCount(0) {
    SkASSERT(commandList && bufferManager);
}

void DrawWriter::setTemplate(BindBufferInfo staticVertices,
                             BindBufferInfo staticIndices,
                             BindBufferInfo append,
                             uint32_t templateCount) {
    if (fPendingCount == 0) {
        // A pendingCount of zero indicates that a either a newPipelineState() or dynamicState()
        // triggered a flush; thus we want to update the append buffer to the incoming buffer.
        fAppend = append;
        fTemplateCount = templateCount;
        fPendingIsIndexed = SkToBool(staticIndices.fBuffer);
        fPendingStaticBaseVertices = staticVertices.fBuffer ?
                                     staticVertices.fOffset / fStaticStride : 0;
        fPendingStaticBaseIndices = fPendingIsIndexed ?
                                    staticIndices.fOffset / sizeof(uint16_t) : 0;
    } else {
        // IF there is non-zero pending count, we haven't called newPipelineState() yet, and we
        // aren't expecting RenderSteps to append data with different buffers but the same pipeline.
        // IF a new buffer is acquired in reserve() BUT a subsequent setTemplate is called without a
        // newPipelineState() or dynamicState() call, the pendingCount will still be zero due to the
        // newBuffer flush.
        SkASSERT(fAppend == append);
        SkASSERT((templateCount == 0 &&
                 (fRenderState & RenderStateFlags::kAppendDynamicInstances)) ||
                 fTemplateCount == templateCount);
    }
    SkASSERT(fPendingStaticBaseIndices * sizeof(uint16_t) == staticIndices.fOffset &&
             fPendingStaticBaseVertices * fStaticStride == staticVertices.fOffset);
}

void DrawWriter::flushInternal() {
    // Skip flush if no items appended, or dynamic instances resolved to zero count.
    if (fPendingCount == 0 ||
        ((fRenderState & RenderStateFlags::kAppendDynamicInstances) && fTemplateCount == 0)) {
        return;
    }

    // Calculate base offsets from buffer info for the draw commands.
    // - If a valid bufferFoo exists and  matches the current stride, use pendingBaseFoo as a
    //   pseudo-alias for offset and reset the offset and size before assigning to boundBufferFoo.
    // - If a valid bufferFoo does not exist, or is not stride aligned, then draw from the start of
    //   the offset with pendingBaseFoo = 0 and assign the current buffer.
    uint32_t pendingBaseAppend = 0;
    if (fAppend.fBuffer) {
        BindBufferInfo newBinding = fAppend;
        if (fAppend.fOffset % fAppendStride == 0) {
            pendingBaseAppend = fAppend.fOffset / fAppendStride;
            newBinding = {fAppend.fBuffer, 0, SkTo<uint32_t>(fAppend.fBuffer->size())};
        }
        if (fBoundAppend != newBinding) {
            fBoundAppend = newBinding;
            fCommandList->bindAppendDataBuffer(fBoundAppend);
        }
    }

    // Before any draw commands are added, check if the DrawWriter has an assigned barrier type
    // to issue prior to draw calls.
    if (fBarrierToIssueBeforeDraws.has_value()) {
        fCommandList->addBarrier(fBarrierToIssueBeforeDraws.value());
    }

    // Issue the appropriate draw call (instanced vs. non-instanced) based on the current
    // fTemplateCount. Because of the initial AppendDynamicInstances && fTemplateCount check, any
    // DynamicInstance render step must have valid (positive) templateCount data at this point; thus
    // this check covers RenderStateFlags:: AppendInstances, AppendDynamicInstances, and Fixed.
    if (fTemplateCount) {
        SkASSERT((pendingBaseAppend + fPendingCount)*fAppendStride <= fBoundAppend.fSize);
        if (fPendingIsIndexed) {
            // It's not possible to validate that the indices stored in fIndices access only valid
            // data within fVertices. Simply validate that fIndices holds enough data for the
            // vertex count that's drawn.
            // SkASSERT(fTemplateCount*sizeof(uint16_t) <= fBoundIndices.fSize);
            fCommandList->drawIndexedInstanced(fPrimitiveType,
                                               fPendingStaticBaseIndices,
                                               fTemplateCount,
                                               fPendingStaticBaseVertices,
                                               pendingBaseAppend,
                                               fPendingCount);

        } else {
            // SkASSERT(fTemplateCount*fStaticStride <= fBoundStatic.fSize);
            fCommandList->drawInstanced(fPrimitiveType, fPendingStaticBaseVertices, fTemplateCount,
                                        pendingBaseAppend, fPendingCount);
        }

        // Clear instancing template state after the draw is recorded for non-Fixed RenderState
        if (fRenderState &
            (RenderStateFlags::kAppendInstances | RenderStateFlags::kAppendDynamicInstances)) {
            fAppend = {};
            if (fRenderState & RenderStateFlags::kAppendDynamicInstances) {
                fTemplateCount = 0;
            }
        }
    } else {
        // Issue non-instanced draw call (indexed or non-indexed).
        if (fPendingIsIndexed) {
            // As before, just validate there is sufficient index data
            // SkASSERT(fPendingCount*sizeof(uint16_t) <= fBoundIndices.fSize);
            fCommandList->drawIndexed(fPrimitiveType,
                                      fPendingStaticBaseIndices,
                                      fPendingCount,
                                      pendingBaseAppend);
        } else {
            SkASSERT((pendingBaseAppend + fPendingCount)*fStaticStride <= fBoundAppend.fSize);
            fCommandList->draw(fPrimitiveType, pendingBaseAppend, fPendingCount);
        }

        // Clear vertex template state after the draw is recorded for non-Fixed RenderState
        if (fRenderState & RenderStateFlags::kAppendVertices) {
            fAppend = {};
        }
    }

    // Mark all appended items as drawn.
    fPendingCount = 0;
}

} // namespace skgpu::graphite
