---
name: skia/tests/document-approximation-sampling-parameters
description: When defining sample counts, step sizes, or tolerances for numerical approximations in tests, document what the approximation represents and the sampling strategy
rubric: readability
severity: should_fix
---

**Rule.** When defining sample counts, step sizes (such as dt), or iteration
bounds for numerical approximations in tests (such as discrete parametric curve
sampling or distance estimation), document what the approximation represents and
the sampling strategy with an explanatory comment. Clarify the geometric or
mathematical purpose of the samples so maintainers understand the verification
logic without needing to reverse-engineer loop increments and division formulas.

**Why.** Constants controlling numerical approximations, sample counts, and step
sizes appear arbitrary without contextual comments. Documenting what the
sampling routine evaluates and the spacing or resolution of the steps ensures
test intent is clear and prevents maintainers from misinterpreting approximation
bounds or removing necessary sample density during refactoring.

## Examples

Bad:

```cpp
// Bad: Opaque sample count and step size without explanation of approximation strategy
static constexpr int kLineSamples = 100;
static constexpr float kLineDt = 1.0f / static_cast<float>(kLineSamples);

for (int i = 0; i < kLineSamples; i += 4) {
    skvx::float4 t03 = skvx::float4(i, i + 1, i + 2, i + 3) * kLineDt;
    ...
}
```

Good:

```cpp
// Good: Documents the number of evenly spaced parametric samples and their geometric purpose
// The number of evenly spaced parametric samples evaluated to estimate distance to a curve.
static constexpr int kLineSamples = 100;
static constexpr float kLineDt = 1.0f / static_cast<float>(kLineSamples);

for (int i = 0; i < kLineSamples; i += 4) {
    skvx::float4 t03 = skvx::float4(i, i + 1, i + 2, i + 3) * kLineDt;
    ...
}
```
