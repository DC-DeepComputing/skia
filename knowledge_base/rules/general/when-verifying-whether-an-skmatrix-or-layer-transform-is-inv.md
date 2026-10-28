---
name: skia/when-verifying-whether-an-skmatrix-or-layer-transform-is-inv
description: When verifying whether an SkMatrix or layer transform is invertible without needing the inverse matrix, pass nullptr rather than declaring a dummy out-parameter variable
rubric: readability
severity: nit
---

**Rule.** When calling SkMatrix::invert() or LayerSpace<SkMatrix>::invert()
solely to check whether a matrix is invertible, pass nullptr (such as
/*inverse=*/nullptr) instead of declaring an unread dummy local matrix
out-parameter. Alternatively, use the parameterless invert() overload returning
std::optional<SkMatrix> when checking validity directly.

**Why.** SkMatrix::invert(SkMatrix* inverse) and its layer-space wrappers
explicitly support a null pointer when the caller only checks invertibility.
Declaring a dummy local matrix solely to receive an unread out-parameter adds
local variable clutter and incurs an unnecessary matrix copy on success.

## Examples

Bad:

```cpp
SkMatrix inv;
if (!transform.invert(&inv)) {
    return nullptr;
}
```

Good:

```cpp
if (!transform.invert(/*inverse=*/nullptr)) {
    return nullptr;
}
```
