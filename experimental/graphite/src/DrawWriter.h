/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DrawWriter_DEFINED
#define skgpu_DrawWriter_DEFINED

#include "experimental/graphite/src/DrawTypes.h"
#include "src/gpu/BufferWriter.h"

namespace skgpu {

class DrawBufferManager;

class DrawDispatcher; // Forward declaration, handles virtual dispatch of binds/draws

/**
 * DrawWriter is a helper around recording draws (to a temporary buffer or directly to a
 * CommandBuffer), particularly when the number of draws is not known ahead of time, or the vertex
 * and instance data is computed at record time and does not have a known size.
 *
 * Usage for a single, already bound pipeline:
 *
 *     DrawWriter writer(commandBuffer, bufferManager, primitiveType, vertexStride, instanceStride);
 *     writer.setInstanceTemplate(...);
 *     writer.appendInstances(2) << ...
 *                               << ...;
 *     // ~DrawWriter will bind dynamic buffers and make draw calls for appended data
 *
 * Usage in a loop that may change state:
 *
 *     DrawWriter writer(commandBuffer, bufferManager);
 *     for (...) {
 *         // determine if state or pipeline need to change, needsPipelineChange needs to be true
 *         // on the first iteration in order to get a valid DrawWriter.
 *         if (needsStateChange || needsPipelineChange) {
 *             // ~DrawWriter() of old value pushes pending draws before the state change
 *             writer = needsPipelineChange ? writer.forNewPipeline(nextType, ...) :
 *                                            writer.forDynamiceStateChange();
 *
 *             // bind pipeline and make state changes here
 *         }
 *
 *         writer.setIntanceTemplate(...);
 *         writer.appendInstances(...) << ...; // using data for current iteration
 *     }
 *     // ~DrawWriter at the end of scope will push any remaining pending draws
 *
 *
 * CommandBuffer::draw(vertices)
 *  - dynamic vertex data     -> DrawWriter::appendVertices(n)
 *  - fixed vertex data       -> DrawWriter::draw(vertices, {}, vertexCount)
 *
 * CommandBuffer::drawIndexed(vertices, indices)
 *  - dynamic vertex data     -> unsupported
 *  - fixed vertex,index data -> DrawWriter::draw(vertices, indices, indexCount)
 *
 * CommandBuffer::drawInstances(vertices, instances)
 *  - dynamic instance data + fixed vertex data        ->
 *          DrawWriter::setInstanceTemplate(vertices, {}, vertexCount) then
 *          DrawWriter::appendInstances(n)
 *  - fixed vertex and instance data                   ->
 *          DrawWriter::setInstanceTemplate(vertices, {}, vertexCount) then
 *          DrawWriter::drawInstanced(instances, instanceCount)
 *
 * CommandBuffer::drawIndexedInstanced(vertices, indices, instances)
 *  - dynamic instance data + fixed vertex, index data ->
 *          DrawWriter::setInstanceTemplate(vertices, indices, indexCount) then
 *          DrawWriter::appendInstances(n)
 *  - fixed vertex, index, and instance data           ->
 *          DrawWriter::setInstanceTemplate(vertices, indices, indexCount) then
 *          DrawWriter::drawInstanced(instances, instanceCount)
 */
class DrawWriter {
public:
    // NOTE: This constructor creates a writer that has 0 vertex and instance stride, so can only
    // be used to draw triangles with pipelines that rely solely on the vertex and instance ID.
    DrawWriter(DrawDispatcher*, DrawBufferManager*);

    DrawWriter(DrawDispatcher*, DrawBufferManager*,
               PrimitiveType type, size_t vertexStride, size_t instanceStride);

    DrawWriter(DrawWriter&& that) {
        *this = static_cast<const DrawWriter&>(that);
        that.fPendingCount = 0;
    }

    // RAII - Issues draws for collected vertex or instance data and returns any unused reserved
    // data back to the resource manager.
    ~DrawWriter();

    DrawWriter& operator=(DrawWriter&& that) {
        // "this" is being overwritten, so draw everything that had been collected.
        this->drawPendingVertices();
        *this = static_cast<const DrawWriter&>(that); // use private copy assignment
        that.fPendingCount = 0;
        return *this;
    }

    // Return a new DrawWriter that picks up after this writer has been destroyed, with the intent
    // that dynamic state (but not a pipeline change) has been made after this writer is gone and
    // before the returned writer accumulates draws.
    //
    // This preserves the last bound buffers and accounts for any offsets using the base vertex or
    // base instance passed to draw calls to avoid re-binding buffers unnecessarily.
    DrawWriter forDynamicStateChange() const;

    // Return a new DrawWriter that picks up after this writer has been destroyed, with the intent
    // that a new GraphicsPipeline will have been bound after this writer is gone and before the
    // returned writer accumulates draws.
    DrawWriter forNewPipeline(PrimitiveType type, size_t vertexStride, size_t instanceStride) const;

    // Collects new vertex data for a call to CommandBuffer::draw(). Automatically accumulates
    // vertex data into a buffer, issuing draw and bind calls as needed when a new buffer is
    // required, so that it is seamless to the caller.
    //
    // Since this accumulates vertex data (and does not use instances or indices), this overrides
    // the instance template when finally drawn.
    //
    // This should not be used when the vertex stride is 0.
    VertexWriter appendVertices(unsigned int numVertices) {
        SkASSERT(fVertexStride > 0);
        return this->appendData(VertexMode::kVertices, fVertexStride, numVertices);
    }

