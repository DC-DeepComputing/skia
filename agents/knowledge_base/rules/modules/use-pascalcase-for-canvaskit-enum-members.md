---
name: skia/modules/use-pascalcase-for-canvaskit-enum-members
description: When referencing CanvasKit enum members in JavaScript or TypeScript, use PascalCase property names rather than lowercase or camelCase to avoid passing undefined
rubric: correctness
severity: nit
---

**Rule.** When referencing CanvasKit enum values in JavaScript or TypeScript
(such as CanvasKit.RectWidthStyle, CanvasKit.RectHeightStyle, or
CanvasKit.BlendMode), use PascalCase property names (e.g.,
CanvasKit.RectWidthStyle.Tight). Do not use lowercase or camelCase member names
(such as .tight), as JavaScript property access is case-sensitive and will
evaluate to undefined.

**Why.** CanvasKit enum objects exposed through Emscripten Embind define their
enumerators in PascalCase. Referencing an enum member with lowercase or
camelCase evaluates to undefined in JavaScript, which is passed into WebAssembly
bindings and causes runtime conversion errors or incorrect rendering behavior.

## Examples

Bad:

```cpp
const rects = paragraph.getRectsForRange(
    0, 1, CanvasKit.RectHeightStyle.Tight, CanvasKit.RectWidthStyle.tight);
```

Good:

```cpp
const rects = paragraph.getRectsForRange(
    0, 1, CanvasKit.RectHeightStyle.Tight, CanvasKit.RectWidthStyle.Tight);
```
