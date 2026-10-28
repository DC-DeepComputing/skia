---
name: skia/fuse-correlated-temporary-allocations
description: When allocating temporary small-storage buffers for correlated data with matching lifetimes, fuse them into a single per-item struct instead of parallel buffers
rubric: safety
severity: should_fix
---

**Rule.** When allocating temporary or small-storage buffers with matching
element counts and lifetimes (such as metadata paired with per-item scratch or
payload storage), fuse them into a single per-item struct rather than
maintaining separate parallel allocations (such as AutoSTMalloc, SkAutoSMalloc,
or STArray). Embed per-element scratch arrays or payload fields directly into
the struct, passing a single unified SkSpan to downstream consumers.

**Why.** Allocating separate parallel small-storage buffers multiplies heap
allocation overhead whenever the element count exceeds inline capacity,
incurring multiple dynamic heap allocations instead of one. Embedding scratch
storage within the item struct guarantees at most one heap allocation on
fallback, improves cache locality, and simplifies parameter passing.

## Examples

Bad:

```cpp
// Separate parallel small-storage buffers multiply heap allocations on overflow
int numPatches = fMemoryCtxInfos.size();
AutoSTMalloc<2, MemoryCtxPatch> patches(numPatches);
SkAutoSMalloc<2 * kMaxScratchPerPatch> scratch(numPatches * kMaxScratchPerPatch);

start_pipeline(patches.data(), numPatches, scratch.get());
```

Good:

```cpp
// Embedding per-item scratch storage guarantees at most one heap allocation
struct MemoryCtxPatch {
    MemoryCtxInfo info;
    void* backup;
    std::byte scratch[kMaxScratchPerPatch];
};

int numPatches = fMemoryCtxInfos.size();
AutoSTMalloc<2, MemoryCtxPatch> patches(numPatches);

start_pipeline(SkSpan{patches.data(), numPatches});
```
