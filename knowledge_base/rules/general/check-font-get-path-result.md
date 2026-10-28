---
name: skia/check-font-get-path-result
description: When extracting glyph outlines with SkFont::getPath or exposing text-to-path APIs, check for null or nullopt and report unconverted glyph counts
rubric: correctness
severity: should_fix
---

**Rule.** When extracting glyph outlines using SkFont::getPath() or
SkFont::getPaths(), check whether the returned std::optional<SkPath> contains a
value or whether the callback path pointer is non-null. Gracefully handle, log,
or document unsupported bitmap or color glyphs (such as emoji) where no vector
path is available. Additionally, when exposing text-to-path or glyph-to-path
extraction APIs (such as Paragraph::getPath), do not silently discard glyphs
that lack vector outlines; return the count or details of unconverted glyphs so
callers can detect omissions and execute appropriate fallbacks.

**Why.** SkFont::getPath() returns std::nullopt and SkFont::getPaths() passes a
nullptr callback argument for glyphs represented by bitmap or color layers (such
as CBDT, SBIX, or COLRv0/v1) rather than vector outlines. Ignoring failures or
silently discarding unconvertible glyphs produces invisible rendering or missing
content without notifying callers, whereas reporting unconverted glyph counts
allows callers to detect incomplete outlines and trigger fallback rendering.

## Examples

Bad:

```cpp
// BAD: Unconditionally dereferencing getPath or assuming an outline always exists
SkPath path = *font.getPath(glyph);
font_glyphs.push_back({id, width * scale, path});

// BAD: Assuming getPaths callback path pointer is non-null
font.getPaths(glyphIDs, [](const SkPath* path, const SkMatrix& mx, void* ctx) {
    static_cast<SkPath*>(ctx)->addPath(*path, mx);
});

// BAD: Silently dropping glyphs that lack path outlines in text-to-path APIs
virtual SkPath getPath(int lineNumber) = 0;
```

Good:

```cpp
// GOOD: Check std::optional and handle or log non-outline glyphs
if (auto result = font.getPath(glyph)) {
    font_glyphs.push_back({id, width * scale, *result});
} else {
    // Non-outline glyphs (e.g. bitmap or color emoji) cannot be converted to paths.
    std::cerr << "Glyph ID " << glyph << " could not be converted to a path, discarding.\n";
}

// GOOD: Verify callback path pointer before dereferencing in getPaths
font.getPaths(glyphIDs, [](const SkPath* path, const SkMatrix& mx, void* ctx) {
    if (path) {
        static_cast<SkPath*>(ctx)->addPath(*path, mx);
    }
});

// GOOD: Return count of unconverted glyphs so callers can detect omissions
virtual int getPath(int lineNumber, SkPath* dest) = 0;
```
