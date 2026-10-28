---
name: skia/document-intentional-sequence-gaps
description: When defining sequences of enums, constants, or binned cases with intentional gaps or irregular progressions, document the rationale in comments
rubric: readability
severity: nit
---

**Rule.** When defining sequences of enums, constants, or binned cases that
follow an established stepping or progression, document any intentional gaps,
skipped values, or changes in step size with an explanatory inline comment.
Clarify why the anomaly exists (such as coarse binning for larger values or
reserved ranges) so readers and reviewers know the omission is deliberate and
not an accidental typo or bug.

**Why.** Sequences of enums or numeric constants that follow a regular
progression (such as steps of 4 or power-of-two multiples) lead readers and
reviewers to assume missing entries are accidental bugs or copy-paste
oversights. Documenting intentional gaps or altered step sizes prevents
erroneous bug reports, mistaken cleanups, or well-intentioned patches attempting
to 'fix' the missing values.

## Examples

Bad:

```cpp
enum class StableKey : uint32_t {
    ...
    k1DBlur16,
    k1DBlur20,
    k1DBlur28,
};
```

Good:

```cpp
enum class StableKey : uint32_t {
    ...
    k1DBlur16,
    k1DBlur20,
    // For large kernels we bin by a multiple of eight (so no k1DBlur24)
    k1DBlur28,
};
```
