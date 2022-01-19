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
 * To use, construct the DrawWriter with the current pipeline layout or call newPipelineState() on
 * an existing DrawWriter and then bind that matching pipeline. When other dynamic state needs to
 * change between draw calls, notify the DrawWriter using newDynamicState() before recording the
 * modifications. See the listing below for how to append dynamic data or draw with existing buffers
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

    // Cannot move or copy
    DrawWriter(const DrawWriter&) = delete;
    DrawWriter(DrawWriter&&) = delete;

    // flush() should be called before the writer is destroyed
    ~DrawWriter() { SkASSERT(fPendingCount == 0); }

    DrawBufferManager* bufferManager() { return fManager; }

    // Notify the DrawWriter that dynamic state that does not affect the pipeline needs to be
    // changed. This issues draw calls for pending vertex/instance data that referred to the old
    // state, so this must be called *before* changing the dynamic state.
    //
    // This preserves the last bound buffers and accounts for any offsets using the base vertex or
    // base instance passed to draw calls to avoid re-binding buffers unnecessarily.
    void newDynamicState();

    // Notify the DrawWriter that a new pipeline needs to be bound, providing the primitive type and
    // attribute strides of that pipeline. This issues draw calls for pending data that relied on
    // the old pipeline, so this must be called *before* binding the new pipeline.
    void newPipelineState(PrimitiveType type, size_t vertexStride, size_t instanceStride);

    // Issue draw calls for any pending vertex and instance data collected by the writer.
    void flush() { this->drawPendingVertices(); }

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
    void setInstanceTemplate(BindBufferInfo vertices, BindBufferInfo indices, unsigned int count) {
        this->setTemplateInternal(vertices, indices, count, /*drawPending=*/true);
    }

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
    void setTemplateInternal(BindBufferInfo vertices, BindBufferInfo indices,
                             unsigned int count, bool drawPending);
    void drawInternal(BindBufferInfo instances, unsigned int base, unsigned int instanceCount);
    void drawPendingVertices();
};

class DrawWriter3 {
public:
    class Vertices : private Appender {
    public:
        explicit Vertices(DrawWriter3& w) : Appender(w, true) {
            w.setMode(w.fVertices, {}, {}, 0);
        }

        void reserve(unsigned int count) { this->Appender::reserve(count); }

        VertexWriter append(unsigned int count) { return this->Appender::append(count); }
    };

    class PseudoInstances : private Appender {
    public:
        PseudoInstances(DrawWriter3& w,
                        BindBufferInfo indices,
                        unsigned int vertexCount,
                        unsigned int maxInstances)
                : Appender(w, true)
                , fMaxVertices(vertexCount * maxInstances) {
            SkASSERT(vertexCount > 0 && maxInstances > 0);
            w.setMode(w.fVertices, {}, indices, vertexCount);
        }

        void reserve(unsigned int count) {
            unsigned int newVerts = this->vertexCount(count);
            this->flushIfNeeded(newVerts);
            this->Appender::reserve(newVerts);
        }

        VertexWriter append(unsigned int count) {
            unsigned int newVerts = this->vertexCount(count);
            this->flushIfNeeded(newVerts);
            return this->Appender::append(newVerts);
        }

    private:
        unsigned int vertexCount(unsigned int instanceCount) const {
            unsigned int vertexCount = fWriter.fTemplateCount * instanceCount;
            SkASSERT(vertexCount <= fMaxVertices);
            return vertexCount;
        }

        void flushIfNeeded(unsigned int newVerts) {
            if (fWriter.fPendingCount + newVerts > fMaxVertices) {
                fWriter.flush();
            }
        }

        unsigned int fMaxVertices;
    };

    class Instances : private Appender {
        Instances(DrawWriter3& w,
                  BindBufferInfo vertices,
                  BindBufferInfo indices,
                  unsigned int vertexCount)
                : Appender(w, false) {
            SkASSERT(vertexCount > 0);
            w.setMode(vertices, w.fInstances, indices, vertexCount);
        }

        void reserve(unsigned int count) { this->Appender::reserve(count); }

        VertexWriter append(unsigned int count) { return this->Appender::append(count); }
    };

    class DynamicInstances {
        DynamicInstances(DrawWriter3& w,
                         BindBufferInfo vertices,
                         BindBufferInfo indices)
                : Appender(w, false) {
            w.setMode(vertices, w.fInstances, indices, -1);
        }

