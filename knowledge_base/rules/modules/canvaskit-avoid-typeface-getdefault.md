---
name: skia/modules/canvaskit-avoid-typeface-getdefault
description: In CanvasKit examples and applications, avoid CanvasKit.Typeface.GetDefault() and instantiate typefaces explicitly from font data or FontMgr to support builds without embedded fonts
rubric: readability
severity: should_fix
---

**Rule.** In CanvasKit applications, examples, and tests, do not rely on
CanvasKit.Typeface.GetDefault() to create fonts or text components. Explicitly
instantiate typefaces from font data using
CanvasKit.Typeface.MakeTypefaceFromData() or load them via a CanvasKit.FontMgr
(e.g., FontMgr.FromData).

**Why.** CanvasKit.Typeface.GetDefault() only returns a valid typeface when
CanvasKit is built with an embedded font (CK_EMBED_FONT), which is omitted in
lean and custom production builds. Relying on GetDefault() causes it to return
null in those configurations, breaking font creation and text rendering.

## Examples

Bad:

```cpp
const textFont = new CanvasKit.Font(CanvasKit.Typeface.GetDefault(), 20);
```

Good:

```cpp
const roboto = CanvasKit.Typeface.MakeTypefaceFromData(fontData);
const textFont = new CanvasKit.Font(roboto, 20);
```
