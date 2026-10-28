---
name: skia/sksl-document-unpremultiplied-color-return-rationale
description: When a shader or color-processing function intentionally departs from convention by returning unpremultiplied colors, document the rationale in comments rather than using terse labels
rubric: readability
severity: nit
---

**Rule.** When a shader, image sampling helper, or color pipeline function
intentionally departs from Skia's convention by returning unpremultiplied colors
(or omitting premultiplication), document the optimization rationale in an
explanatory comment (such as avoiding redundant premul/unpremul roundtrips
before downstream color space transformations). Do not use terse, ambiguous
comments like `// unpremul` that fail to explain why premultiplication is
bypassed.

**Why.** In Skia, shaders and color processing stages are conventionally
expected to return premultiplied colors. When an internal stage intentionally
leaves colors unpremultiplied as an optimization, an unexplained or cryptic
comment leaves maintainers unsure whether the non-standard color representation
is an accidental bug, an incomplete implementation, or a deliberate performance
shortcut.

## Examples

Bad:

```cpp
// unpremul
return sampleColor;
```

Good:

```cpp
// We always return an unpremul color here to skip work when transforming colorspaces
return sampleColor;
```
