---
name: skia/prefer-sktinternalllist-for-arena-draw-collections
description: When managing collections in SkArenaAlloc, do not instantiate dynamically growing heap-backed containers like TArray; use intrusive lists or arena arrays
rubric: safety
severity: should_fix
---

**Rule.** Do not instantiate or declare dynamically growing heap-backed
containers (such as skia_private::TArray or std::vector) inside an SkArenaAlloc
(e.g., via alloc->make<TArray<T>>()). When collecting arena-allocated draws,
tasks, or objects, use arena-native intrusive linked lists (such as
SkTInternalLList paired with SK_DECLARE_INTERNAL_LLIST_INTERFACE or
SkArenaAllocList) or fixed-size arena arrays (makeArray/makeArrayCopy) to
eliminate heap allocation overhead.

**Why.** alloc->make<T>() only allocates sizeof(T) bytes in the arena and
invokes T's constructor; any backing storage allocated during dynamic resizing
(such as push_back()) is allocated on the system heap. Intrusive lists store
pointer linkage directly within arena-allocated node objects, grouping elements
with zero heap allocations.

## Examples

Bad:

```cpp
// Bad: TArray backing buffer allocates on the system heap despite arena instantiation
struct DepthDrawInfo {
    skia_private::TArray<DepthDraw*>* fDraws = nullptr;
};
fDepthInfo->fDraws = alloc->make<skia_private::TArray<DepthDraw*>>();
fDepthInfo->fDraws->push_back(deferredDraw);
```

Good:

```cpp
// Good: Intrusive list embeds linkage in arena-allocated nodes with zero heap allocations
struct DepthDraw {
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(DepthDraw);
};
struct DepthDrawList {
    SkTInternalLList<DepthDraw> fDraws;
};
fDepthInfo->fDraws.addToTail(deferredDraw);
```
