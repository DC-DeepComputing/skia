---
name: skia/gpu/accurately-document-blend-mode-hardware-limitations
description: When documenting hardware blending fallbacks or shader emulation for blend modes, accurately describe specification constraints such as SkBlendMode::kPlus clamping rather than misattributing hardware behavior
rubric: readability
severity: should_fix
---

**Rule.** When documenting why hardware blending is bypassed or emulated in
shaders for specific blend modes, accurately explain the blend mode
specification requirements and format constraints. For example, explain that
SkBlendMode::kPlus requires clamping its output to [0, 1] (r = min(s + d, 1)),
which hardware blending cannot guarantee for unclamped floating-point surfaces
(such as F16), rather than claiming that hardware clamping is being avoided.

**Why.** Inaccurate comments misinform maintainers about blend mode math and
hardware capabilities. SkBlendMode::kPlus requires clamping by definition, which
fixed-point surfaces perform automatically on write; shader emulation is
required because unclamped floating-point surfaces do not clamp hardware
additive blending, not because clamping is undesirable.

## Examples

Bad:

```cpp
// Using SkBlendMode::kPlus (we may be drawing to an unsaturated surface (e.g. F16), and we
// don't want to let the hardware clamp the color output in that case).
if (bm == SkBlendMode::kPlus && !TextureFormatAutoClamps(targetFormat)) {
    return false;
}
```

Good:

```cpp
// SkBlendMode::kPlus always clamps its output to [0,1], but we can't rely on hardware
// blending to do that for all texture formats (e.g. F16).
if (bm == SkBlendMode::kPlus && !TextureFormatAutoClamps(targetFormat)) {
    return false;
}
```
