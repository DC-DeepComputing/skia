---
name: skia/document-matrix-element-validation-heuristics
description: When validating matrices or transform structures via ad-hoc element index checks, document the underlying domain invariant and row or column layout
rubric: readability
severity: nit
---

**Rule.** When performing non-obvious matrix or transform validation by
inspecting specific element indices (such as checking for non-zero values across
rows or columns to detect missing tags or degenerate data), document the
underlying domain invariant in an inline comment. Explain what the rows and
columns represent (e.g., colorant contributions to XYZ) and why testing those
specific indices guarantees valid data.

**Why.** Reviewers and maintainers cannot deduce from raw matrix indices (such
as matrix.v[0][1]) what mathematical or color-space properties are being
verified. Documenting what rows and columns encode and how the element checks
guard against missing metadata or degenerate transforms makes the validation
logic understandable and prevents accidental removal or breakage during
refactoring.

## Examples

Bad:

```cpp
let red_valid = matrix.v[0][0] != 0.0 || matrix.v[1][0] != 0.0 || matrix.v[2][0] != 0.0;
let green_valid = matrix.v[0][1] != 0.0 || matrix.v[1][1] != 0.0 || matrix.v[2][1] != 0.0;
let blue_valid = matrix.v[0][2] != 0.0 || matrix.v[1][2] != 0.0 || matrix.v[2][2] != 0.0;
out.has_to_xyzd50 = red_valid && green_valid && blue_valid;
```

Good:

```cpp
// Validate colorant matrix: each column must have at least one non-zero value.
// Each column represents a colorant's contribution to XYZ (red, green, blue).
// An all-zero column would mean that colorant contributes nothing, which is invalid.
// moxcms returns a zero matrix when colorant tags (rXYZ, gXYZ, bXYZ) are missing.
let red_valid = matrix.v[0][0] != 0.0 || matrix.v[1][0] != 0.0 || matrix.v[2][0] != 0.0;
let green_valid = matrix.v[0][1] != 0.0 || matrix.v[1][1] != 0.0 || matrix.v[2][1] != 0.0;
let blue_valid = matrix.v[0][2] != 0.0 || matrix.v[1][2] != 0.0 || matrix.v[2][2] != 0.0;
out.has_to_xyzd50 = red_valid && green_valid && blue_valid;
```