        void reserve(unsigned int count) { this->Appender::reserve(count); }

        VertexWriter append(unsigned int vertexCount, unsigned int instanceCount) {
            SkASSERT(vertexCount > 0);
            VertexWriter newInstances = this->Appender::append(instanceCount);
            fWriter.fTemplateCount = std::min(fWriter.fTemplateCount, -vertexCount - 1);
        }
    };

    void draw(BindBufferInfo vertices, unsigned int count) {
        this->setMode(vertices, {}, {}, 0);
        fPendingBase = 0;
        fPendingCount = count;
        this->flush();
    }

    void drawIndexed(BindBufferInfo vertices, BindBufferInfo indices, unsigned int count) {
        this->setMode(vertices, {}, indices, 0);
        fPendingBase = 0;
        fPendingCount = count;
        this->flush();
    }

    void drawInstanced(BindBufferInfo vertices, unsigned int vertexCount, BindBufferInfo instances, unsigned int count) {
        this->setMode(vertices, instances, {}, vertexCount);
        fPendingBase = 0;
        fPendingCount = count;
        this->flush();
    }

    void drawIndexedInstanced(BindBufferInfo vertices, BindBufferInfo indices, unsigned int vertexCount, BindBufferInfo instances, unsigned int count) {
        this->setMode(vertices, instances, indices, vertexCount);
        fPendingBase = 0;
        fPendingCount = count;
        this->flush();
    }

    void newDynamicState() { this->flush(); }

    void newPipelineState(PrimitiveType type, size_t vertexStride, size_t instanceStride) {
        this->flush();

        fPrimitiveType = type;
        fVertexStride = vertexStride;
        fInstanceStride = instanceStride;

        fPendingBase = 0;
        SkASSERT(fPendingCount == 0);
    }

private:
    // Pipeline state
    PrimitiveType fPrimitiveType;
    size_t fVertexStride;
    size_t fInstanceStride;

    // Draw buffer binding state for pending draws
    BindBufferInfo fVertices;
    BindBufferInfo fIndices;
    BindBufferInfo fInstances;
    // Sign-encoded vertex/index count for [pseudo]-instanced rendering.
    // == 0 is vertex-only drawing; > 0 is regular or pseudo-instanced drawing; < 0 is the current
    // vertex count for dynamic instances, as (-count - 1).
    int fTemplateCount;

    unsigned int fPendingCount; // # of vertices or instances (depending on mode) to be drawn
    unsigned int fPendingBase; // vertex/instance offset (depending on mode) applied to buffer
    bool fPendingBufferBinds; // true if {fVertices,fIndices,fInstances} has changed since last draw

    void setMode(BindBufferInfo vertices, BindBufferInfo instances, BindBufferInfo indices,
                 int vertexCount) {
        if (vertices != fVertices || instances != fInstances || fIndices != indices) {
            if (fPendingCount > 0) {
                this->flush();
            }

            bool willAppendVertices = vertexCount == 0;
            bool isAppendingVertices = fTemplatecount == 0;
            if (willAppendVertices != isAppendingVertices ||
                (isAppendingVertices && fVertices != vertices) ||
                (!isAppendingVertices && fInstances != instances)) {
                // The buffer binding target for appended data is changing, so reset the base offset
                fPendingBase = 0;
            }

            fVertices = vertices;
            fInstances = instances;
            fIndices = indices;

            fTemplateCount = vertexCount;

            fPendingBufferBinds = true;
        } else if (vertexCount < 0 && fTemplateCount >= 0) || (vertexCount >= 0 && vertexCount != fTemplateCount)) {
            if (fPendingCount > 0) {
                this->flush();
            }
            if ((vertexCount == 0) != (fTemplateCount == 0)) {
                // Switching from appending vertices to instances, or vice versa, so the pending
                // base vertex for appended data is invalid
                fPendingBase = 0;
            }
            fTemplateCount = vertexCount;
        }

