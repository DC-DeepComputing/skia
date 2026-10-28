---
name: skia/size-skarenaalloc-for-minimal-workloads
description: When configuring initial capacity for SkArenaAlloc or SkArenaAllocWithReset, size for minimal common workloads rather than peak capacity, relying on Fibonacci growth
rubric: safety
severity: should_fix
---

**Rule.** When specifying the initial block size for SkArenaAlloc,
SkArenaAllocWithReset, or SkSTArenaAlloc, size the initial allocation to
accommodate minimal common workloads (e.g., 1–2 items or draws) rather than peak
batch capacity. Rely on SkArenaAlloc's built-in Fibonacci block progression to
scale allocation size for larger workloads.

**Why.** SkArenaAlloc allocates subsequent blocks according to a Fibonacci
progression, providing exponential capacity growth without excessive
reallocations. Sizing initial allocations for peak capacity wastes memory on
common small workloads or across numerous arena instances.

## Examples

Bad:

```cpp
// Over-allocating initial capacity wastes memory for typical small workloads
static constexpr uint32_t kDefaultAllocation = 4096;
SkArenaAllocWithReset fStorage{kDefaultAllocation};
```

Good:

```cpp
// Sized to accommodate 1-2 items; Fibonacci block growth handles larger batches
static constexpr uint32_t kDefaultAllocation = 256;
SkArenaAllocWithReset fStorage{kDefaultAllocation};
```
