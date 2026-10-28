---
name: skia/gpu/use-skrect-roundout-and-inset-for-subset-clamp-bounds
description: When computing texture subset clamp bounds for nearest-neighbor sampling, apply SkRect::roundOut and SkRect::inset directly rather than manually intersecting with fractional bounds
rubric: readability
severity: should_fix
---

**Rule.** When computing clamp bounds for a texture subset under
nearest-neighbor filtering, round out the subset rectangle using
SkRect::roundOut and apply the half-texel linear inset using SkRect::inset. Do
not perform redundant manual component-wise intersections with the original
fractional subset bounds, as integer texel bounds inset by half a texel already
prevent sampling neighbor texels.

**Why.** For nearest-neighbor sampling, clamping coordinates to integer texel
boundaries inset by half a texel guarantees sampling boundary texel centers
without bleeding into adjacent texels; intersecting with the original fractional
coordinates is mathematically redundant. Using SkRect::roundOut and
SkRect::inset avoids verbose, error-prone manual per-coordinate min/max
expressions.

## Examples

Bad:

```cpp
const float l = imgData.fSubset.left();
const float t = imgData.fSubset.top();
const float r = imgData.fSubset.right();
const float b = imgData.fSubset.bottom();
gatherer->write((imgData.fSampling.filter == SkFilterMode::kNearest)
        ? SkRect::MakeLTRB(std::max(std::floor(l) + kLinearInset, l),
                           std::max(std::floor(t) + kLinearInset, t),
                           std::min(std::ceil(r) - kLinearInset, r),
                           std::min(std::ceil(b) - kLinearInset, b))
        : SkRect::MakeLTRB(l + kLinearInset, t + kLinearInset,
                           r - kLinearInset, b - kLinearInset));
```

Good:

```cpp
SkRect subsetInsetClamp = imgData.fSubset;
if (imgData.fSampling.filter == SkFilterMode::kNearest) {
    subsetInsetClamp.roundOut(&subsetInsetClamp);
}
subsetInsetClamp.inset(kLinearInset, kLinearInset);
gatherer->write(subsetInsetClamp);
```
