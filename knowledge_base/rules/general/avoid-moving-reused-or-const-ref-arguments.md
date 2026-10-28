---
name: skia/avoid-moving-reused-or-const-ref-arguments
description: When passing arguments to functions taking const references or when objects are accessed on subsequent paths, do not use std::move
rubric: safety
severity: should_fix
---

**Rule.** Do not use std::move when passing an argument to a function or method
that takes a const reference (or const value), nor when the source object is
accessed again on subsequent statements or fallback code paths. Transfer
ownership with std::move only at the final consumption point of an object when
the callee takes ownership.

**Why.** Invoking std::move on an argument passed to a const reference is an
ineffective no-op that misleadingly implies resource consumption and ownership
transfer. If the callee takes or is later modified to take ownership, moving an
object that is subsequently accessed on fallback or remaining execution paths
causes use-after-move bugs.

## Examples

Bad:

```cpp
// Bad: Moving an argument passed to const HBFace& or accessed on fallback paths
if (sk_sp<SkData> cffData = extract_cff_data(std::move(subset))) {
    return cffData;
}
HBBlob result(hb_face_reference_blob(subset.get()));
```

Good:

```cpp
// Good: Pass by lvalue when callee takes const& or when object is reused subsequently
if (sk_sp<SkData> cffData = extract_cff_data(subset)) {
    return cffData;
}
HBBlob result(hb_face_reference_blob(subset.get()));
```
