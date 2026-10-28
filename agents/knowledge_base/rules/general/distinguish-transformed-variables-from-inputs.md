---
name: skia/distinguish-transformed-variables-from-inputs
description: When wrapping or transforming an input parameter, name the local variable to reflect the transformation rather than using a colliding abbreviation or near-homonym
rubric: readability
severity: nit
---

**Rule.** When wrapping, adjusting, filtering, or transforming an input
parameter or existing variable within a function, give the resulting local
variable a distinctive name that explicitly reflects the transformation (such as
'adjText', 'adjustedCoords', or 'transformedBuffer'). Do not name the new
variable a slight abbreviation, truncation, or near-homonym of the parameter
(such as 'txt' alongside 'text' or 'buf' alongside 'buffer') that obscures the
distinction between raw and transformed state in the same scope.

**Why.** Having two variables in the same scope with nearly identical or
abbreviated names obscures which object represents raw versus transformed data.
Distinctive, descriptive names make data flow explicit and prevent accidental
misuse or passing the original unmodified input where the transformed instance
is required.

## Examples

Bad:

```cpp
Shaper::Result Shaper::Shape(const SkString& text, const TextDesc& desc, const SkPoint& point,
                             const sk_sp<SkFontMgr>& fontmgr) {
    // 'txt' is a slight abbreviation of parameter 'text' in the same scope:
    const AdjustedText txt(text, desc);
    return ShapeImpl(txt, desc, SkRect::MakeEmpty().makeOffset(point.x(), point.y()), fontmgr);
}
```

Good:

```cpp
Shaper::Result Shaper::Shape(const SkString& text, const TextDesc& desc, const SkPoint& point,
                             const sk_sp<SkFontMgr>& fontmgr) {
    // Distinctive prefix 'adjText' explicitly communicates the transformation:
    const AdjustedText adjText(text, desc);
    return ShapeImpl(adjText, desc, SkRect::MakeEmpty().makeOffset(point.x(), point.y()), fontmgr);
}
```
