---
name: skia/document-derived-loop-bounds
description: When using hardcoded loop bounds or iteration limits derived from capacity or domain bounds, document the mathematical derivation and underlying limit in comments
rubric: readability
severity: nit
---

**Rule.** When implementing fixed-depth loops, binary searches, or unrolled
iterations whose bounds or step counts are derived from a capacity limit or
domain bound (e.g., binary search depth ceil(log2(N))), document the
mathematical derivation and maximum capacity in an explanatory comment.
Explicitly state the formula and underlying capacity bound (e.g., log2(32)=5)
rather than leaving an unexplained numeric literal in the loop condition.

**Why.** Hardcoded loop bounds and iteration counts in fixed-depth algorithms
look like arbitrary magic numbers to readers and reviewers. Documenting the
derivation formula and underlying capacity limit clarifies why the bound is
sufficient for correctness and prevents future maintainers from inadvertently
shrinking or relaxing the iteration count during refactoring.

## Examples

Bad:

```cpp
// Bad: hardcoded iteration count without documenting the derivation or domain capacity
// Binary search for the interval containing x. This will require sampling at most 5 more
// control points.
for (int step = 0; step < 5; ++step) {
    int mid = (low + high) / 2;
    // ...
}
```

Good:

```cpp
// Good: documents the mathematical formula and capacity bound (log2(32)=5)
// Binary search for the interval containing x. This will require sampling at most
// log2(32)=5 more control points.
for (int step = 0; step < 5; ++step) {
    int mid = (low + high) / 2;
    // ...
}
```
