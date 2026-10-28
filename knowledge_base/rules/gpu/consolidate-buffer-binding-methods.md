---
name: skia/gpu/consolidate-buffer-binding-methods
description: When implementing GPU command buffer binding methods for multiple buffer slots, consolidate duplicate binding routines into a single helper parameterized by binding index
rubric: design
severity: should_fix
---

**Rule.** Do not write separate buffer binding methods that differ only by the
target buffer slot or binding index (such as separate static and append data
binders). Consolidate identical null checks, buffer casts, and backend encoder
calls into a single helper parameterized by the binding slot index.

**Why.** Backend command buffer binding routines across Metal, Dawn, and Vulkan
perform identical null checking, buffer downcasting, and alignment verification
before recording encoder commands. Duplicating these routines across individual
buffer slots bloats command buffer interfaces and risks divergent validation or
encoder handling across slots.

## Examples

Bad:

```cpp
void MtlCommandBuffer::bindStaticDataBuffer(const Buffer* staticBuffer, size_t offset) {
    SkASSERT(fActiveRenderCommandEncoder);
    if (staticBuffer) {
        id<MTLBuffer> mtlBuffer = static_cast<const MtlBuffer*>(staticBuffer)->mtlBuffer();
        SkASSERT((offset & 0b11) == 0);
        fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, offset,
                                                     MtlGraphicsPipeline::kStaticDataBufferIndex);
    }
}

void MtlCommandBuffer::bindAppendDataBuffer(const Buffer* appendBuffer, size_t offset) {
    SkASSERT(fActiveRenderCommandEncoder);
    if (appendBuffer) {
        id<MTLBuffer> mtlBuffer = static_cast<const MtlBuffer*>(appendBuffer)->mtlBuffer();
        SkASSERT((offset & 0b11) == 0);
        fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, offset,
                                                     MtlGraphicsPipeline::kAppendDataBufferIndex);
    }
}
```

Good:

```cpp
void MtlCommandBuffer::bindInputBuffer(const Buffer* buffer, size_t offset, uint32_t bindingIndex) {
    SkASSERT(fActiveRenderCommandEncoder);
    if (buffer) {
        id<MTLBuffer> mtlBuffer = static_cast<const MtlBuffer*>(buffer)->mtlBuffer();
        SkASSERT((offset & 0b11) == 0);
        fActiveRenderCommandEncoder->setVertexBuffer(mtlBuffer, offset, bindingIndex);
    }
}
```
