---
name: skia/api/name-span-parameters-with-plural-nouns
description: When declaring or defining functions accepting SkSpan or collection parameters, name them with plural nouns and ensure identifiers match @param docs
rubric: readability
severity: nit
---

**Rule.** Name SkSpan and collection parameters using plural nouns (e.g., unis,
glyphs, points) rather than singular nouns, reflecting that they represent
multiple elements and avoiding confusion with single-element overloads. In
addition, ensure parameter identifiers match between declarations and
definitions, and align with accompanying @param doc comments.

**Why.** Spans and collection types represent sequences of elements rather than
individual items. Using singular parameter names creates ambiguity at call
sites, clashes with single-element helper overloads (such as unicharToGlyph),
and produces misleading documentation when parameter names diverge from their
@param tags or between header declarations and source definitions.

## Examples

Bad:

```cpp
// Bad: Singular parameter name for a span, and mismatch with @param tag
/**
 * @param unis span of UTF32 chars
 * @param glyphs output glyph IDs
 */
void SkTypeface::unicharsToGlyphs(SkSpan<const SkUnichar> uni, SkSpan<SkGlyphID> glyphs) const;
```

Good:

```cpp
// Good: Plural parameter name matching span collection semantics and @param tag
/**
 * @param unis span of UTF32 chars
 * @param glyphs output glyph IDs
 */
void SkTypeface::unicharsToGlyphs(SkSpan<const SkUnichar> unis, SkSpan<SkGlyphID> glyphs) const;
```
