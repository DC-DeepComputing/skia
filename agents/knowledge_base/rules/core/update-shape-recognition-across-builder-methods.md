---
name: skia/core/update-shape-recognition-across-builder-methods
description: When updating path builder methods that classify shape convexity (such as tolerating leading moves), update all shape-adding methods like addRect, addOval, and addRRect consistently
rubric: correctness
severity: should_fix
---

**Rule.** When modifying path builder methods that classify geometric shape or
convexity state (such as allowing shapes to be recognized despite preceding
non-drawing verbs like moveTo), apply the logic consistently across all
shape-adding methods in the family, including addRect, addOval, and addRRect. Do
not update only a subset of shape primitives while leaving others with stale or
stricter emptiness checks.

**Why.** Path builders track whether a path represents a recognized primitive
shape to preserve convexity and fast-path drawing metadata when populated on an
effectively empty path. Updating only a subset of shape addition methods causes
inconsistent classification and rendering behavior depending on which primitive
shape is added.

## Examples

Bad:

```cpp
// Updated addOval and addRRect to tolerate leading moves, but left addRect with strict isEmpty()
SkPathBuilder& SkPathBuilder::addRect(const SkRect& rect, SkPathDirection dir, unsigned index) {
    const bool wasEmpty = this->isEmpty();
    this->addRaw(SkPathRawShapes::Rect(rect, dir, index));
    if (wasEmpty) {
        fConvexity = SkPathDirection_ToConvexity(dir);
    }
    return *this;
}
```

Good:

```cpp
// Applied consistent emptiness/move check across all shape-adding methods
SkPathBuilder& SkPathBuilder::addRect(const SkRect& rect, SkPathDirection dir, unsigned index) {
    const bool wasEmpty = is_empty_or_moves(fVerbs);
    this->addRaw(SkPathRawShapes::Rect(rect, dir, index));
    if (wasEmpty) {
        fConvexity = SkPathDirection_ToConvexity(dir);
    }
    return *this;
}
```
