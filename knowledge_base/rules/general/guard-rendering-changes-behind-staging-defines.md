---
name: skia/guard-rendering-changes-behind-staging-defines
description: When modifying rendering math, color conversions, or historical thresholds and cutoffs, guard changes behind a preprocessor staging define to prevent breaking client pixel tests
rubric: correctness
severity: blocking
---

**Rule.** When modifying rendering math, color conversion coefficients, or
historical rendering thresholds, cutoffs, and bailout conditions (even when
mathematically more precise), do not alter defaults unconditionally. Retain
legacy behavior by default or guard updated values behind a compile-time staging
define. Use an opt-out legacy guard (#if !defined(SK_USE_LEGACY_...) or #if
defined(SK_SUPPORT_LEGACY_...)) when only Chromium needs rebaselining, or an
opt-in guard (#if defined(SK_USE_NEW_...)) when coordinating across multiple
downstream clients, allowing embedders to rebaseline golden images
incrementally.

**Why.** Altering long-standing rendering math, color conversions, or filter
cutoffs changes pixel output across clients like Chromium, breaking automated
rolls and risking web specification regressions. Staging changes behind
compile-time defines decouples the rollout, allowing clients like Chromium or
Android to evaluate and rebaseline golden images incrementally without stalling
autorolls.

## Examples

Bad:

```cpp
// src/core/SkYUVMath.cpp
// Unconditionally updating color conversion math alters layout test baselines
constexpr float JPEG_full_rgb_to_yuv[] = {
    0x1.322d0ep-2f, 0x1.2c8b42p-1f, 0x1.d2f1aap-4f, ...
};

// src/core/SkMaskBlurFilter.cpp
// Unconditionally altering historical filter cutoffs breaks web specs and tests
bool SkMaskBlurFilter::hasNoBlur() const {
    constexpr double kNoWindowSigma = 0.531923; // Mathematically derived threshold
    return fSigmaW < kNoWindowSigma && fSigmaH <= kNoWindowSigma;
}
```

Good:

```cpp
// src/core/SkYUVMath.cpp
// Guard updated coefficients behind a staging define so clients can rebaseline
constexpr float JPEG_full_rgb_to_yuv[] = {
#if defined(SK_YUV_COLOR_SPACE_HIGH_PRECISION)
    0x1.322d0ep-2f, 0x1.2c8b42p-1f, 0x1.d2f1aap-4f, ...
#else
    0.299000f,      0.587000f,      0.114000f, ...
#endif
};

// src/core/SkMaskBlurFilter.cpp
// Retain legacy cutoff by default under an opt-in staging define
bool SkMaskBlurFilter::hasNoBlur() const {
#if defined(SK_USE_LARGER_NO_BLUR_THRESHOLD)
    constexpr double kNoWindowSigma = 0.531923;
#else
    constexpr double kNoWindowSigma = 1./3.; // Historical default
#endif
    return fSigmaW < kNoWindowSigma && fSigmaH <= kNoWindowSigma;
}
```
