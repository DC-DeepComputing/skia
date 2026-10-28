---
name: skia/include-wrapper-size-in-approximate-bytes-used
description: When implementing onApproximateBytesUsed or memory estimation for wrapper classes, include sizeof(WrapperClass) or sizeof(*this) alongside wrapped object bytes
rubric: correctness
severity: should_fix
---

**Rule.** When implementing onApproximateBytesUsed() or similar
memory-accounting methods for a wrapper or decorator class, include
sizeof(WrapperClass) or sizeof(*this) in addition to child object bytes. Do not
merely add the sizeof() of an individual member field or forward only to the
wrapped object.

**Why.** Wrapper and decorator classes have their own storage footprint,
including other member fields, base classes, and vtables. Adding only an
individual member's size undercounts memory in cache accounting (such as
SkStrike), leading to inaccurate cache eviction decisions and unbudgeted memory
consumption.

## Examples

Bad:

```cpp
size_t onApproximateBytesUsed() override {
    return fDrawable->approximateBytesUsed() + sizeof(SkMatrix);
}
```

Good:

```cpp
size_t onApproximateBytesUsed() override {
    return fDrawable->approximateBytesUsed() + sizeof(DrawableMatrixWrapper);
}
```
