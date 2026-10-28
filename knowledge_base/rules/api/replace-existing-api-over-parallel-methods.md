---
name: skia/api/replace-existing-api-over-parallel-methods
description: When modernizing public APIs with specialized fast paths, update existing methods directly, qualify preconditions explicitly, and delegate dispatchers rather than duplicating calculations inline
rubric: design
severity: should_fix
---

**Rule.** When introducing an optimized, modernized, or faster implementation of
an existing operation in public headers, update or replace the existing public
API directly rather than adding a parallel method with redundant functionality.
Keep canonical dispatching methods unadorned with standard operation names
(e.g., mapPoint), avoid ambiguous qualifiers like *Check or *Fast that obscure
what condition is checked, and explicitly qualify specialized fast paths with
their specific precondition (e.g., mapPointAffine). Additionally, in canonical
or fallback dispatching methods, delegate directly to those specialized helper
methods rather than duplicating calculation or transformation formulas inline.

**Why.** Ambiguous suffixes like *Check or *Fast confuse callers about what
invariants are verified, while parallel methods bloat the public API surface.
Explicit condition qualifiers make fast-path preconditions self-documenting, and
delegating dispatchers to specialized helpers keeps calculation logic
single-sourced, maintainable, and protected by the helper's internal debug
assertions.

## Examples

Bad:

```cpp
class SK_API SkMatrix {
public:
    // Redundant parallel method with ambiguous 'Check' qualifier and duplicated inline math
    SkPoint mapPointCheck(SkPoint p) const {
        if (this->hasPerspective()) {
            return this->mapPoint(p);
        } else {
            return {
                (p.fX * fMat[0] + p.fY * fMat[1]) + fMat[2],
                (p.fX * fMat[3] + p.fY * fMat[4]) + fMat[5],
            };
        }
    }
    SkPoint mapXY(SkScalar x, SkScalar y) const;
};
```

Good:

```cpp
class SK_API SkMatrix {
public:
    // Canonical method updates existing API directly and delegates without duplicating math
    SkPoint mapPoint(SkPoint p) const {
        if (this->hasPerspective()) {
            return this->mapPointPerspective(p);
        }
        return this->mapPointAffine(p);
    }
    // Fast path explicitly qualifies its precondition and encapsulates assertions
    SkPoint mapPointAffine(SkPoint p) const;
};
```
