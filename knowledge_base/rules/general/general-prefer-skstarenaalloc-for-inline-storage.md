---
name: skia/general-prefer-skstarenaalloc-for-inline-storage
description: When provisioning SkArenaAlloc with inline storage, use SkSTArenaAlloc<N> instead of manually declaring a separate char buffer and passing it to SkArenaAlloc
rubric: readability
severity: should_fix
---

**Rule.** When providing inline storage for an SkArenaAlloc or
SkArenaAllocWithReset, use SkSTArenaAlloc<N> or SkSTArenaAllocWithReset<N>
instead of manually declaring a separate raw char buffer (such as char
storage[N]) and passing its pointer and size to SkArenaAlloc. Pass any heap
growth parameters directly to the SkSTArenaAlloc constructor.

**Why.** SkSTArenaAlloc<N> embeds the inline storage buffer directly within the
allocator type, eliminating boilerplate storage declarations, constructor
wiring, and potential member-initialization order bugs where an arena could be
initialized before its backing buffer.

## Examples

Bad:

```cpp
class MemoryPool {
    MemoryPool() : fArena(fBlock, sizeof(fBlock), /*firstHeapAllocation=*/sizeof(fBlock) / 2) {}

    SkArenaAlloc fArena;
    char fBlock[65536];
};
```

Good:

```cpp
class MemoryPool {
    SkSTArenaAlloc<65536> fArena{/*firstHeapAllocation=*/32768};
};
```
