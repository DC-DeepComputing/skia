---
name: skia/when-declaring-c-methods-in-headers-omit-redundant-parameter
description: When declaring C++ methods in headers, omit redundant parameter names for self-describing types to prevent exceeding 100 columns and maintain consistency
rubric: readability
severity: nit
---

**Rule.** In C++ header declarations (especially virtual overrides), omit
redundant parameter names when the parameter type is self-describing (such as
const SkPaint&, const SkGlyph&, or SkFontTableTag) to keep declarations compact
and avoid exceeding Skia's 100-column line limit without unnecessary multi-line
wrapping. Maintain consistency across sibling method declarations rather than
mixing named and unnamed parameters for the same self-documenting type.

**Why.** When a parameter's type is unambiguous and self-explanatory, repeating
the type as an identifier adds visual noise without conveying semantic value.
Omitting redundant names keeps method declarations concise, enables them to fit
within Skia's 100-column limit on a single line, and ensures consistent styling
across related method signatures.

## Examples

Bad:

```cpp
// Inconsistent naming across sibling declarations:
void drawRect(const SkRect& r, const SkPaint& paint) override;
void drawPoints(SkCanvas::PointMode, SkSpan<const SkPoint>, const SkPaint&) override;

// Redundant parameter name forces declaration to exceed 100 columns:
size_t onGetTableData(SkFontTableTag tag, size_t offset, size_t length, void* data) const override;
SkScalerContext::GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc* alloc) override;
```

Good:

```cpp
// Consistently omit redundant parameter names for self-describing types:
void drawRect(const SkRect& r, const SkPaint&) override;
void drawPoints(SkCanvas::PointMode, SkSpan<const SkPoint>, const SkPaint&) override;

// Omitting self-evident parameter names keeps declarations within 100 columns:
size_t onGetTableData(SkFontTableTag, size_t offset, size_t length, void* data) const override;
SkScalerContext::GlyphMetrics generateMetrics(const SkGlyph&, SkArenaAlloc* alloc) override;
```
