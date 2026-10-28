---
name: skia/gm/tests-document-artificial-transforms-to-defeat-glyph-caching
description: When applying artificial font skewing or transforms in tests or GMs to prevent glyph reuse and force cache overflow, document the intent with an explanatory comment
rubric: readability
severity: nit
---

**Rule.** When using non-obvious font modifications or canvas transform
variations (such as incrementally setting SkFont::setSkewX or adjusting matrix
parameters across iterations) in tests or GMs to defeat glyph caching or force
atlas/cache page overflow, document the intent with an explanatory comment.
Clarify that the artificial variation is a deliberate test harness setup
designed to trigger cache allocation paths or multi-page glyph handling rather
than arbitrary styling.

**Why.** Without explanatory comments, non-standard font skewing, scaling, or
matrix shifts appear to be arbitrary visual styling choices. Documenting that
the manipulation deliberately defeats glyph reuse prevents future maintainers
from simplifying or removing the loop variations and inadvertently regressing
cache-overflow test coverage.

## Examples

Bad:

```cpp
for (int i = 1; i <= 6; ++i) {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->translate(SkIntToScalar(10), (i * lineSpacing) - 10);
    font.setSkewX(i * 0.05f);
    canvas->drawSimpleText(dfText, dfLen, SkTextEncoding::kUTF8, 0, 0, font, paint);
}
```

Good:

```cpp
for (int i = 1; i <= 6; ++i) {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->translate(SkIntToScalar(10), (i * lineSpacing) - 10);
    // Skew the font in order to prevent glyph reuse and force the glyph cache page to overflow.
    font.setSkewX(i * 0.05f);
    canvas->drawSimpleText(dfText, dfLen, SkTextEncoding::kUTF8, 0, 0, font, paint);
}
```
