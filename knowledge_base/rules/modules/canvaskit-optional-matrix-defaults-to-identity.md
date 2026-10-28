---
name: skia/modules/canvaskit-optional-matrix-defaults-to-identity
description: In CanvasKit bindings, do not add manual null checks to reset matrices when using OptionalMatrix, which already defaults to identity
rubric: readability
severity: nit
---

**Rule.** In CanvasKit C++ bindings, do not add manual null pointer checks to
reset or assign a matrix to identity when using OptionalMatrix. Construct
OptionalMatrix(mPtr) directly, as it inherits from SkMatrix and automatically
defaults to the identity matrix when the WASM pointer is null.

**Why.** OptionalMatrix derives from SkMatrix and leaves the matrix initialized
to identity via SkMatrix's default constructor whenever the pointer is null.
Explicitly checking for a null pointer to reassign SkMatrix::I() introduces
redundant branching and dead code.

## Examples

Bad:

```cpp
SkMatrix ctm = OptionalMatrix(mPtr);
if (!mPtr) {
    ctm = SkMatrix::I();
}
```

Good:

```cpp
OptionalMatrix ctm(mPtr);
```
