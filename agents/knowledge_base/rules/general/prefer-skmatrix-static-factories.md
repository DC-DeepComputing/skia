---
name: skia/prefer-skmatrix-static-factories
description: When constructing translation, scale, or rotation matrices, use SkMatrix static factories (Translate, Scale, RotateDeg) instead of default-constructing and mutating
rubric: readability
severity: nit
---

**Rule.** Prefer SkMatrix static factory functions (such as SkMatrix::Translate,
SkMatrix::Scale, SkMatrix::RotateDeg, SkMatrix::RotateRad, or
SkMatrix::RectToRect) over default-constructing a mutable SkMatrix and
initializing it via setTranslate(), setScale(), setRotate(), or similar
mutators.

**Why.** Static factory functions condense multi-line declaration and mutation
sequences into a single self-documenting expression. This eliminates unnecessary
mutable local state, allows the resulting matrix variable to be const-qualified,
and supports passing matrix values inline directly to transformation APIs.

## Examples

Bad:

```cpp
// Default construction followed by imperative mutation
SkMatrix matrix;
matrix.setTranslate(dx, dy);
return this->addPath(path, matrix, mode);
```

Good:

```cpp
// Direct initialization with static factory, enabling const qualification
const SkMatrix matrix = SkMatrix::Translate(dx, dy);
return this->addPath(path, matrix, mode);

// Or passed directly inline
return this->addPath(path, SkMatrix::Translate(dx, dy), mode);
```
