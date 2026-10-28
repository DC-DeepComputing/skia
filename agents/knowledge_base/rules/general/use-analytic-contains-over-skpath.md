---
name: skia/use-analytic-contains-over-skpath
description: When testing point or rect containment against primitive shapes like SkRRect or SkRect, use analytic contains() directly instead of converting to SkPath
rubric: safety
severity: should_fix
---

**Rule.** When testing whether a point or rectangle lies inside a primitive
geometric shape such as SkRRect or SkRect, call analytic containment methods
(such as SkRRect::contains(point) or SkRRect::contains(rect)) directly. Do not
convert the shape into an SkPath (e.g., via asPath() or SkPathBuilder) solely to
test containment.

**Why.** Converting primitive shapes like SkRRect into SkPath allocates and
constructs verb, point, and conic buffers, then evaluates expensive winding
containment algorithms. Primitive geometric shapes provide direct, closed-form
analytic containment queries that execute in constant time without memory
allocation.

## Examples

Bad:

```cpp
bool RRect::onContains(const SkPoint& p) const {
    if (!fRRect.rect().contains(p.x(), p.y())) {
        return false;
    }
    // Converting to SkPath solely for containment check
    return this->asPath().contains(p.x(), p.y());
}
```

Good:

```cpp
bool RRect::onContains(const SkPoint& p) const {
    // Test containment directly against the analytic primitive
    return fRRect.contains(p);
}
```
