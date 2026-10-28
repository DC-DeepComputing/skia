---
name: skia/gpu/match-buffer-type-index-in-wrappers
description: When implementing typed GPU buffer manager wrappers, verify each specialized helper forwards its corresponding buffer type index rather than a copy-pasted sibling constant
rubric: correctness
severity: blocking
---

**Rule.** When implementing specialized wrapper methods or dispatch helpers in
GPU buffer managers, verify that each typed helper forwards its corresponding
buffer type index or constant (such as kIndexBufferIndex for index buffers)
rather than a copy-pasted sibling constant (such as kVertexBufferIndex). Ensure
all sibling wrappers in the family match their target resource type.

**Why.** Buffer managers maintain distinct allocation pools, alignment
constraints, and GPU usage flags for different buffer types. Forwarding the
wrong buffer type index allocates from an unintended pool or binds resources
with invalid usage flags, resulting in pipeline validation errors, layout
corruption, or silent rendering defects.

## Examples

Bad:

```cpp
MappedAllocationInfo getMappedIndexBuffer(size_t count) {
    // Bug: copy-pasted kVertexBufferIndex forwards the wrong buffer pool and flags.
    return this->getMappedBuffer(kVertexBufferIndex, count, sizeof(uint16_t));
}
```

Good:

```cpp
MappedAllocationInfo getMappedIndexBuffer(size_t count) {
    return this->getMappedBuffer(kIndexBufferIndex, count, sizeof(uint16_t));
}
```
