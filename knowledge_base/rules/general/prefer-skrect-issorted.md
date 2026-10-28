---
name: skia/prefer-skrect-issorted
description: When checking or asserting that an SkRect is sorted, use rect.isSorted() instead of comparing rect.makeSorted() == rect or open-coding coordinate checks
rubric: readability
severity: nit
---

**Rule.** When checking or asserting that an SkRect has sorted coordinate
ordering (fLeft <= fRight and fTop <= fBottom), call rect.isSorted() rather than
comparing rect.makeSorted() == rect or manually checking coordinate pairs. Use
the dedicated member predicate across runtime assertions (SkASSERT), debug
checks, and conditional branching.

**Why.** SkRect::isSorted() is the dedicated, idiomatic predicate for verifying
rectangle coordinate order. Comparing rect.makeSorted() == rect redundantly
constructs a temporary rectangle, computes min and max across all four
coordinates, and compares floats, which obscures intent and incurs unnecessary
work compared to the direct member predicate.

## Examples

Bad:

```cpp
SkASSERT(fBounds.makeSorted() == fBounds);
```

Good:

```cpp
SkASSERT(fBounds.isSorted());
```
