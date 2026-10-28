---
name: skia/document-mismatched-parallel-array-bounds
description: When processing parallel arrays with mismatched sizes or asymmetric loop bounds, document the sizing differences and boundary invariants in inline comments
rubric: readability
severity: nit
---

**Rule.** When copying, iterating over, or slicing parallel arrays with
mismatched lengths (such as n items versus n+1 trailing bounds, offsets, or
positions) or applying asymmetric loop bounds across them, document the sizing
differences and boundary invariants in inline comments. Explain why certain
arrays are indexed conditionally or omitted, and clarify what downstream
invariants or sentinel conventions the bounds preserve.

**Why.** In structures with correlated parallel arrays, mismatched capacities
(such as storing an extra trailing position or boundary sentinel) and asymmetric
copy bounds appear arbitrary or like off-by-one errors to reviewers and
maintainers. Explicitly documenting sizing discrepancies and range invariants
clarifies why arrays are bounded differently and prevents accidental refactoring
regressions or buffer mismatches.

## Examples

Bad:

```cpp
auto index = i - glyphs.start;
if (i < glyphs.end) {
    piece->fGlyphs[index] = run->fGlyphs[i];
    piece->fClusterIndexes[index] = run->fClusterIndexes[i];
}
piece->fPositions[index] = run->fPositions[i] - zero;
piece->fOffsets[index] = run->fOffsets[i];
```

Good:

```cpp
auto index = i - glyphs.start;
if (i < glyphs.end) {
    // There are only n glyphs in a run, not n+1.
    piece->fGlyphs[index] = run->fGlyphs[i];

    // fClusterIndexes n+1 is already set to the end of the run.
    // Do not attempt to overwrite this value with the cluster index
    // that starts the next Run.
    // It is assumed later that all clusters in a Run are contained by the Run.
    piece->fClusterIndexes[index] = run->fClusterIndexes[i];
}
piece->fPositions[index] = run->fPositions[i] - zero;
piece->fOffsets[index] = run->fOffsets[i];
```
