---
name: skia/prefer-starray-for-small-bounded-arrays
description: When managing small bounded collections, stacks, or object pools, prefer skia_private::STArray over dynamic containers or fixed arrays with manual slot counters
rubric: readability
severity: should_fix
---

**Rule.** Prefer `skia_private::STArray<N, T>` over purely dynamic
heap-allocating array containers (such as `SkTDArray<T>` or `TArray<T>`) and
fixed-size arrays with manual slot counters for state stacks, temporary caches,
or small object pools. Choose an inline capacity `N` sized to cover typical
operations without wasting excessive space, and use standard container
operations (`push_back`, `pop_back`, `size()`) rather than maintaining separate
slot index counters alongside fixed-size arrays.

**Why.** `skia_private::STArray<N, T>` embeds inline storage for `N` elements
directly within the enclosing object or stack frame, eliminating heap allocation
and deallocation overhead on common paths while preserving cache locality. Using
standard container operations like `push_back()` and `pop_back()` also cleanly
encapsulates size tracking and bounds checking, avoiding mutable slot counters
that can desynchronize with array state.

## Examples

Bad:

```cpp
// 1. Dynamic heap-allocating container for small bounded stack
SkTDArray<SkMatrix> fMatrixStack;

// 2. Fixed array with manual slot counter for small object pool
std::array<std::unique_ptr<Helper>, kMaxHelpers> fHelpers;
int fHelperSlot = 0;
if (fHelperSlot < kMaxHelpers) {
    fHelpers[fHelperSlot++] = std::move(helper);
}
```

Good:

```cpp
// 1. Inline storage for up to 4 matrices without heap allocation
skia_private::STArray<4, SkMatrix> fMatrixStack;

// 2. Encapsulated size tracking and stack operations via STArray
skia_private::STArray<kMaxHelpers, std::unique_ptr<Helper>> fHelpers;
if (fHelpers.size() < kMaxHelpers) {
    fHelpers.push_back(std::move(helper));
}
```
