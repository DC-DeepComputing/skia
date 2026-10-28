---
name: skia/distinguish-relaxed-constraint-methods-over-overloads
description: When declaring methods with differing safety or constraint guarantees, use distinct names or explicit parameters rather than implicit parameter-omission overloads
rubric: readability
severity: should_fix
---

**Rule.** Do not rely on method overloading where omitting a parameter
implicitly changes semantic guarantees, safety invariants, or constraint checks
(such as strict subsetting versus relaxed, fast sampling). Use distinct,
descriptive method names (e.g., suffixing Fast echoing kFast_SrcRectConstraint)
or pass an explicit constraint parameter (such as SkCanvas::SrcRectConstraint or
an explicit bool strict flag).

**Why.** Predicating different safety guarantees or bounds constraints purely on
the presence or absence of an apparently unrelated parameter (such as a tile
mode) obscures behavioral contracts at call sites and causes subtle rendering
artifacts or out-of-bounds sampling. Distinct names or explicit constraint
parameters make call-site intent and safety trade-offs obvious.

## Examples

Bad:

```cpp
// Overloading where omitting tileMode implicitly relaxes subsetting constraints
sk_sp<SkShader> asShader(SkTileMode tileMode,
                         const SkSamplingOptions& sampling,
                         const SkMatrix& lm) const; // strict subsetting
sk_sp<SkShader> asShader(const SkSamplingOptions& sampling,
                         const SkMatrix& lm) const; // non-strict subsetting
```

Good:

```cpp
// Explicitly pass the constraint or give the relaxed variant a distinct name
sk_sp<SkShader> asShader(SkTileMode tileMode,
                         const SkSamplingOptions& sampling,
                         const SkMatrix& lm,
                         bool strict = true) const;

// Or use distinct method names when constraints differ:
sk_sp<SkShader> asShader(SkTileMode tileMode,
                         const SkSamplingOptions& sampling,
                         const SkMatrix& lm) const;
sk_sp<SkShader> asShaderFast(const SkSamplingOptions& sampling,
                             const SkMatrix& lm) const;
```
