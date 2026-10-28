---
name: skia/modules/query-non-empty-range-in-half-open-interval
description: When querying SkRange or TextRange half-open intervals for single-index containment, query a 1-length interval [index, index + 1) rather than empty [index, index)
rubric: correctness
severity: nit
---

**Rule.** When querying whether an SkRange (such as TextRange) half-open
interval [start, end) contains a single character or code unit index, query a
1-length interval [index, index + 1) or compare start <= index && index < end.
Do not query an empty interval [index, index).

**Why.** Under half-open interval containment rules (start <= other.start && end
>= other.end), an empty range [index, index) is trivially contained by both the
preceding range [..., index) and succeeding range [index, ...). Querying [index,
index) incorrectly matches boundary code units against predecessor lines or
clusters.

## Examples

Bad:

```cpp
// Empty range [codeUnitIndex, codeUnitIndex) matches predecessor line because end >= query.end
if (line.text().contains({codeUnitIndex, codeUnitIndex})) {
    return lineNumber;
}
```

Good:

```cpp
// Query 1-length range [codeUnitIndex, codeUnitIndex + 1) for character containment
if (line.text().contains({codeUnitIndex, codeUnitIndex + 1})) {
    return lineNumber;
}
```
