---
name: skia/core/assert-active-blur-pass-sigma-bounds
description: When asserting or documenting blur sigma limits in SkBlurEngine algorithms, check bounds and cite constraints from active passes rather than legacy TentPass
rubric: readability
severity: should_fix
---

**Rule.** In SkBlurEngine algorithm implementations and maxSigma() overrides,
assert blur sigma bounds against and attribute mathematical constraints to the
active pass makers (such as GaussPass or A8Pass, where 3-box filter accumulator
limits restrict sigma <= 135) rather than legacy TentPass limits (which support
sigma <= 2183). Ensure unsupported sigmas fail assertions before pass
evaluation, and cite the correct active pass when documenting maximum supported
sigma rationale.

**Why.** TentPass uses a 2-box filter supporting sigmas up to 2183, whereas
3-box filter passes like GaussPass and A8Pass restrict window sizes to <= 255
(sigma <= 135) to prevent uint32_t accumulator overflow. Asserting against 2183
in 3-box algorithms allows unsupported sigmas to bypass validation and trigger
unhandled aborts in MakeMaker(), while citing TentPass in comments misleads
maintainers about where the mathematical constraint originates.

## Examples

Bad:

```cpp
// Bad: Citing TentPass for GaussPass limit and asserting against TentPass bounds
// See analysis in description of TentPass for the max supported sigma.
float maxSigma() const override {
    static constexpr float kMaxSigma = 135.f;
    SkASSERT(SkBlurEngine::BoxBlurWindow(kMaxSigma) <= 255);
    return kMaxSigma;
}

// In an algorithm that only uses GaussianPass and A8Pass (max sigma 135):
SkASSERT(0 <= sigma && sigma <= 2183);
if (PassMaker* maker = GaussianPass<uint8_t>::MakeMaker(sigma, &alloc)) {
    return maker;
}
if (PassMaker* maker = A8Pass::MakeMaker(sigma, &alloc)) {
    return maker;
}
SK_ABORT("Sigma is out of range.");
```

Good:

```cpp
// Good: Citing GaussPass for the 3-box filter limit and asserting against actual active bound
// See analysis in description of GaussPass for the max supported sigma.
float maxSigma() const override {
    static constexpr float kMaxSigma = 135.f;
    SkASSERT(SkBlurEngine::BoxBlurWindow(kMaxSigma) <= 255);
    return kMaxSigma;
}

// Assert against the actual maximum sigma handled by the active passes:
SkASSERT(0 <= sigma && sigma <= 135);
if (PassMaker* maker = GaussianPass<uint8_t>::MakeMaker(sigma, &alloc)) {
    return maker;
}
if (PassMaker* maker = A8Pass::MakeMaker(sigma, &alloc)) {
    return maker;
}
SK_ABORT("Sigma is out of range.");
```
