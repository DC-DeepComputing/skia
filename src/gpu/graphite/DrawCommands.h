/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawPassCommands_DEFINED
#define skgpu_graphite_DrawPassCommands_DEFINED

#include "include/core/SkRect.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkArenaAlloc.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"

namespace skgpu::graphite {

namespace DrawPassCommands {

// A list of all the commands types used by a DrawPass.
// Each of these is reified into a struct below.
//
// The design of this systems is based on SkRecords.

// (We're using the macro-of-macro trick here to do several different things with the same list.)
//
// We leave this SKGPU_DRAW_COMMAND_TYPES macro defined for use by code that wants to operate on
// DrawPassCommands types polymorphically.
#define SKGPU_DRAW_PASS_COMMAND_TYPES(M)                            \
    M(BindGraphicsPipeline)                                         \
    M(BindUniformBuffer)                                            \
    M(BindTexturesAndSamplers)                                      \
    M(SetScissor)                                                   \
    M(BindDrawBuffers)                                              \
    M(Draw)                                                         \
    M(DrawIndexed)                                                  \
    M(DrawInstanced)                                                \
    M(DrawIndexedInstanced)

// Defines DrawPassCommands::Type, an enum of all draw command types.
#define ENUM(T) T##_Type,
enum Type { SKGPU_DRAW_PASS_COMMAND_TYPES(ENUM) };
#undef ENUM

#define ACT_AS_PTR(ptr)                 \
    operator T*() const { return ptr; } \
    T* operator->() const { return ptr; }

// PODArray doesn't own the pointer's memory, and we assume the data is POD.
template <typename T>
class PODArray {
public:
    PODArray() {}
    PODArray(T* ptr) : fPtr(ptr) {}
    // Default copy and assign.

    ACT_AS_PTR(fPtr)
private:
    T* fPtr;
};

#undef ACT_AS_PTR

// A macro to make it a little easier to define a struct that can be stored in DrawPass.
#define COMMAND(T, ...)           \
struct T {                              \
    static const Type kType = T##_Type; \
    __VA_ARGS__;                        \
};

COMMAND(BindGraphicsPipeline,
         uint32_t fPipelineIndex);
COMMAND(BindUniformBuffer,
         BindBufferInfo fInfo;
         UniformSlot fSlot);
COMMAND(BindTexturesAndSamplers,
         int fNumTexSamplers;
         PODArray<int> fTextureIndices;
         PODArray<int> fSamplerIndices);
COMMAND(BindDrawBuffers,
         BindBufferInfo fVertices;
         BindBufferInfo fInstances;
         BindBufferInfo fIndices);
COMMAND(SetScissor,
         SkIRect fScissor);
COMMAND(Draw,
         PrimitiveType fType;
         uint32_t fBaseVertex;
         uint32_t fVertexCount);
COMMAND(DrawIndexed,
         PrimitiveType fType;
         uint32_t fBaseIndex;
         uint32_t fIndexCount;
         uint32_t fBaseVertex);
COMMAND(DrawInstanced,
         PrimitiveType fType;
         uint32_t fBaseVertex;
         uint32_t fVertexCount;
         uint32_t fBaseInstance;
         uint32_t fInstanceCount);
COMMAND(DrawIndexedInstanced,
         PrimitiveType fType;
         uint32_t fBaseIndex;
         uint32_t fIndexCount;
         uint32_t fBaseVertex;
         uint32_t fBaseInstance;
         uint32_t fInstanceCount);

// TODO: BindSampler

#undef COMMAND

class List final : public DrawDispatcher {
public:
    List() = default;
    ~List() override = default;

    void bindGraphicsPipeline(uint32_t pipelineIndex) {
        this->add<BindGraphicsPipeline>(pipelineIndex);
    }

    void bindUniformBuffer(BindBufferInfo info, UniformSlot slot) {
        this->add<BindUniformBuffer>(info, slot);
    }

    void bindTexturesAndSamplers(int numTexSamplers,
                                 int* textureIndices,
                                 int* samplerIndices) {
        this->add<BindTexturesAndSamplers>(numTexSamplers,
                                           this->copy(textureIndices, numTexSamplers),
                                           this->copy(samplerIndices, numTexSamplers));
    }

