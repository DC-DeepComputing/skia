---
name: skia/prefer-skcolor4f-isopaque-helper
description: When checking opacity or clamping alpha on SkColor4f or SkRGBA4f, use isOpaque() or pinAlpha() instead of float comparisons or SkTPin
rubric: readability
severity: should_fix
---

**Rule.** When checking opacity or clamping alpha on SkColor4f or SkRGBA4f
instances, use dedicated member methods instead of open-coding raw float
operations. Call color.isOpaque() (or !color.isOpaque()) rather than comparing
alpha directly against 1.0f (such as color.fA == 1.0f), and call
color.pinAlpha() instead of unpacking RGB channels and clamping with
SkTPin(color.fA, 0.0f, 1.0f).

**Why.** SkColor4f member methods convey intent directly and encapsulate alpha
validation. isOpaque() includes internal debug assertions verifying normalized
[0.0f, 1.0f] ranges while avoiding floating-point comparison noise, and
pinAlpha() eliminates verbose aggregate unpacking and reconstruction when
clamping alpha.

## Examples

Bad:

```cpp
// Check opacity
if (fColor.fA != 1.0f) {
    applyTransparency();
}
// Clamp alpha
SkColor4f srgb = {color.fR, color.fG, color.fB, SkTPin(color.fA, 0.0f, 1.0f)};
```

Good:

```cpp
// Check opacity
if (!fColor.isOpaque()) {
    applyTransparency();
}
// Clamp alpha
SkColor4f srgb = color.pinAlpha();
```
