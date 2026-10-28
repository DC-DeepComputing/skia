---
name: skia/gpu/document-buffer-writer-alignment-contracts
description: When declaring GPU buffer suballocation or data writer methods with similar signatures, document whether alignment satisfies hardware binding requirements or element-stride shader indexing
rubric: readability
severity: nit
---

**Rule.** When exposing GPU buffer suballocation or data writer methods with
identical or similar signatures (such as uniform versus SSBO/storage buffer
writers), document the specific alignment and indexing contract guaranteed by
each method in header comments. Explicitly state whether the suballocated offset
is aligned for hardware buffer binding (e.g., minimum uniform buffer offset
alignment) or aligned for direct shader indexing according to the provided
element stride.

**Why.** Different GPU buffer types and binding models enforce distinct
alignment invariants: uniform buffer bindings require hardware offset alignment,
whereas storage buffers (SSBOs) are typically bound once and indexed in shaders
per element stride. When sibling writer methods share identical signatures and
parameter types, omitting their alignment contracts leads to subtle offset
mismatches, invalid hardware bindings, and shader indexing corruption.

## Examples

Bad:

```cpp
// Sibling writer methods with identical signatures but undocumented alignment contracts
std::pair<UniformWriter, BindBufferInfo> getUniformWriter(size_t count, size_t stride);
std::pair<UniformWriter, BindBufferInfo> getSsboWriter(size_t count, size_t stride);
```

Good:

```cpp
// Uniform writers are aligned for binding, per hardware alignment requirements.
std::pair<UniformWriter, BindBufferInfo> getUniformWriter(size_t count, size_t stride);

// SSBO writers are aligned for indexing from the shader, per the element stride provided.
std::pair<UniformWriter, BindBufferInfo> getSsboWriter(size_t count, size_t stride);
```
