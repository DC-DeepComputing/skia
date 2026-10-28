---
name: skia/tests/test-multi-parameter-formulas-in-isolation
description: When writing unit tests for multi-parameter mathematical formulas or component mixing functions, test each parameter or coefficient in isolation before testing combined cases
rubric: testing
severity: should_fix
---

**Rule.** When authoring unit tests for multi-parameter mathematical formulas,
color mixing functions, transfer functions, or polynomial evaluations, test each
parameter or coefficient in isolation (such as basis or one-hot parameter sets)
in addition to representative combined cases. Do not test only combined inputs
where multiple parameters vary simultaneously or omit individual parameters
entirely.

**Why.** Omitting individual parameters or only testing combined values leaves
individual parameter code paths unexercised and can mask bugs such as swapped
coefficients, incorrect weights, sign errors, or accidental typos that cancel
out in combined calculations. Testing each parameter in isolation verifies its
specific contribution to the formula.

## Examples

Bad:

```cpp
// Bad: Only tests combined parameters, leaving fRed, fGreen, and fBlue untested.
skhdr::Agtm::ComponentMixingFunction mix;
mix.fMax = 0.125f;
mix.fMin = 0.25f;
mix.fComponent = 0.625f;
actual = skhdr::AgtmHelpers::EvaluateComponentMixingFunction(mix, input);
REPORTER_ASSERT(r, actual == expected);
```

Good:

```cpp
// Good: Tests each parameter in isolation before testing combined weights.
test("Red only", skhdr::Agtm::ComponentMixingFunction({.fRed = 1.f}), ...);
test("Green only", skhdr::Agtm::ComponentMixingFunction({.fGreen = 1.f}), ...);
test("Blue only", skhdr::Agtm::ComponentMixingFunction({.fBlue = 1.f}), ...);
test("Max only", skhdr::Agtm::ComponentMixingFunction({.fMax = 1.f}), ...);
test("Combined", skhdr::Agtm::ComponentMixingFunction({.fMax = 0.75f, .fComponent = 0.25f}), ...);
```