        SkASSERT(fVertices == vertices);
        SkASSERT(fInstances == instances);
        SkASSERT(fIndices == indices);
        SkASSERT((vertexCount < 0) == (fTemplateCount < 0));
        SkASSERT((fTemplateCount < 0 && vertexCount == -1) || (fTemplateCount >= 0 && fTemplateCount == vertexCount));
    }

    void flush() {
        if (fPendingCount == 0) {
            return;
        }
        if (fPendingBufferBinds) {
            fDispatcher->bindDrawBuffers(fVertices, fInstances, fIndices);
            fPendingBufferBinds = false;
        }

        if (fTemplateCount == 0) {
            SkASSERT(!fIndices && !fInstances && fMaxCount == 0);
            fDispatcher->draw(fPrimitiveType, fPendingBaseVertex, fPendingCount);
        } else if (fMaxCount > 0) {
            SkASSERT(fPendingCount <= fMaxCount && !fInstances);
            fDispatcher->drawIndexed(fPrimitiveType, 0, fPendingCount, fPendingBaseVertex);
        } else {
            unsigned int vertexCount;
            if (fTemplateCount < 0) {
                vertexCount = -fTemplateCount - 1;
                fTemplateCount = -1; // reset to re-accumulate max count for next flush
            } else {
                vertexCount = fTemplateCount;
            }
            if (fIndices) {
                fDispatcher->drawIndexedInstanced(fPrimitiveType, 0, vertexCount, 0, fPendingBaseVertex, fPendingCount);
            } else {
                fDispatcher->drawInstanced(fPrimitiveType, 0, vertexCount, fPendingBaseVertex, fPendingCount);
            }
        }

        fPendingBase += fPendingCount;
        fPendingCount = 0;
    }

    class Appender {
    public:
        Appender(DrawWriter& w, bool appendToVertices) : fWriter(w), fTargetBinding(appendToVertices ? fWriter.fVertices : fWriter.fInstances), fTargetStride(appendToVertices ? fWriter.fVertexStride : fWriter.fInstanceStride) {}
        ~Appender() {
            if (fReservedCount > 0) {
                fWriter.fManager.returnVertices(fReserveCount * fTargetStride);
            }
        }

        void reserve(unsigned int count) {
            if (fReservedCount >= count) {
                return;
            } else if (fReservedCount > 0) {
                // Have contiguous bytes that can't satisfy request, so return them in the event the
                // DBM has additional contiguous bytes after the prior reserved range.
                fWriter.fManager.returnVertices(fReservedCount * fTargetStride);
            }

            fReservedCount = count;
            auto [fReservedWriter, reservedChunk] = fWriter.fManager->getVertexWriter(count * fTargetStride);
            if (reservedChunk.fBuffer != fTargetBinding.fBuffer ||
                reservedChunk.fOffset != fTargetBinding.fOffset + (fWriter.fPendingBase + fWriter.fPendingCount) * fTargetStride) {
            if (writer != fReservedWriter) {
                // Not contiguous, so must update the binding
                fWriter.flush();
                fTargetBinding = reservedChunk;
                fWriter.fPendingBase = 0;
                fWriter.fPendingBufferBinds = true;
                fReservedWriter = std::move(writer);
            }
        }

        VertexWriter append(unsigned int count) {
            SkASSERT(count > 0);
            this->reserve(count);

            SkASSERT(fReservedCount >= count);
            fReservedCount -= count;
            fWriter.fPendingCount += count;
            return std::exchange(fReservedWriter, fReservedWriter.makeOffset(count * fTargetStride));
        }

    private:
        DrawWriter& fWriter;
        BindBufferInfo& fTargetBinding;
        size_t fTargetStride;

        unsigned int fReservedCount;
        VertexWriter fReservedWriter;
    };

};

// Mirrors the CommandBuffer API, since a DrawWriter is meant to aggregate and then map onto
// CommandBuffer commands, although these are virtual to allow for recording to intermediate
// storage before a CommandBuffer is available.
class DrawDispatcher {
public:
    virtual ~DrawDispatcher() = default;

    virtual void bindDrawBuffers(BindBufferInfo vertexAttribs,
                                 BindBufferInfo instanceAttribs,
                                 BindBufferInfo indices) = 0;

    virtual void draw(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount) = 0;
    virtual void drawIndexed(PrimitiveType type, unsigned int baseIndex,
                             unsigned int indexCount, unsigned int baseVertex) = 0;
    virtual void drawInstanced(PrimitiveType type,
                               unsigned int baseVertex, unsigned int vertexCount,
                               unsigned int baseInstance, unsigned int instanceCount) = 0;
    virtual void drawIndexedInstanced(PrimitiveType type,
                                      unsigned int baseIndex, unsigned int indexCount,
                                      unsigned int baseVertex, unsigned int baseInstance,
                                      unsigned int instanceCount) = 0;
};

} // namespace skgpu

#endif // skgpu_DrawWriter_DEFINED
