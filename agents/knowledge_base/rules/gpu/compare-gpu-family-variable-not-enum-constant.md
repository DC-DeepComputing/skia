---
name: skia/gpu/compare-gpu-family-variable-not-enum-constant
description: When checking capabilities against Metal GPU family thresholds, compare the detected GPU family variable against MTLGPUFamily rather than comparing an enum constant against an integer literal
rubric: correctness
severity: should_fix
---

**Rule.** When gating Metal device capabilities or feature support on GPU
generation thresholds, compare the recorded GPU family member variable (such as
fGPUFamily) against the target MTLGPUFamily enum constant. Do not compare the
enum constant itself against an integer literal, which converts implicitly and
creates an unintentional tautological comparison that always evaluates to true.

**Why.** In Objective-C and C++, unscoped enums implicitly convert to integers.
Comparing an enum constant against an integer literal (e.g., 'MTLGPUFamilyApple7
>= 7') compiles without error but evaluates tautologically, causing capabilities
to be treated as supported on all devices regardless of actual hardware
generation.

## Examples

Bad:

```cpp
// Bad: compares enum constant against integer literal; always evaluates to true
fClampToBorderSupport = (this->isMac() || MTLGPUFamilyApple7 >= 7);
```

Good:

```cpp
// Good: compares detected GPU family variable against the enum threshold
fClampToBorderSupport = (this->isMac() || fGPUFamily >= MTLGPUFamilyApple7);
```
