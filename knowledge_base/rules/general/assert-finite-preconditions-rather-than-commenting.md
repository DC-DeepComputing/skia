---
name: skia/assert-finite-preconditions-rather-than-commenting
description: When code or geometry operations assume state invariants or preconditions, enforce them with SkASSERT rather than relying solely on prose comments
rubric: readability
severity: should_fix
---

**Rule.** When a function, branch, or geometry operation assumes preconditions
or state invariants—such as coordinates being finite, bounds being present, or
transform matrices having specific scale/rotation properties—enforce the
requirement explicitly with an assertion (such as SkASSERT(...)). Do not rely
solely on prose comments to state that a calculation is valid or that certain
conditions hold.

**Why.** Prose comments stating assumed invariants or preconditions are ignored
by the compiler and runtime, leaving conditions unverified when unexpected
inputs or invalid states occur. Enforcing invariants with SkASSERT documents
programmer expectations, validates them in test suites and debug builds, and
catches subtle edge cases, invalid calculations, and logic errors early.

## Examples

Bad:

```cpp
// Case 1: Precondition stated only in comment
if (convexity == SkPathConvexity::kUnknown) {
    // this is only valid if we're finite
    convexity = SkPathPriv::ComputeConvexity(fPts, fVerbs, fConicWeights);
}

// Case 2: Assumed matrix transform invariant left unasserted
// There is a rotation of 90 (Clockwise 90) or 270 (Counter clockwise 90).
if (!matrix.isScaleTranslate()) {
    const bool isClockwise = matrix.getSkewX() < 0;
```

Good:

```cpp
// Case 1: Precondition explicitly asserted
if (convexity == SkPathConvexity::kUnknown) {
    SkASSERT(this->computeFiniteBounds().has_value());
    convexity = SkPathPriv::ComputeConvexity(fPts, fVerbs, fConicWeights);
}

// Case 2: Transform invariant asserted with SkASSERT
if (!matrix.isScaleTranslate()) {
    SkASSERT(matrix.getScaleX() == 0.f && matrix.getScaleY() == 0.f &&
             matrix.getSkewX() != 0.f && matrix.getSkewY() != 0.f);
    const bool isClockwise = matrix.getSkewX() < 0;
```