    void setScissor(SkIRect scissor) {
        this->add<SetScissor>(scissor);
    }

    // DrawDispatcher overrides
    void bindDrawBuffers(BindBufferInfo vertexAttribs,
                         BindBufferInfo instanceAttribs,
                         BindBufferInfo indices) override {
        this->add<BindDrawBuffers>(vertexAttribs, instanceAttribs, indices);
    }

    void draw(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount) override {
        this->add<Draw>(type, baseVertex, vertexCount);
    }

    void drawIndexed(PrimitiveType type, unsigned int baseIndex,
                     unsigned int indexCount, unsigned int baseVertex) override {
        this->add<DrawIndexed>(type, baseIndex, indexCount, baseVertex);
    }

    void drawInstanced(PrimitiveType type,
                       unsigned int baseVertex, unsigned int vertexCount,
                       unsigned int baseInstance, unsigned int instanceCount) override {
        this->add<DrawInstanced>(type, baseVertex, vertexCount, baseInstance, instanceCount);
    }

    void drawIndexedInstanced(PrimitiveType type,
                              unsigned int baseIndex, unsigned int indexCount,
                              unsigned int baseVertex, unsigned int baseInstance,
                              unsigned int instanceCount) override {
        this->add<DrawIndexedInstanced>(type,
                                        baseIndex,
                                        indexCount,
                                        baseVertex,
                                        baseInstance,
                                        instanceCount);
    }

private:
    template <typename T, typename... Args>
    void add(Args&&... args) {
        new (this->append<T>()) T{std::forward<Args>(args)...};
    }

    // Add a new command of type T to the end of this SkRecord.
    // You are expected to placement new an object of type T onto this pointer.
    template <typename T>
    T* append() {
        if (fCount == fReserved) {
            this->grow();
        }
        return fCommands[fCount++].set(this->allocCommand<T>());
    }

    template <typename T>
    std::enable_if_t<std::is_empty<T>::value, T*> allocCommand() {
        static T singleton = {};
        return &singleton;
    }

    template <typename T>
    std::enable_if_t<!std::is_empty<T>::value, T*> allocCommand() { return this->alloc<T>(); }

    void grow() {
        SkASSERT(fCount == fReserved);
        fReserved = fReserved ? fReserved * 2 : 4;
        fCommands.realloc(fReserved);
    }

    // Allocate contiguous space for count Ts, to be freed when the SkRecord is destroyed.
    // Here T can be any class, not just those from SkRecords.  Throws on failure.
    template <typename T>
    T* alloc(size_t count = 1) {
        struct RawBytes {
            alignas(T) char data[sizeof(T)];
        };
        fApproxBytesAllocated += count * sizeof(T) + alignof(T);
        return (T*)fAlloc.makeArrayDefault<RawBytes>(count);
    }

    // This copy() is for arrays.
    // It will work with POD or non-POD, though currently we only use it for POD.
    template <typename T>
    T* copy(const T src[], size_t count) {
        if (nullptr == src) {
            return nullptr;
        }
        T* dst = this->alloc<T>(count);
        for (size_t i = 0; i < count; i++) {
            new (dst + i) T(src[i]);
        }
        return dst;
    }

    struct Command {
        DrawPassCommands::Type fType;
        void*                  fPtr;

        // Point this Command to its data in fAlloc.  Returns ptr for convenience.
        template <typename T>
        T* set(T* ptr) {
            fType = T::kType;
            fPtr  = ptr;
            SkASSERT(this->ptr() == ptr && this->type() == T::kType);
            return ptr;
        }

        DrawPassCommands::Type type() const { return fType; }
        void* ptr() const { return fPtr; }
    };

    int fCount{0};
    int fReserved{0};
    SkAutoTMalloc<Command> fCommands;

    // fAlloc needs to be a data structure which can append variable length data in contiguous
    // chunks, returning a stable handle to that data for later retrieval.
    SkArenaAlloc fAlloc{256};
    size_t       fApproxBytesAllocated{0};
};

} // namespace DrawPassCommands

} // namespace skgpu::graphite

#endif // skgpu_graphite_DrawPassCommands_DEFINED

