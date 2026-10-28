---
name: skia/modules/name-active-scope-query-methods
description: When declaring query or coordinate transformation methods on context classes tracking an active scope, explicitly specify that operations target the current scope
rubric: readability
severity: nit
---

**Rule.** On context classes that track an active node, element, or scope (such
as SkSVGRenderContext), name query and coordinate transformation methods to
explicitly indicate that the computation evaluates against the current scope
(e.g., transformForCurrentOBB rather than bare obbTransform). Avoid names that
ambiguously imply the method transforms an arbitrary input argument rather than
querying the context's active state.

**Why.** A method name like obbTransform(units) is ambiguous in a stateful
context class because it suggests transforming a caller-supplied bounding box
rather than computing the coordinate transform for the context's currently
active element. Explicitly qualifying that the method targets the current scope
prevents confusion at call sites and distinguishes context queries from
standalone transformation utilities.

## Examples

Bad:

```cpp
// In SkSVGRenderContext:
OBBTransform obbTransform(SkSVGObjectBoundingBoxUnits) const;
```

Good:

```cpp
// In SkSVGRenderContext:
OBBTransform transformForCurrentOBB(SkSVGObjectBoundingBoxUnits) const;
```
