---
name: skia/tests/tests-exercise-nullable-parameters-in-random-generators
description: When generating randomized test cases for APIs accepting optional pointer arguments, exercise both non-null pointers and nullptr rather than unconditionally passing non-null values
rubric: testing
severity: should_fix
---

**Rule.** In randomized test generators and test fixtures calling APIs that
accept optional pointer arguments (such as WithWorkingFormat where nullptr
indicates destination or default fallback), randomly pass both valid pointers
and nullptr (e.g., using rand->nextBool() ? &value : nullptr) rather than
unconditionally supplying non-null pointers.

**Why.** APIs accepting optional pointer parameters typically treat nullptr as
an explicit signal to take fallback, default, or inherited behaviors (such as
adopting a destination surface's gamut or transfer function). Unconditionally
passing non-null values leaves the nullptr fallback path unexercised in
randomized testing, masking regressions in default pipeline construction.

## Examples

Bad:

```cpp
const skcms_TransferFunction* tf = &random_xfer_function(rand);
const skcms_Matrix3x3* gamut = &random_gamut(rand);
sk_sp<SkColorFilter> cf =
        SkColorFilterPriv::WithWorkingFormat(std::move(childCF), tf, gamut, &unpremul);
```

Good:

```cpp
const skcms_TransferFunction* tf = rand->nextBool() ? &random_xfer_function(rand) : nullptr;
const skcms_Matrix3x3* gamut = rand->nextBool() ? &random_gamut(rand) : nullptr;
sk_sp<SkColorFilter> cf =
        SkColorFilterPriv::WithWorkingFormat(std::move(childCF), tf, gamut, &unpremul);
```
