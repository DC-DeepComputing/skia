---
name: skia/rust/validate-icc-colorant-matrix-columns
description: When validating ICC colorant matrices (such as to_xyzd50), verify each colorant column independently contains non-zero entries instead of testing cells with logical OR
rubric: correctness
severity: should_fix
---

**Rule.** When validating whether an ICC colorant matrix (such as `to_xyzd50` or
`toXYZD50`) is present and valid, verify that each colorant channel column (red,
green, blue) independently contains at least one non-zero value. Do not validate
matrix presence by testing arbitrary cells combined with logical OR (`||`).

**Why.** In an ICC colorant matrix, each column represents an individual
colorant's (red, green, or blue) contribution to XYZ space. If underlying
profile tags (rXYZ, gXYZ, bXYZ) are missing or zeroed out, an all-zero column
indicates that colorant contributes nothing, making the matrix invalid; testing
arbitrary cells with logical OR permits profiles with missing or zeroed colorant
columns to falsely pass validation.

## Examples

Bad:

```cpp
// Flawed: arbitrary non-zero cells pass even if a colorant column is all zeros
out.has_to_xyzd50 = matrix.v[0][0] != 0.0
    || matrix.v[0][1] != 0.0
    || matrix.v[0][2] != 0.0
    || matrix.v[1][0] != 0.0;
```

Good:

```cpp
// Verify that red (col 0), green (col 1), and blue (col 2) each have non-zero data
let red_valid = matrix.v[0][0] != 0.0 || matrix.v[1][0] != 0.0 || matrix.v[2][0] != 0.0;
let green_valid = matrix.v[0][1] != 0.0 || matrix.v[1][1] != 0.0 || matrix.v[2][1] != 0.0;
let blue_valid = matrix.v[0][2] != 0.0 || matrix.v[1][2] != 0.0 || matrix.v[2][2] != 0.0;
out.has_to_xyzd50 = red_valid && green_valid && blue_valid;
```
