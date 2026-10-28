---
name: skia/core/core-do-not-require-sdf-format-for-standard-glyph-mask-atlas
description: When determining atlas acceptance for standard glyph masks, verify atlas fit without restricting the check to SkMask::Format::kSDF_Format
rubric: correctness
severity: should_fix
---

**Rule.** When determining whether a standard glyph mask can be accepted into
the texture atlas (such as in SkGlyphDigest or SkStrike mask digestion), only
verify that the glyph fits via fitsInAtlasDirect(). Do not require
SkMask::Format::kSDF_Format, which applies strictly to distance-field (SDFT)
glyphs.

**Why.** Standard raster glyph masks use formats such as A8 or LCD16. Checking
for kSDF_Format during standard mask digestion causes valid raster glyphs that
fit in the atlas to be erroneously rejected, forcing unnecessary unaccelerated
fallbacks.

## Examples

Bad:

```cpp
if (digestPtr->fitsInAtlasDirect() &&
    digestPtr->maskFormat() == SkMask::Format::kSDF_Format) {
    action = GlyphAction::kAccept;
}
```

Good:

```cpp
if (digestPtr->fitsInAtlasDirect()) {
    action = GlyphAction::kAccept;
}
```
