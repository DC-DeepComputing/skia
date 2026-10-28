---
name: skia/sksl/describe-intrinsic-type-invariants-in-comments
description: In SkSL frontend IR, type checking, or slot calculation comments, describe intrinsic type invariants directly rather than attributing them to backend limitations
rubric: readability
severity: nit
---

**Rule.** When documenting frontend IR semantics, type validation, or
compile-time calculations (such as slot counting) in SkSL, explain constraints
in terms of intrinsic language or type properties. Do not attribute intrinsic
type limitations (such as unsized or dynamically sized arrays lacking a fixed
compile-time slot count) to backend limitations or lack of backend support.

**Why.** Frontend IR structures and type constraints exist independently of code
generation backends. Attributing intrinsic semantic properties (like unsized
types having no static size) to backend-specific limitations is inaccurate and
misleading to maintainers who may assume the limitation is temporary or
backend-specific.

## Examples

Bad:

```cpp
// Number of slots does not apply to unsized arrays since they are
// dynamically sized and are not supported for the raster pipeline.
```

Good:

```cpp
// Number of slots does not apply to unsized arrays since they are
// dynamically sized.
```
