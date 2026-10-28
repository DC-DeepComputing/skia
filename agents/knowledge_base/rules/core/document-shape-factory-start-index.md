---
name: skia/core/document-shape-factory-start-index
description: When declaring shape or path factory or builder methods taking a startIndex parameter, document that it defines the initial contour vertex and cite SkPath conventions
rubric: readability
severity: nit
---

**Rule.** When declaring shape or path factory or builder methods (such as Rect,
Oval, or RRect on SkPath, SkPathBuilder, or SkPathData) that accept a startIndex
parameter, document its semantics with an explanatory comment. Explicitly state
that startIndex defines the initial vertex position of the constructed contour
and cite the established conventions in SkPath and SkPathBuilder.

**Why.** Parameters named startIndex on geometry factories are ambiguous without
documentation—callers and reviewers cannot determine whether the index specifies
a vertex position, a contour index, or an array offset. Clarifying the
parameter's meaning and cross-referencing SkPath conventions ensures consistent
contour construction across the path subsystem.

## Examples

Bad:

```cpp
static sk_sp<SkPathData> Rect(const SkRect&,
                              SkPathDirection = SkPathDirection::kDefault,
                              unsigned startIndex = 0);
static sk_sp<SkPathData> Oval(const SkRect&,
                              SkPathDirection = SkPathDirection::kDefault,
                              unsigned startIndex = 1);
static sk_sp<SkPathData> RRect(const SkRRect&, SkPathDirection, unsigned startIndex);
```

Good:

```cpp
/*
 *  When a factory takes a startIndex, this refers to the position of the first point
 *  when constructing one of our simple shapes: rect, oval, rrect.
 *  The index is the same as that passed to the equivalent factories in SkPath
 *  and in the associated addRect/Oval/RRect methods on SkPathBuilder.
 */
static sk_sp<SkPathData> Rect(const SkRect&,
                              SkPathDirection = SkPathDirection::kDefault,
                              unsigned startIndex = 0);
static sk_sp<SkPathData> Oval(const SkRect&,
                              SkPathDirection = SkPathDirection::kDefault,
                              unsigned startIndex = 1);
static sk_sp<SkPathData> RRect(const SkRRect&, SkPathDirection, unsigned startIndex);
```
