---
name: skia/gpu/negate-low-fragment-precision-for-high-precision-gpu-ops
description: When querying GrShaderCaps::fHasLowFragmentPrecision to gate high-precision GPU operations or passes, negate the flag rather than passing it directly
rubric: correctness
severity: should_fix
---

**Rule.** When querying or passing GrShaderCaps::fHasLowFragmentPrecision to
gate operations that require high coordinate or shader precision (such as
GPU-rendered lazy picture subsets or oval geometry), negate the flag
(!fHasLowFragmentPrecision). Do not pass the capability flag directly to
affirmative conditions or parameters expecting high precision support.

**Why.** GrShaderCaps::fHasLowFragmentPrecision is a negative capability flag
indicating that the hardware lacks sufficient fragment precision (such as
Mali-400 GPUs). Passing it directly to an affirmative parameter inverts the
logic by enabling precision-sensitive GPU paths on low-precision hardware while
disabling them on standard GPUs.

## Examples

Bad:

```cpp
// Inverts logic: enables GPU rendering of lazy tiles on low-precision GPUs
TiledTextureUtils::DrawAsTiledImageRect(
        ...,
        rCtx->priv().caps()->shaderCaps()->fHasLowFragmentPrecision);
```

Good:

```cpp
// Disables GPU rendering of lazy tiles when fragment precision is low
TiledTextureUtils::DrawAsTiledImageRect(
        ...,
        !rCtx->priv().caps()->shaderCaps()->fHasLowFragmentPrecision);
```
