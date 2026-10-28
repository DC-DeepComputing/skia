---
name: skia/gpu/generalize-table-driven-buffer-sizing
description: In table-driven GPU buffer managers, apply dynamic sizing and lifecycle updates across all descriptor entries rather than special-casing specific buffer types
rubric: design
severity: should_fix
---

**Rule.** In GPU buffer and resource managers configured via descriptor tables
(such as DrawBufferManager's fCurrentBuffers), apply sizing, capacity
adjustments, and lifecycle calculations generically across all descriptor
entries in a loop rather than hardcoding special-case logic for specific buffer
indices. Rely on per-entry bounds (such as matching min and max block sizes for
fixed allocations) to let entries naturally opt into or out of dynamic sizing.

**Why.** Hardcoding special cases for individual buffer indices breaks
table-driven architecture, creates subtle code divergence, and requires
modifying procedural code whenever other buffer types need similar behavior.
Driving sizing logic through descriptor table parameters allows entries to
configure their own bounds cleanly and enables dynamic sizing across buffer
types without procedural branching.

## Examples

Bad:

```cpp
// Bad: Hardcoding sizing logic for a specific buffer index outside the descriptor loop
BufferInfo& info = fCurrentBuffers[kStorageBufferIndex];
if (info.fUsedSize > 0) {
    const uint32_t reqSize = SkAlignTo(info.fUsedSize + info.fOffset, info.fMinBlockSize);
    info.fCurBlockSize = std::clamp(reqSize, info.fMinBlockSize, info.fMaxBlockSize);
    info.fUsedSize = 0;
}
```

Good:

```cpp
// Good: Applying sizing generically across all descriptors using per-entry bounds
for (auto& info : fCurrentBuffers) {
    if (info.fUsedSize > 0) {
        const uint32_t reqSize = SkAlignTo(info.fUsedSize + info.fOffset, info.fMinBlockSize);
        info.fCurBlockSize = std::clamp(reqSize, info.fMinBlockSize, info.fMaxBlockSize);
        info.fUsedSize = 0;
    }
}
```
