---
name: skia/api/prefer-pair-over-two-element-tuple
description: When returning or storing two-element product values, prefer std::pair over a 2-element std::tuple to minimize template overhead and improve readability
rubric: readability
severity: nit
---

**Rule.** Prefer std::pair over a 2-element std::tuple when returning or storing
a pair of values. Reserve std::tuple for three or more heterogeneous elements,
and prefer a dedicated struct with named fields when the values have distinct
semantic roles.

**Why.** std::pair is the idiomatic C++ standard type for a two-element
heterogeneous product. Using a 2-element std::tuple introduces unnecessary
template complexity and compilation overhead without benefit, whereas std::pair
is lighter, directly accessible via .first and .second, and fully supports
structured bindings.

## Examples

Bad:

```cpp
// Bad: using a 2-element std::tuple for a pair of values
std::tuple<CompressedPaintersOrder, Layer*> updateClipStateForDraw(
        const Clip& clip,
        const ElementList& effectiveElements,
        const BoundsManager*,
        PaintersDepth z);
```

Good:

```cpp
// Good: use std::pair for two-element values
std::pair<CompressedPaintersOrder, Layer*> updateClipStateForDraw(
        const Clip& clip,
        const ElementList& effectiveElements,
        const BoundsManager*,
        PaintersDepth z);
```
