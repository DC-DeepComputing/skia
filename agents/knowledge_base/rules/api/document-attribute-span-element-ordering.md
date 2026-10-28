---
name: skia/api/document-attribute-span-element-ordering
description: When declaring APIs that return spans or collections representing multi-element attributes, document the element ordering contract or referencing enum
rubric: readability
severity: should_fix
---

**Rule.** When an API returns a span or collection representing a fixed set of
logical attributes or components (e.g., corners, vertices, or color channels),
document its element ordering contract—such as referencing the defining indexing
enum—or explicitly note that the order is unspecified.

**Why.** Callers indexing into a returned span or collection of attributes must
know which index corresponds to which logical component (such as upper-left vs.
lower-right corner). Undocumented ordering forces callers to inspect
implementation details or guess indices, risking indexing mismatch bugs if
caller assumptions diverge from internal memory layouts.

## Examples

Bad:

```cpp
SkSpan<const SkVector> radii() const { return SkSpan(fRadii, 4); }
```

Good:

```cpp
/**
 * Returns the corner radii for all four corners, in the same order as `Corner`.
 */
SkSpan<const SkVector> radii() const { return SkSpan(fRadii, 4); }
```
