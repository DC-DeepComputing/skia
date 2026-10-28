---
name: skia/gpu/remove-redundant-encoder-buffer-tracking
description: When removing low-level vertex buffer tracking in Graphite Metal command encoders, also remove redundant fragment buffer tracking and offset updates
rubric: correctness
severity: should_fix
---

**Rule.** Do not retain redundant buffer bind tracking or offset updating (such
as setFragmentBufferOffset or fCurrentFragmentBuffer arrays) in Graphite Metal
command encoder wrappers when removing vertex buffer tracking. Because vertex
and fragment buffer bindings share the same semantics in Metal and are obviated
by DrawWriter tracking and SSBO usage, remove low-level state tracking across
both shader stages simultaneously.

**Why.** In Metal, vertex and fragment buffer indexing follow identical binding
semantics, and Graphite relies on higher-level DrawWriter tracking and native
SSBO indexing from offset 0. Retaining encoder-level caching and conditional
offset updates for fragment buffers when vertex caching is removed preserves
dead tracking state and creates inconsistent binding patterns across shader
stages.

## Examples

Bad:

```cpp
void setFragmentBuffer(id<MTLBuffer> buffer, NSUInteger offset, NSUInteger index) {
    if (fCurrentFragmentBuffer[index] == buffer) {
        this->setFragmentBufferOffset(offset, index);
        return;
    }
    if (fCurrentFragmentBuffer[index] != buffer || fCurrentFragmentOffset[index] != offset) {
        [(*fCommandEncoder) setFragmentBuffer:buffer offset:offset atIndex:index];
        fCurrentFragmentBuffer[index] = buffer;
        fCurrentFragmentOffset[index] = offset;
    }
}
```

Good:

```cpp
void setFragmentBuffer(id<MTLBuffer> buffer, NSUInteger offset, NSUInteger index) {
    SkASSERT(buffer != nil);
    SkASSERT(index < kMaxExpectedBuffers);
    [(*fCommandEncoder) setFragmentBuffer:buffer
                                   offset:offset
                                  atIndex:index];
}
```
