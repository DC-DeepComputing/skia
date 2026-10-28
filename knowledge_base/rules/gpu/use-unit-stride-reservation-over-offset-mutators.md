---
name: skia/gpu/use-unit-stride-reservation-over-offset-mutators
description: When reserving byte padding or advancing GPU buffer offsets, use existing allocator reservation APIs with unit stride instead of adding specialized offset mutators
rubric: design
severity: should_fix
---

**Rule.** Do not add specialized buffer offset manipulation methods (such as
takeVertexBytes) to GPU buffer allocators when existing reservation APIs can
reserve arbitrary byte padding. Instead, call the standard reservation API (such
as getVertexWriter) specifying the required padding count with a unit stride and
alignment of 1.

**Why.** Adding ad-hoc methods to manually increment internal buffer offsets
bypasses allocator encapsulation, complicates buffer lifecycle tracking, and
creates redundant interfaces. Reusing the general reservation API with unit
stride preserves existing overflow handling and offset accounting while keeping
the allocator interface minimal.

## Examples

Bad:

```cpp
// Adding an ad-hoc method to manually bump buffer offset:
void DrawBufferManager::takeVertexBytes(size_t bytes) {
    fCurrentBuffers[kVertexBufferIndex].fOffset += bytes;
}

// Caller:
fManager->takeVertexBytes(byteDiff);
```

Good:

```cpp
// Reserve arbitrary byte padding using the standard allocator API with unit stride:
fManager->getVertexWriter(byteDiff, /*dataStride=*/1, /*alignStride=*/1);
```
