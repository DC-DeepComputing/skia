---
name: skia/centralize-input-validation-in-shared-types
description: When handling invalid inputs or numerical edge cases, sanitize and validate values inside shared data structures or constructors rather than patching individual call sites
rubric: design
severity: should_fix
---

**Rule.** Push input validation, sanitization, and fallback clamping into shared
data structures, setters, or common helper functions rather than patching
individual call sites. Ensuring malformed, non-finite, or out-of-range inputs
are normalized at the source guarantees consistent protection across all
callers.

**Why.** Patching malformed data or clamping edge cases at an individual call
site leaves other consumers of the same data structure, setter, or algorithm
vulnerable to invalid inputs or logic errors. Centralizing validation and
sanitization at the source or in shared models guarantees consistent handling
across all current and future callers.

## Examples

Bad:

```cpp
// In SkEdgeBuilder::build():
SkASSERT(iter.conicWeight() > 0);
const SkScalar weight = iter.conicWeight() > 0 ? iter.conicWeight() : 1.0f;
const SkPoint* quadPts = quadder.computeQuads(e.fPts, weight, conicTol);
```

Good:

```cpp
// In SkConic::setW():
void setW(SkScalar w) {
    if (SkIsFinite(w)) {
        SkASSERT(w > 0);
    }
    fW = w > 0 && SkIsFinite(w) ? w : 1;
}

// In SkEdgeBuilder::build():
const SkPoint* quadPts = quadder.computeQuads(e.fPts, iter.conicWeight(), conicTol);
```
