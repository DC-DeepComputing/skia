---
name: skia/default-null-destination-color-space-to-source
description: When resolving untagged or null color spaces in legacy fallback logic, default a null destination to the resolved source color space rather than sRGB
rubric: correctness
severity: should_fix
---

**Rule.** When implementing legacy color space fallback logic or equality
checks, default a null destination color space to the resolved source color
space (dstCS = srcCS), not sRGB (sk_srgb_singleton()), after defaulting a null
source to sRGB. Follow the required evaluation order: if !srcCS, srcCS defaults
to sRGB; then if !dstCS, dstCS defaults to srcCS.

**Why.** Skia's documented color space convention (site/docs/user/color.md and
SkColorSpaceXformSteps) treats untagged sources as sRGB, but treats untagged
destinations as fluidly matching the source color space. Defaulting a null
destination to sRGB instead of the resolved source color space causes non-sRGB
sources to falsely register a color space mismatch when drawing into untagged
destinations, triggering unnecessary conversions and bypassing no-op
optimizations.

## Examples

Bad:

```cpp
const SkColorSpace* srcCS = fImage->getColorSpace() ? fImage->getColorSpace()
                                                    : sk_srgb_singleton();
// Flawed: defaulting null destination to sRGB forces conversion for non-sRGB sources.
const SkColorSpace* dstCS = ctx.colorSpace() ? ctx.colorSpace()
                                             : sk_srgb_singleton();
bool needsXform = !SkColorSpace::Equals(srcCS, dstCS);
```

Good:

```cpp
const SkColorSpace* srcCS = fImage->getColorSpace() ? fImage->getColorSpace()
                                                    : sk_srgb_singleton();
// Correct: untagged destination matches source, preserving unmanaged drawing.
const SkColorSpace* dstCS = ctx.colorSpace() ? ctx.colorSpace()
                                             : srcCS;
bool needsXform = !SkColorSpace::Equals(srcCS, dstCS);
```
