---
name: skia/tests/cover-minimal-degenerate-inputs
description: When testing algorithms operating on sequences, curves, or point sets, include explicit unit test coverage for minimal degenerate inputs such as single-element or single-point cases
rubric: testing
severity: should_fix
---

**Rule.** When authoring unit tests or regression tests for algorithms that
operate on sequences, curves, polylines, or point sets, include explicit test
coverage for minimal degenerate inputs (such as a single point, single element,
or minimal boundary size). Do not test only multi-point or multi-element cases
(such as N >= 2 or N >= 3).

**Why.** Algorithms that process sequences, curves, or point sets often
implicitly assume multi-element inputs when computing differences, slopes,
subdivisions, or neighbor indices. Minimal inputs like a single point or element
are frequent sources of edge-case bugs (such as division by zero, out-of-bounds
indexing, or null dereferences) and require explicit unit test verification.

## Examples

Bad:

```cpp
// Test table only exercises multi-point inputs (N >= 2)
const SlopeTest tests[] = {
    {
        .name = "N=2, two-point difference",
        .curve = { .fControlPoints = {{0.f, 0.f}, {1.f, 1.f}} },
        .expectedM = 1.f,
    },
    {
        .name = "Three-point endpoint slope",
        .curve = { .fControlPoints = {{0.f, 0.f}, {1.f, 1.f}, {2.f, 0.f}} },
        .expectedM = 0.5f,
    },
};
```

Good:

```cpp
// Explicitly cover the minimal degenerate input (single point / N=1) alongside multi-point cases
const SlopeTest tests[] = {
    {
        .name = "SinglePoint",
        .curve = { .fControlPoints = {{1.f, 1.f}} },
        .expectedM = 0.f,
    },
    {
        .name = "N=2, two-point difference",
        .curve = { .fControlPoints = {{0.f, 0.f}, {1.f, 1.f}} },
        .expectedM = 1.f,
    },
};
```
