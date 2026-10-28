---
name: skia/tests/name-test-helpers-after-salient-properties
description: When defining test helper factories, name them with snake_case describing salient configured properties rather than generic PascalCase names like CreateTestPaint
rubric: readability
severity: should_fix
---

**Rule.** When defining test helper factory functions (e.g., creating SkPaint,
SkSurface, SkCanvas, or SkPath), name them in lowercase snake_case describing
the salient properties or configurations they set up (such as
create_aa_hairline_paint() or create_white_raster_surface()). Avoid generic
PascalCase or vague names like CreateTestPaint() or make_test_surface() that
obscure test preconditions at call sites.

**Why.** Generic helper names like CreateTestPaint() obscure fixture state and
test preconditions at call sites, forcing readers to inspect helper definitions
to understand what is being tested. Descriptive names make test cases
self-documenting, and using lowercase snake_case conforms to Skia's style guide
for static non-class helper functions in implementation and test files.

## Examples

Bad:

```cpp
// Vague generic PascalCase helper obscures configured properties at call sites
static inline SkPaint CreateTestPaint() {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    return paint;
}
```

Good:

```cpp
// Explicit snake_case name documents salient properties at call sites
static SkPaint create_aa_hairline_paint() {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    return paint;
}
```