    // Collects new instance data for a call to CommandBuffer::drawInstanced() or
    // drawIndexedInstanced(). The specific draw call that's issued depends on the buffers passed to
    // setInstanceTemplate(). If the template has a non-null index buffer, the eventual draw calls
    // correspond to drawindexedInstanced(), otherwise to drawInstanced().
    //
    // Like appendVertices(), this automatically manages an internal instance buffer and merges
    // the appended data into as few buffer binds and draw calls as possible, while remaining
    // seamless to the caller.
    //
    // This requires that an instance template be specified before appending instance data. However,
    // the fixed vertex buffer can be null (or have a stride of 0) if the vertex shader only relies
    // on the vertex ID and no other per-vertex data.
    //
    // This should not be used when the instance stride is 0.
    VertexWriter appendInstances(unsigned int numInstances) {
        SkASSERT(fInstanceStride > 0);
        return this->appendData(VertexMode::kInstances, fInstanceStride, numInstances);
    }

    // Set the fixed vertex and index buffers referenced when appending instance data or calling
    // drawIndexed(). 'count' is the number of vertices in the template, which is either the
    // vertex count (when 'indices' has a null buffer), or the index count when 'indices' are
    // provided.
    void setInstanceTemplate(BindBufferInfo vertices, BindBufferInfo indices, unsigned int count);

    // Issues a draw with fully specified data. This can be used when all instance data has already
    // been written to known buffers, or when the vertex shader only depends on the vertex or
    // instance IDs.
    //
    // The specific draw call issued depends on the buffers set via 'setInstanceTemplate' and the
    // 'instances' parameter. If the template has a non-null index buffer, it will use
    // drawIndexedInstanced(), otherwise it will use drawInstanced().
    //
    // This will not merge with any already appended instance or vertex data, pending data is issued
    // in its own draw call first.
    void drawInstanced(BindBufferInfo instances, unsigned int count) {
        this->drawPendingVertices();
        this->drawInternal(instances, 0, count);
    }

    // Issues a non-instanced draw call with existing, fully specified data. The specific draw call
    // depends on the buffers passed to this function. If a non-null index buffer is specified, it
    // will use drawIndexed(), otherwise it will use the vertex-only draw().
    //
    // This will not merge with any existing appended instance or vertex data, which will issue it
    // own draw call. This overrides what was last set for the instance template.
    void draw(BindBufferInfo vertices, BindBufferInfo indices, unsigned int count) {
        this->setInstanceTemplate(vertices, indices, count); // will draw pending if needed
        this->drawInstanced({}, 1);
    }

private:
    enum class VertexMode : unsigned {
        kVertices, kInstances
    };

    // Used internally, public move semantics are effectively copying + management of pending count.
    DrawWriter(const DrawWriter&) = default;
    DrawWriter& operator=(const DrawWriter&) = default;

    // Both of these pointers must outlive the DrawWriter.
    DrawDispatcher*    fDispatcher;
    DrawBufferManager* fManager;

    // Must be constructed to match the pipeline that's bound
    PrimitiveType fPrimitiveType;
    size_t fVertexStride;
    size_t fInstanceStride;

    // State tracking appended vertices or instances
    VertexMode     fPendingMode       = VertexMode::kVertices;
    unsigned int   fPendingCount      = 0; // vertex or instance count depending on mode
    unsigned int   fPendingBaseVertex = 0; // or instance
    BindBufferInfo fPendingAttrs      = {};

    // State to track the instance template that is re-used across drawn instances. These are not
    // yet bound if fFixedBuffersDirty is true. Non-instanced draw buffers (e.g. draw() and
    // drawIndexed()) are treated as drawing one instance, with no extra instance attributes.
    BindBufferInfo fFixedVertexBuffer = {};
    BindBufferInfo fFixedIndexBuffer  = {};
    unsigned int   fFixedVertexCount  = 0; // or index count if fFixedIndexBuffer is non-null

    bool fFixedBuffersDirty = true;

    // Will either be 'fPendingAttrData' or the arg last passed to drawInstanced(), since it may
    // change even if the fixed vertex and index buffers have not.
    BindBufferInfo fLastInstanceBuffer = {};

    VertexWriter appendData(VertexMode mode, size_t stride, unsigned int count);
    void drawInternal(BindBufferInfo instances, unsigned int base, unsigned int instanceCount);
    void drawPendingVertices();
};

// Mirrors the CommandBuffer API, since a DrawWriter is meant to aggregate and then map onto
// CommandBuffer commands, although these are virtual to allow for recording to intermediate
// storage before a CommandBuffer is available.
class DrawDispatcher {
public:
    virtual ~DrawDispatcher() = default;

    virtual void bindVertexBuffers(BindBufferInfo vertexAttribs,
                                   BindBufferInfo instanceAttribs,
                                   BindBufferInfo indices) = 0;

    virtual void draw(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount) = 0;
    virtual void drawIndexed(PrimitiveType type, unsigned int baseVertex,
                             unsigned int baseIndex, unsigned int indexCount) = 0;
    virtual void drawInstanced(PrimitiveType type,
                               unsigned int baseVertex, unsigned int vertexCount,
                               unsigned int baseInstance, unsigned int instanceCount) = 0;
    virtual void drawIndexedInstanced(PrimitiveType type,
                                      unsigned int baseVertex, unsigned int baseIndex,
                                      unsigned int indexCount, unsigned int baseInstance,
                                      unsigned int instanceCount) = 0;
};

} // namespace skgpu

#endif // skgpu_DrawWriter_DEFINED
