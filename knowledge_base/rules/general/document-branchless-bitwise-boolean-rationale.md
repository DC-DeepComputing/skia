---
name: skia/document-branchless-bitwise-boolean-rationale
description: When using bitwise operators instead of logical operators on boolean expressions for branchless evaluation, document the performance rationale in an inline comment
rubric: readability
severity: nit
---

**Rule.** When intentionally using bitwise operators (`&`, `|`) instead of
logical operators (`&&`, `||`) on boolean operands or predicate expressions to
eliminate branching and short-circuiting overhead, document the rationale in an
inline comment. Clarify that bitwise evaluation is an intentional branchless
micro-optimization so future maintainers or linters do not refactor the logic
back to branching operators.

**Why.** Bitwise operators applied to boolean conditions can look like typos or
unidiomatic C++ to readers and static analysis tools. Documenting that the
branchless evaluation was measured and deliberate prevents well-intentioned
refactoring that re-introduces branch misprediction penalties in
performance-critical paths.

## Examples

Bad:

```cpp
// Bad: Bitwise '&' on booleans appears like an accidental typo or linter violation.
if (!(((v0.fX == 0) ^ (v0.fY == 0)) &
      axis_aligned_orthogonal(v0, v1) &
      axis_aligned_orthogonal(v1, v2))) {
    return std::nullopt;
}
```

Good:

```cpp
// Good: Comment documents intentional bitwise evaluation for branchless performance.
// Note: bitwise operators are measurably faster in microbenchmarks (no short-circuiting).
if (!(((v0.fX == 0) ^ (v0.fY == 0)) &
      axis_aligned_orthogonal(v0, v1) &
      axis_aligned_orthogonal(v1, v2))) {
    return std::nullopt;
}
```
