---
name: skia/prefer-autotarray-over-raw-array-pointers
description: When managing dynamically allocated arrays in class members or local scopes, prefer skia_private::AutoTArray over raw heap pointers and separate size fields
rubric: safety
severity: should_fix
---

**Rule.** Prefer RAII array wrappers such as skia_private::AutoTArray<T> (or
AutoSTArray<N, T> for inline storage) over raw heap pointers (T*) and manual
length members when storing dynamically allocated arrays. Rely on the
container's built-in .size() and automatic lifetime management instead of
manually allocating with new[], tracking element counts, and deleting in
destructors or reset helpers.

**Why.** Raw heap array pointers allocated with new[] require manual cleanup and
are prone to memory leaks, dangling references, or double-frees if error paths
or destructors miss delete[]. Sized RAII containers like AutoTArray tie memory
deallocation to object lifecycle, eliminate separate count member tracking, and
avoid count-pointer synchronization mismatches.

## Examples

Bad:

```cpp
// Raw array pointer requires manual delete[] and redundant count member.
class SwapchainTracker {
    uint32_t fImageCount = 0;
    VkImage* fImages = nullptr;
    ~SwapchainTracker() { delete[] fImages; }
};
```

Good:

```cpp
// RAII wrapper automatically manages heap lifetime and tracks .size().
class SwapchainTracker {
    skia_private::AutoTArray<VkImage> fImages;
};
```
