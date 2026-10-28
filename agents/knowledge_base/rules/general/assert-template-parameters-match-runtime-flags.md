---
name: skia/assert-template-parameters-match-runtime-flags
description: When templating functions or methods on compile-time booleans to avoid runtime branching, assert that template arguments match dynamic state flags using SkASSERT and SkToBool
rubric: readability
severity: should_fix
---

**Rule.** When templating a method or function on a compile-time boolean to
eliminate runtime branching on a dynamic state flag or bitmask, assert with
SkASSERT that the template argument matches the corresponding runtime state. Use
SkToBool to convert bitmask expressions before equality comparison. Do not omit
the assertion, as it ensures callers dispatch the correct template instantiation
matching active object state.

**Why.** When runtime branch flags are specialized into compile-time template
parameters for performance, caller dispatch bugs or state desynchronization can
silently execute inappropriate code paths. Asserting that static template
arguments match dynamic state flags catches incorrect static dispatch early in
debug builds without incurring release-build overhead.

## Examples

Bad:

```cpp
template <bool kAppendVerts>
void reserve(unsigned int count) {
    // Missing assertion that static template parameter matches dynamic state
    uint32_t alignedCount = count;
    uint32_t alignedStride = fDrawer.fAppendStride;
    if constexpr (kAppendVerts) {
        ...
    }
}
```

Good:

```cpp
template <bool kAppendVerts>
void reserve(unsigned int count) {
    // Assert compile-time template parameter matches dynamic state flag
    SkASSERT(kAppendVerts == SkToBool(fDrawer.fRenderState & RenderStateFlags::kAppendVertices));
    uint32_t alignedCount = count;
    uint32_t alignedStride = fDrawer.fAppendStride;
    if constexpr (kAppendVerts) {
        ...
    }
}
```
