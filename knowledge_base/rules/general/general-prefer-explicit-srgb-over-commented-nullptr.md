---
name: skia/general-prefer-explicit-srgb-over-commented-nullptr
description: When passing sRGB color space arguments, pass explicit factories like SkColorSpace::MakeSRGB() instead of commented nullptr defaults to keep code self-documenting
rubric: readability
severity: nit
---

**Rule.** When an API parameter accepts an explicit color space and sRGB is
intended, pass an explicit factory such as SkColorSpace::MakeSRGB() rather than
passing nullptr paired with inline explanatory comments (such as
/*space=*/nullptr // null == sRGB). Avoid micro-optimizing reference-count churn
on singleton color spaces in non-hot paths.

**Why.** While Skia historically treats untagged or nullptr color spaces as sRGB
in many contexts, passing nullptr with explanatory comments obscures intent and
leaves readers unsure whether the color is truly untagged or explicitly sRGB.
Calling SkColorSpace::MakeSRGB() expresses caller semantics unambiguously, and
singleton ref-count overhead is negligible outside profiled hotspots.

## Examples

Bad:

```cpp
// Bad: relying on implicit nullptr sRGB fallback with explanatory comment
return SkShaders::Color(SkColor4f::FromColor(color), /*space=*/nullptr); // null == sRGB
```

Good:

```cpp
// Good: self-documenting explicit color space factory
return SkShaders::Color(SkColor4f::FromColor(color), SkColorSpace::MakeSRGB());
```
