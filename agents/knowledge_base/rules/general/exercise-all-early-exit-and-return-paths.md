---
name: skia/exercise-all-early-exit-and-return-paths
description: When adding or modifying validation and early-exit branches in core functions, add unit tests exercising all return and failure paths including non-finite edge cases
rubric: testing
severity: should_fix
---

**Rule.** When adding or modifying validation logic, fallible computations, or
early-exit branches in core functions (such as matrix inversion, geometry
parsing, or scalar normalization), add unit tests that explicitly exercise every
return and failure path. Verify non-finite floating-point edge cases (NaN,
infinity, and subnormal/underflow/overflow values) and test all output parameter
variations, asserting that passing nullptr behaves consistently with passing a
valid destination pointer.

**Why.** Omitting failure and edge-case branches in unit tests leaves fallback
logic and bailout guards unverified, concealing arithmetic bugs, silent data
corruption, or unintended behavioral drift. Testing non-finite floating-point
inputs and both null and non-null output pointers verifies that query-only and
mutating modes remain consistent and that invalid inputs bail out safely.

## Examples

Bad:

```cpp
// Modified matrix inversion to add failure branches for non-finite inputs,
// but only added or ran unit tests checking valid invertible matrices:
SkMatrix mat;
mat.setScale(2.f, 2.f);
SkMatrix inverse;
REPORTER_ASSERT(reporter, mat.invert(&inverse));
```

Good:

```cpp
// Exercise all early-exit and non-finite failure paths with and without destination pointer
SkMatrix mat;
SkMatrix inverse;

// Subnormal scale causing inverse overflow to infinity
mat.setAll(std::numeric_limits<float>::denorm_min(), 0.f, 0.f,
           0.f, 1.f, 0.f,
           0.f, 0.f, 1.f);
REPORTER_ASSERT(reporter, !mat.invert(nullptr));
REPORTER_ASSERT(reporter, !mat.invert(&inverse));

// Non-finite translation (NaN)
mat.setAll(2.f, 0.f, std::numeric_limits<float>::quiet_NaN(),
           0.f, 2.f, 0.f,
           0.f, 0.f, 1.f);
REPORTER_ASSERT(reporter, !mat.invert(nullptr));
REPORTER_ASSERT(reporter, !mat.invert(&inverse));
```
