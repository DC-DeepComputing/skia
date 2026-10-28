---
name: skia/use-directional-terms-for-interval-endpoints
description: When naming 1D sequence or interval boundary elements, use explicit directional terms like leftmost or rightmost rather than ambiguous relative terms like inner or outer
rubric: readability
severity: should_fix
---

**Rule.** When identifying boundary elements, endpoints, or duplicate entries in
1D sequences or intervals (such as gradient stops), use explicit directional or
positional terms (such as 'leftmost'/'rightmost' or 'start'/'end'). Avoid
ambiguous relative terms like 'inner' and 'outer' that can be confused between
global domain boundaries and local cluster interiors.

**Why.** In 1D intervals, relative terms like 'inner' and 'outer' are ambiguous
because they can refer either to distance from the interval's center (domain
edges vs. interior) or to duplicate elements within a local cluster (boundary
endpoints vs. interior duplicates). Explicit positional terms like 'leftmost'
and 'rightmost' unambiguously communicate sequence ordering and boundary intent.

## Examples

Bad:

```cpp
// For non-clamp tilemodes, only keep the inner stop for duplicate 0.0 or 1.0 stops, else keep outer stops.
bool ignoreOuter = dupStop && fTileMode != SkTileMode::kClamp && fPositions[i] == 0;
if (!ignoreOuter) {
    appendStop(i);
}
```

Good:

```cpp
// Ignore the leftmost stop (i) if it is a non-clamp tilemode with a duplicate stop on t = 0.
bool ignoreLeftmost = dupStop && fTileMode != SkTileMode::kClamp && fPositions[i] == 0;
if (!ignoreLeftmost) {
    appendStop(i);
}
```
