---
name: skia/gpu/encapsulate-compound-shape-predicates
description: When checking geometric shapes or objects for compound state representing a semantic concept, provide a dedicated predicate method rather than repeating raw boolean expressions
rubric: design
severity: should_fix
---

**Rule.** Encapsulate compound state checks representing a distinct semantic
concept (such as a full-coverage flood fill represented by isEmpty() &&
inverted()) into a dedicated predicate method on the class (e.g.,
Shape::isFloodFill()). Do not repeat raw compound boolean checks across multiple
call sites or assertions.

**Why.** Repeating raw compound boolean expressions like shape->isEmpty() &&
shape->inverted() across call sites obscures geometric intent, leaks internal
representation details, and creates maintenance hazards if the underlying
representation evolves. A dedicated predicate method makes call sites
self-documenting and keeps geometric logic cohesive.

## Examples

Bad:

```cpp
// Bad: inlined compound representation details across call sites
bool shapeIntersectable = shape->isRect() ||
                          shape->isRRect() ||
                          (shape->isEmpty() && shape->inverted());
SkASSERT(!(otherShape.isEmpty() && otherShape.inverted()));
```

Good:

```cpp
// Good: semantic predicate method encapsulates representation
bool shapeIntersectable = shape->isRect() ||
                          shape->isRRect() ||
                          shape->isFloodFill();
SkASSERT(!otherShape.isFloodFill());
```
