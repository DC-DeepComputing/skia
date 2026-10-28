---
name: skia/use-size-t-for-counts-with-skspan
description: When adopting or exposing SkSpan, use size_t for internal count members, accessors, and loop indices rather than int to eliminate repetitive casts
rubric: readability
severity: nit
---

**Rule.** When refactoring or implementing classes and APIs to use SkSpan, align
internal count and size member variables, accessors, and related loop indices to
size_t. Avoid keeping counts as signed int when constructing SkSpan instances or
checking index bounds, which introduces repetitive casts and signed/unsigned
comparison warnings.

**Why.** SkSpan and standard C++ containers use size_t for counts and indices.
Retaining int for internal count members requires explicit casts (such as
(size_t)count or SkToSizeT) when returning spans or checking bounds, introducing
unnecessary casting noise and potential signed/unsigned comparison issues.

## Examples

Bad:

```cpp
// Bad: Keeping internal count as int causes repetitive size_t casts at span boundaries
class Gradient {
    int fColorCount;

    SkScalar getPos(size_t i) const {
        SkASSERT(i < (size_t)fColorCount);
        return fPositions ? fPositions[i] : SkIntToScalar(i) / (fColorCount - 1);
    }
    SkSpan<const SkColor4f> colors() const { return {fColors, (size_t)fColorCount}; }
};
```

Good:

```cpp
// Good: Align internal count and index types to size_t when using SkSpan
class Gradient {
    size_t fColorCount;

    SkScalar getPos(size_t i) const {
        SkASSERT(i < fColorCount);
        return fPositions ? fPositions[i] : SkIntToScalar(i) / (fColorCount - 1);
    }
    SkSpan<const SkColor4f> colors() const { return {fColors, fColorCount}; }
};
```
