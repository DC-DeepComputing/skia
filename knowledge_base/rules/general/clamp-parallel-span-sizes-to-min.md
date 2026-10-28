---
name: skia/clamp-parallel-span-sizes-to-min
description: When processing parallel SkSpans, clamp counts to std::min at caller boundaries, and assert matching lengths with SkASSERT in internal methods
rubric: readability
severity: should_fix
---

**Rule.** When implementing functions that accept paired source and destination
SkSpan parameters (or multiple parallel spans) and copying elements or
delegating to pointer-and-count APIs, clamp the element count to
std::min(src.size(), dst.size()) rather than unconditionally using src.size().
Ensure buffer reads and writes never exceed the capacity of any involved span.
Additionally, in lower-level internal methods where upstream callers already
guarantee and clamp consistent span sizes, assert matching lengths with SkASSERT
before indexing across them rather than adding redundant runtime sanitization.

**Why.** Callers may pass source and destination spans of unequal lengths,
leading to out-of-bounds writes or heap buffer overflows if the count is not
clamped to std::min. In internal methods with established caller contracts,
asserting matching lengths with SkASSERT catches upstream desynchronization in
debug builds without incurring redundant release-build sanitization.

## Examples

Bad:

```cpp
// Caller / boundary API: unconditionally using src.size() overflows dst:
void SkFont::unicharsToGlyphs(SkSpan<const SkUnichar> uni, SkSpan<SkGlyphID> glyphs) const {
    this->getTypeface()->unicharsToGlyphs(uni.data(), uni.size(), glyphs.data());
}

// Internal method: indexing parallel spans without asserting matching lengths:
void SkDraw::drawAtlas(SkSpan<const SkRSXform> xform,
                       SkSpan<const SkRect> textures,
                       SkSpan<const SkColor> colors) {
    for (size_t i = 0; i < xform.size(); ++i) {
        mx.preTranslate(-textures[i].fLeft, -textures[i].fTop);
    }
}
```

Good:

```cpp
// Caller / boundary API: clamp element count to std::min:
void SkFont::unicharsToGlyphs(SkSpan<const SkUnichar> uni, SkSpan<SkGlyphID> glyphs) const {
    const size_t n = std::min(uni.size(), glyphs.size());
    this->getTypeface()->unicharsToGlyphs(uni.data(), n, glyphs.data());
}

// Internal method: assert matching lengths before indexing:
void SkDraw::drawAtlas(SkSpan<const SkRSXform> xform,
                       SkSpan<const SkRect> textures,
                       SkSpan<const SkColor> colors) {
    SkASSERT(xform.size() == textures.size());
    SkASSERT(colors.empty() || (xform.size() == colors.size()));
    for (size_t i = 0; i < xform.size(); ++i) {
        mx.preTranslate(-textures[i].fLeft, -textures[i].fTop);
    }
}
```
