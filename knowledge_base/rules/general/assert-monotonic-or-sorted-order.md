---
name: skia/assert-monotonic-or-sorted-order
description: When an algorithm or loop assumes input data is sorted or monotonically advancing, verify the ordering invariant with SkASSERT or std::is_sorted
rubric: readability
severity: nit
---

**Rule.** When an algorithm, loop, or helper relies on input data, spans, or
indices being sorted or monotonically advancing, verify that ordering invariant
with a debug assertion using SkASSERT (such as `SkASSERT(prev < curr)` or
`SkASSERT(prev <= curr)`). For sorted arrays or spans passed into routines that
assume ordering, assert the precondition across the range using
`SkASSERT(std::is_sorted(...))`.

**Why.** Algorithms that process ordered sequences (such as run-length grouping,
interval sweeping, or binary search) silently produce corrupted state or
incorrect results when unexpected out-of-order data is encountered. Asserting
monotonicity documents the algorithmic invariant and immediately catches
ordering regressions or malformed inputs in debug builds.

## Examples

Bad:

```cpp
// Run-length or sequence grouping assuming monotonic progression
if (currentPage != thisPage) {
    currentPage = thisPage;
    currentRun = 0;
}

// Function assuming pre-sorted range
void processRanges(SkSpan<const int> values) {
    // Assumes values are sorted without verifying
    for (size_t i = 1; i < values.size(); ++i) {
        handleInterval(values[i - 1], values[i]);
    }
}
```

Good:

```cpp
// Run-length or sequence grouping assuming monotonic progression
if (currentPage != thisPage) {
    SkASSERT(currentPage < thisPage);
    currentPage = thisPage;
    currentRun = 0;
}

// Function assuming pre-sorted range
void processRanges(SkSpan<const int> values) {
    SkASSERT(std::is_sorted(values.begin(), values.end()));
    for (size_t i = 1; i < values.size(); ++i) {
        handleInterval(values[i - 1], values[i]);
    }
}
```
