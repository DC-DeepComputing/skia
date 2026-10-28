---
name: skia/encapsulate-fallback-and-precondition-checks-in-helpers
description: When defining or invoking shared helpers, encapsulate capability, identity, and fallback checks internally instead of duplicating guards or fast-path checks across callers
rubric: safety
severity: should_fix
---

**Rule.** Encapsulate capability, fallback, and identity or no-op bypass checks
inside shared helper functions rather than duplicating conditional dispatch
across call sites. When implementing transformation or processing helpers, check
for identity or no-op conditions (such as identity flags or zero transforms)
internally so all callers consistently take the fast path. Callers should invoke
the helper directly without gating on fast-path preconditions or capability
checks unless an alternative branch handles the rejected condition.

**Why.** Duplicating capability or fast-path checks across call sites leads to
maintenance divergence, code duplication, and missed optimizations when
individual callers omit the check. Furthermore, prematurely gating calls to
fallback-capable helpers can bypass internal fallback paths or cause unnecessary
failures on valid inputs.

## Examples

Bad:

```cpp
// Caller duplicates capability check and fallback ternary:
std::optional<AutoLayerForImageFilter> layer =
        this->topDevice()->useDrawCoverageMaskForMaskFilters()
                ? this->attemptBlurredRRectDraw(SkRRect::MakeRect(r), paint, kPredrawFlags)
                : this->aboutToDraw(paint, &r, kPredrawFlags);

// Caller gates on fast-path check, bypassing helper's internal fallback logic:
if (typefaceAsset && typefaceAsset->getMemoryBase()) {
    face = stream_to_face(std::move(typefaceAsset), index);
}

// Caller checks no-op flag externally; other callers omit check and miss fast path:
$pure half4 sk_hw_image_shader(...) {
    half4 sampleColor = $sample_image(coords, invImgSize, readSwizzle, s);
    if (csXformFlags == 0) return sampleColor;
    return sk_color_space_transform(sampleColor, csXformFlags, ...);
}
```

Good:

```cpp
// Helper encapsulates capability check and falls back directly:
std::optional<AutoLayerForImageFilter> SkCanvas::attemptBlurredRRectDraw(...) {
    if (!this->topDevice()->useDrawCoverageMaskForMaskFilters()) {
        return this->aboutToDraw(paint, &bounds, flags);
    }
    // ... specialized drawing logic ...
}

// Caller invokes fallback-capable helper directly without premature guards:
if (typefaceAsset) {
    face = stream_to_face(std::move(typefaceAsset), index);
}

// Helper encapsulates no-op early return internally for all callers:
$pure half4 sk_color_space_transform(half4 halfColor, int flags, ...) {
    if (flags == 0) {
        return halfColor;
    }
    // ... perform color space transformation ...
}
```
