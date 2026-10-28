---
name: skia/docs/document-bitmap-fonts-and-glyph-caching
description: When documenting Skia text rendering architecture, describe font fallback in SkFontMgr and note bitmap font support and SkStrikeCache for SkTypeface
rubric: readability
severity: should_fix
---

**Rule.** When documenting Skia's text rendering pipeline or font management
architecture, describe SkFontMgr's font fallback role (resolving typefaces for
missing glyphs/characters using font collections and ordering heuristics) in
addition to family matching and font loading. Additionally, note that SkTypeface
supports non-vector (bitmap) font formats such as color emoji, and clarify that
vector glyphs are rasterized into a glyph mask cache (SkStrikeCache) rather than
treated as on-the-fly SkPaths.

**Why.** Omitting font fallback misrepresents SkFontMgr's role in
character-level font resolution, while assuming all glyphs are vector paths
ignores bitmap font formats and hides SkStrikeCache, Skia's primary caching
mechanism for performant text rasterization.

## Examples

Bad:

```cpp
- **`SkFontMgr`**: Discovers and manages system fonts, matching family names or loading font data.
- **`SkTypeface`**: Represents a font face containing vector paths for each glyph.
...
When drawing text, the backend looks up glyphs from `SkTypeface` and renders each glyph as a standard `SkPath`.
```

Good:

```cpp
- **`SkFontMgr`**: Discovers and manages system fonts. Matches family names, loads font data, and handles fallback to find glyphs from font collections and ordering heuristics.
- **`SkTypeface`**: Represents a font face. Typically contains vector paths but also supports bitmap glyphs. Vector glyphs are rasterized and stored in a glyph cache (`SkStrikeCache`).
```
