---
name: skia/tests/document-fallback-test-expectations
description: When unit tests assert against fallback or dual expected values because reference oracles deviate on edge cases, document the diverging edge-case behavior
rubric: readability
severity: nit
---

**Rule.** When authoring unit or algorithmic tests that compare a tested routine
against a reference oracle or baseline algorithm and accept fallback or
secondary expected values, add an explanatory comment detailing why the
reference oracle deviates on those specific edge cases. Clarify the differing
handling (such as coordinate clamping, degenerate geometry, or rounding rules)
that necessitates matching the fallback expected outcome.

**Why.** Tests comparing implementations against reference algorithms or
ground-truth oracles are expected to produce identical results. When a test
permits a secondary or fallback expected value without explanation, reviewers
and future maintainers cannot determine whether the disparity stems from a known
edge-case compromise, numerical limitation, or an unhandled bug in the
implementation.

## Examples

Bad:

```cpp
// Bad: Undocumented fallback expected values when reference algorithm deviates
std::array<SkPoint, 2> fallbackExp;
if (rawP1.fY < rawP0.fY) {
    fallbackExp = {rawP1, rawP0};
} else {
    fallbackExp = {rawP0, rawP1};
}
auto exp = expectedClipRef.has_value() ? expectedClipRef.value() : fallbackExp;
REPORTER_ASSERT(r, result == exp || result == fallbackExp);
```

Good:

```cpp
// Good: Explains why the reference oracle diverges on corner cases and why fallback is valid
// If a line lies entirely outside the tile but touches it at a single point (grazing a corner),
// the reference Liang-Barsky oracle reports only a single intersection. However, ClipToTile
// clamps coordinates to emit a zero-width degenerate line. We calculate a fallback expected
// value here to verify and allow that intentional clamping behavior.
std::array<SkPoint, 2> fallbackExp;
if (rawP1.fY < rawP0.fY) {
    fallbackExp = {rawP1, rawP0};
} else {
    fallbackExp = {rawP0, rawP1};
}
auto exp = expectedClipRef.has_value() ? expectedClipRef.value() : fallbackExp;
REPORTER_ASSERT(r, result == exp || result == fallbackExp);
```
