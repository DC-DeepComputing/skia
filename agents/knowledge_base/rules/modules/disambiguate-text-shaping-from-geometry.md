---
name: skia/modules/disambiguate-text-shaping-from-geometry
description: When adding or modifying font shaping APIs, headers, or factories in graphics modules, qualify identifiers as TextShaping rather than generic Shaping
rubric: readability
severity: should_fix
---

**Rule.** When defining or integrating font and glyph shaping headers,
namespaces, builder methods, or factory fields in modules where geometric vector
shapes (paths, rects, polylines) are manipulated (such as modules/svg or
modules/skottie), qualify identifiers as 'TextShaping' (or 'TextShaper') rather
than using bare 'Shaping'. Do not use ambiguous names like 'SkSVGShaping' or
'setShapingFactory' that conflate text layout with geometric shape processing.

**Why.** In 2D vector graphics and document formats like SVG and Skottie,
'shape' commonly refers to geometric vector primitives (such as path, rect,
circle, or polygon nodes). Using bare 'Shaping' or 'SkSVGShaping' for font glyph
layout creates domain ambiguity with geometry manipulation, whereas
'TextShaping' explicitly distinguishes font glyph layout from geometric shape
operations.

## Examples

Bad:

```cpp
// modules/svg/include/SkSVGShaping.h
namespace SkSVGShaping {
...
}

// Builder configuration:
Builder& setShapingFactory(sk_sp<SkShapers::Factory> factory);
```

Good:

```cpp
// modules/svg/include/SkSVGTextShaping.h
namespace SkSVGTextShaping {
...
}

// Builder configuration:
Builder& setTextShapingFactory(sk_sp<SkShapers::Factory> factory);
```
