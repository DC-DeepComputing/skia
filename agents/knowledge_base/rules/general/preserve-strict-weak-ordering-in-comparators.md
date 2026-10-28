---
name: skia/preserve-strict-weak-ordering-in-comparators
description: When implementing custom less-than comparators or operator< for multi-field structs, ensure strict weak ordering by checking field inequality before comparing subsequent fields
rubric: correctness
severity: should_fix
---

**Rule.** When implementing custom less-than comparators, sorting predicates, or
operator< for structures with multiple fields, ensure strict weak ordering is
preserved. Do not fall through to compare secondary fields when the primary
field comparison is false; explicitly branch on inequality of earlier fields
(e.g., using ternary checks or returning early when unequal) before evaluating
subsequent fields.

**Why.** A less-than comparator must satisfy strict weak ordering (irreflexivity
and asymmetry where a < b implies !(b < a)). Falling through to compare
secondary fields when a primary field is greater allows both a < b and b < a to
evaluate to true for different field pairings, causing undefined behavior,
infinite loops, or out-of-bounds memory access in sorting algorithms like
std::sort and SkTQSort.

## Examples

Bad:

```cpp
static bool coordinateLess(const Coordinate& a, const Coordinate& b) {
    // BUG: If a.axis > b.axis, falls through to compare secondary field,
    // allowing both a < b and b < a to evaluate to true.
    if (a.axis < b.axis) {
        return true;
    }
    return a.value < b.value;
}
```

Good:

```cpp
static bool coordinateLess(const Coordinate& a, const Coordinate& b) {
    return a.axis != b.axis ? a.axis < b.axis : a.value < b.value;
}
```
