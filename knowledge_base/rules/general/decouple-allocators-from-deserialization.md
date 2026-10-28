---
name: skia/decouple-allocators-from-deserialization
description: When implementing domain deserialization helpers taking an arena or allocator, place them in domain components rather than allocator headers or files
rubric: design
severity: should_fix
---

**Rule.** Keep memory allocators and arena utilities decoupled from
domain-specific data types and serialization logic. Do not place buffer
deserialization or data extraction helpers in allocator headers or source files
merely because they accept an allocator parameter; define such helpers in the
domain container or component that owns and operates on the deserialized data.

**Why.** Allocators are foundational utilities meant to be reused across
disparate subsystems without pulling in higher-level dependencies. Placing
domain-specific deserialization routines (such as parsing points or glyphs from
an SkReadBuffer) into allocator files introduces unnecessary dependencies,
violates architectural layering, and clutters low-level memory interfaces.

## Examples

Bad:

```cpp
// In SubRunAllocator.h / SubRunAllocator.cpp:
SkSpan<SkPoint> MakePointsFromBuffer(SkReadBuffer& buffer, SubRunAllocator* alloc) {
    uint32_t glyphCount = buffer.getArrayCount();
    SkPoint* positionsData = alloc->makePODArray<SkPoint>(glyphCount);
    if (!buffer.readPointArray(positionsData, glyphCount)) { return {}; }
    return {positionsData, glyphCount};
}
```

Good:

```cpp
// In SubRunContainer.h / SubRunContainer.cpp:
SkSpan<SkPoint> MakePointsFromBuffer(SkReadBuffer& buffer, SubRunAllocator* alloc) {
    uint32_t glyphCount = buffer.getArrayCount();
    SkPoint* positionsData = alloc->makePODArray<SkPoint>(glyphCount);
    if (!buffer.readPointArray(positionsData, glyphCount)) { return {}; }
    return {positionsData, glyphCount};
}
```
