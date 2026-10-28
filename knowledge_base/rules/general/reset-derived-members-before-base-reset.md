---
name: skia/reset-derived-members-before-base-reset
description: When implementing custom reset or teardown methods in derived classes, reset derived members referencing base-managed storage before calling the base class reset
rubric: safety
severity: nit
---

**Rule.** In class hierarchies with custom reset(), clear(), or teardown
methods, reset or clear derived class members that reference base-class-managed
storage before invoking the base class reset method. Mirror standard C++
destructor destruction order so that dependent child members are dismantled
before the parent allocators, arenas, or caches they point into are cleared.

**Why.** Derived class members often hold raw pointers, references, or iterators
into arenas, block allocators, or caches owned by the base class. Invoking the
base class reset first destroys or invalidates that underlying storage while
derived members still exist, creating dangling pointers, use-after-free hazards,
or dangling pointer detector violations during teardown.

## Examples

Bad:

```cpp
void DrawList::reset(LoadOp op, SkColor4f clearColor) {
    // Calling base reset first clears allocators while derived members still reference them.
    DrawListBase::reset(op, clearColor);
    fDraws.reset();
    fSortKeys.clear();
}
```

Good:

```cpp
void DrawList::reset(LoadOp op, SkColor4f clearColor) {
    // Clear derived members before clearing base storage to prevent dangling references.
    fDraws.reset();
    fSortKeys.clear();
    DrawListBase::reset(op, clearColor);
}
```
