---
name: skia/borrow-smart-pointers-with-get-not-release
description: When passing a smart pointer to a function or builder that borrows a raw pointer, use .get() instead of .release() to avoid leaking memory
rubric: safety
severity: should_fix
---

**Rule.** Do not call `.release()` on a smart pointer (`std::unique_ptr` or
`sk_sp`) when passing it to a function, constructor, or builder that only
borrows a non-owning raw pointer (`const T*` or `T*`). Pass `.get()` instead to
borrow the raw pointer while retaining ownership.

**Why.** Calling `.release()` relinquishes ownership of the managed pointer
without deleting it. If the target API only borrows or copies from the raw
pointer rather than adopting ownership, the released object is never destroyed
and permanently leaks memory.

## Examples

Bad:

```cpp
// .release() abandons ownership; the builder only borrows the pointer, leaking the object:
fDisplayParams = DisplayParamsBuilder(fDisplayParams.release())
                         .msaaSampleCount(newMSAA)
                         .detach();
```

Good:

```cpp
// .get() borrows the non-owning raw pointer while fDisplayParams retains ownership:
fDisplayParams = DisplayParamsBuilder(fDisplayParams.get())
                         .msaaSampleCount(newMSAA)
                         .detach();
```
