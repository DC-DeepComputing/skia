---
name: skia/gpu/document-zero-count-buffer-suballocations
description: When calling buffer suballocation routines (such as getMappedSubrange or getMappedVertexBuffer) with count=0 to prime alignment, document the purpose of the zero-count call
rubric: readability
severity: nit
---

**Rule.** When calling GPU buffer suballocation or reservation methods (such as
getMappedSubrange or getMappedVertexBuffer) with count=0 to prime alignment
offsets or configure allocator state without consuming space, document the
purpose of the zero-count call in an explanatory comment.

**Why.** Requesting an allocation with count=0 is counterintuitive and appears
redundant or buggy at first glance. An inline comment clarifies that the call
deliberately establishes an aligned buffer offset or initializes allocator
headroom for subsequent appends without allocating elements upfront.

## Examples

Bad:

```cpp
const uint32_t baseAlign = newRenderState & RenderStateFlags::kAppendVertices ?
        4 * fAppendStride : fAppendStride;
std::tie(std::ignore, fAppend) = fCurrentBuffer.getMappedSubrange(/*count=*/0,
                                                                  fAppendStride,
                                                                  baseAlign);
```

Good:

```cpp
const uint32_t baseAlign = newRenderState & RenderStateFlags::kAppendVertices ?
        4 * fAppendStride : fAppendStride;

// Initializes fAppend to hold the aligned offset within fCurrentBuffer and prepares
// fCurrentBuffer for later reservations. Passing count=0 here means no actual space
// (besides alignment padding) is used up until appendMappedWithStride is called.
std::tie(std::ignore, fAppend) = fCurrentBuffer.getMappedSubrange(/*count=*/0,
                                                                  fAppendStride,
                                                                  baseAlign);
```
