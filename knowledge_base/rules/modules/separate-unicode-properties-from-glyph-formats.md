---
name: skia/modules/separate-unicode-properties-from-glyph-formats
description: When designing text inspection APIs, keep Unicode character properties like emoji separate from font glyph visual formats like color or bitmap fonts
rubric: testing
severity: should_fix
---

**Rule.** When adding or modifying text and paragraph inspection APIs, do not
conflate Unicode character properties (such as emoji codepoint flags) with font
glyph rendering formats (such as color fonts, bitmaps, or vector paths) in a
single query method or predicate. Expose separate query methods for
character-level semantic properties (e.g., containsEmoji) and glyph-level visual
representations (e.g., containsColorFontOrBitmap).

**Why.** Unicode emoji properties are character-level semantic attributes used
for text shaping and segmentation, whereas glyph visual representations describe
how a font renders glyphs. Conflating them in a single query leads to incorrect
assumptions and bugs, because emoji can be rendered as monochrome vector
outlines and non-emoji characters can be rendered as color or bitmap glyphs.

## Examples

Bad:

```cpp
// modules/skparagraph/include/Paragraph.h
// Conflates Unicode character properties with font glyph representation formats:
virtual bool containsColorFontOrEmoji(SkTextBlob* textBlob) = 0;
```

Good:

```cpp
// modules/skparagraph/include/Paragraph.h
// Separates character-level Unicode properties from glyph visual representation queries:
virtual bool containsEmoji(SkTextBlob* textBlob) = 0;
virtual bool containsColorFontOrBitmap(SkTextBlob* textBlob) = 0;
```
