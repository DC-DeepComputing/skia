---
name: skia/core/specify-reference-value-in-relative-tolerance-comments
description: When documenting relative tolerance or ratio comparisons in comments, explicitly specify the target reference value rather than bare 'within tolerance'
rubric: readability
severity: should_fix
---

**Rule.** When documenting relative tolerance, ratio, or scaling comparisons in
explanatory comments, explicitly state the target reference value being compared
against (e.g., 'within tolerance of 1.0') rather than using bare 'within
tolerance'. Ensure comments explaining algebraic simplifications of ratio bounds
clearly specify the baseline reference value and comparison equivalence.

**Why.** Describing a ratio or quantity as being 'within tolerance' without
stating the target reference value is mathematically ambiguous, as readers
typically assume 'within tolerance' denotes a distance from zero. Specifying the
target reference value (such as 1.0 for ratios) eliminates ambiguity and
clarifies the mathematical intent for maintainers.

## Examples

Bad:

```cpp
// The ellipse is considered relatively circular if either `rx/ry` or `ry/rx` is within
// `tolerance`, but this is equivalent to comparing the absolute difference...
return (rx <= tolerance && ry <= tolerance) ||
       std::abs(rx - ry) <= tolerance * std::max(rx, ry);
```

Good:

```cpp
// The ellipse is considered relatively circular if either `rx/ry` or `ry/rx` is within
// `tolerance` of 1.0, but this is equivalent to comparing the absolute difference...
return (rx <= tolerance && ry <= tolerance) ||
       std::abs(rx - ry) <= tolerance * std::max(rx, ry);
```
