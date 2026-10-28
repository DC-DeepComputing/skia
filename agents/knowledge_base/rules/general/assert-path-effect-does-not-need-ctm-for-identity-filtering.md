---
name: skia/assert-path-effect-does-not-need-ctm-for-identity-filtering
description: When applying an SkPathEffect outside rasterization using an identity matrix or 3-arg filterPath, assert !effect->needsCTM() with SkASSERT
rubric: correctness
severity: should_fix
---

**Rule.** When evaluating or applying an SkPathEffect outside the rasterization
pipeline using an identity transform (SkMatrix::I()) or the 3-argument
filterPath() overload, assert that the effect does not require a current
transformation matrix using SkASSERT(!effect->needsCTM()). Do not invoke
filterPath with an identity matrix on arbitrary or untested path effects without
verifying CTM-invariance.

**Why.** Certain SkPathEffects depend on the CTM to correctly evaluate geometry
and metrics (such as device-space stroking or dash scaling). In subsystems that
operate in local coordinate space (such as SkSG scene graphs or GrStyle path
caching), passing an identity matrix to a CTM-dependent path effect silently
produces distorted or incorrect geometry; asserting !effect->needsCTM() catches
invalid effect usage early in debug builds.

## Examples

Bad:

```cpp
if (const auto trim = SkTrimPathEffect::Make(fStart, fStop, fMode)) {
    SkStrokeRec rec(SkStrokeRec::kHairline_InitStyle);
    // Bad: invoking filterPath with SkMatrix::I() without verifying the effect is CTM-invariant
    SkAssertResult(trim->filterPath(&builder, path, &rec, nullptr, SkMatrix::I()));
}
```

Good:

```cpp
if (const auto trim = SkTrimPathEffect::Make(fStart, fStop, fMode)) {
    SkStrokeRec rec(SkStrokeRec::kHairline_InitStyle);
    // Good: assert that the effect does not require a CTM before filtering with identity
    SkASSERT(!trim->needsCTM());
    SkAssertResult(trim->filterPath(&builder, path, &rec, nullptr, SkMatrix::I()));
}
```
