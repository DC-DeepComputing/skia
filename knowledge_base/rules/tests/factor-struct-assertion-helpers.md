---
name: skia/tests/factor-struct-assertion-helpers
description: When verifying struct conversions, matrices, or multi-field data in unit tests, compare against expected objects in looped or field-wise assertion helpers
rubric: design
severity: should_fix
---

**Rule.** When authoring unit tests that verify struct conversions, FFI
representations, matrices, or structured layout compatibility, compare directly
against the source or expected object rather than duplicating literal constants
across test initialization and assertions. Factor repetitive comparisons into
dedicated helper functions (such as assert_*_eq), using loops to compare
multi-dimensional array or matrix elements and field-wise comparisons for
structs.

**Why.** Hardcoding literal constants across test setup and unrolled assertions
duplicates magic values, increasing maintenance overhead and the risk of
copy-paste desynchronization. Factoring comparisons into reusable looped or
field-wise helpers comparing against expected objects directly eliminates
boilerplate, standardizes tolerance checks, and keeps assertions maintainable.

## Examples

Bad:

```cpp
// Bad: unrolled per-element assertions with duplicated literal constants
REPORTER_ASSERT(r, fabsf(skcms_matrix.vals[0][0] - 0.4124f) < 0.0001f);
REPORTER_ASSERT(r, fabsf(skcms_matrix.vals[0][1] - 0.3576f) < 0.0001f);
REPORTER_ASSERT(r, fabsf(skcms_tf.g - 2.4f) < 0.0001f);
REPORTER_ASSERT(r, fabsf(skcms_tf.a - 1.0f / 1.055f) < 0.0001f);
```

Good:

```cpp
// Good: extract looped helper for matrices and field-wise helper for structs
static void assert_matrix_eq(skiatest::Reporter* r,
                             const skcms_Matrix3x3& actual,
                             const rust_icc::Matrix3x3& expected,
                             float tolerance = 0.0001f) {
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            REPORTER_ASSERT(r, fabsf(actual.vals[row][col] - expected.vals[row][col]) < tolerance);
        }
    }
}

static void assert_tf_eq(skiatest::Reporter* r,
                         const skcms_TransferFunction& actual,
                         const rust_icc::TransferFunction& expected) {
    REPORTER_ASSERT(r, fabsf(actual.g - expected.g) < 0.0001f);
}
```
