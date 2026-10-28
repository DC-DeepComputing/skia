---
name: skia/sksl/encapsulate-coordinate-transforms-in-sksl-sampling-helpers
description: When defining SkSL texture sampling helpers, encapsulate coordinate scaling and normalization inside the helper instead of stripping transforms to accommodate pre-normalized callers
rubric: design
severity: should_fix
---

**Rule.** Do not strip coordinate normalization or transformations from shared
SkSL sampling helpers to accommodate callers whose coordinates are already
normalized. Keep coordinate scaling (such as pos * invImgSize) encapsulated
within the shared helper for unnormalized callers, and have callers with
pre-normalized coordinates invoke the built-in sample() intrinsic directly.

**Why.** Removing coordinate transformations from a shared helper reduces it to
an empty wrapper around built-in intrinsics like sample() and forces all other
call sites to duplicate coordinate scaling arithmetic. Preserving encapsulated
transformations keeps call sites concise while avoiding redundant intermediate
wrapper functions.

## Examples

Bad:

```cpp
// Helper degenerates into trivial wrapper; callers duplicate pos * invImgSize
$pure half4 $sample_image(float2 pos, sampler2D s) {
    return sample(s, pos);
}

// In caller:
half4 color = $sample_image(clampedPos * invImgSize, s);
```

Good:

```cpp
// Helper encapsulates coordinate normalization for pixel-space callers
$pure half4 $sample_image(float2 pos, float2 invImgSize, sampler2D s) {
    return sample(s, pos * invImgSize);
}

// Caller with already-normalized coordinates calls sample() directly
$pure half4 sk_hw_image_shader(float2 coords, sampler2D s) {
    return sample(s, coords);
}
```
