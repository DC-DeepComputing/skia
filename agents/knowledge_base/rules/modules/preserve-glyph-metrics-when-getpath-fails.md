---
name: skia/modules/preserve-glyph-metrics-when-getpath-fails
description: When extracting glyph outlines with SkFont::getPath, do not return early or discard the glyph on failure; record empty paths to preserve layout metrics
rubric: correctness
severity: should_fix
---

**Rule.** When collecting or exporting glyph outline data using SkFont::getPath,
do not return early or discard the glyph if outline extraction fails or returns
std::nullopt. Continue extracting glyph metrics (such as advance width via
SkFont::getWidths) and record the glyph with an empty SkPath.

**Why.** Glyphs without vector outlines (such as whitespace characters, control
codes, or unsupported bitmap formats) still possess essential layout metrics
like advance width. Discarding or returning early on outline failure omits the
glyph from exported font tables, causing broken text layout spacing and missing
glyph lookups downstream.

## Examples

Bad:

```cpp
SkPath path;
if (auto result = font.getPath(glyph)) {
    path = *result;
} else {
    // Returning early or discarding drops advance widths for whitespace characters.
    return;
}

float width;
font.getWidths(&glyph, 1, &width);
font_glyphs.push_back({glyph, width * scale, std::move(path)});
```

Good:

```cpp
// Retain glyph and advance width even if outline extraction returns empty.
SkPath path = font.getPath(glyph).value_or(SkPath());

float width;
font.getWidths(&glyph, 1, &width);
font_glyphs.push_back({glyph, width * scale, std::move(path)});
```
