---
name: skia/modules/propagate-textstyle-font-properties
description: When adding font configuration properties to TextStyle in SkParagraph, propagate them across all SkFont construction sites including placeholders, ellipses, and metrics
rubric: correctness
severity: should_fix
---

**Rule.** When adding or modifying font configuration properties on TextStyle in
SkParagraph (such as font edging, hinting, or subpixel rendering), update all
sites where SkFont instances are created from a TextStyle. Propagate the
configuration consistently across normal text runs, inline placeholder shaping
in OneLineShaper, ellipsis generation in TextLine, and font metrics calculation
in TextStyle.

**Why.** In SkParagraph, SkFont instances are constructed independently across
standard text runs, inline placeholders, truncated ellipses, and font metrics
queries. Omitting any construction site leaves those runs rendering with default
font settings, causing visual inconsistencies between body text, placeholders,
and ellipsis marks.

## Examples

Bad:

```cpp
// Bad: SkFont configured from TextStyle for placeholders omits custom rendering properties
sk_sp<SkTypeface> typeface = typefaces.empty() ? nullptr : typefaces.front();
SkFont font(typeface, placeholder.fTextStyle.getFontSize());
// Missing propagation of font edging, hinting, and subpixel settings
```

Good:

```cpp
// Good: propagate all font rendering properties from TextStyle to SkFont
sk_sp<SkTypeface> typeface = typefaces.empty() ? nullptr : typefaces.front();
SkFont font(typeface, placeholder.fTextStyle.getFontSize());
font.setEdging(placeholder.fTextStyle.getFontEdging());
font.setHinting(placeholder.fTextStyle.getFontHinting());
font.setSubpixel(placeholder.fTextStyle.getSubpixel());
```
