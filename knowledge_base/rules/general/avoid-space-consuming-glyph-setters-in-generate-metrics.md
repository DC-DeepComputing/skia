---
name: skia/avoid-space-consuming-glyph-setters-in-generate-metrics
description: When implementing SkScalerContext, avoid space-consuming setters in generateMetrics, mark non-outline glyphs with neverRequestPath, and assert in generatePath
rubric: readability
severity: should_fix
---

**Rule.** In SkScalerContext implementations, keep generateMetrics()
lightweight: do not call space-consuming glyph setters (like setDrawable() or
setPath()) or eagerly allocate heavy objects. For glyphs lacking outline paths
(such as bitmap, SVG, or drawable glyphs), mark them during generateMetrics() by
setting mx.neverRequestPath = true (or glyph->setPath(alloc, nullptr, false)),
and assert in generatePath() with SkASSERT that non-outline glyphs are never
reached.

**Why.** SkStrike manages memory accounting and fetches drawables, paths, and
images on demand via prepareForDrawable() and prepareForPath(). Eagerly
populating heavy objects during metrics generation bypasses cache accounting,
whereas explicitly marking non-outline glyphs records that no vector path
exists, skipping redundant generatePath() invocations and allowing
generatePath() to assert callers never request missing paths.

## Examples

Bad:

```cpp
// In generateMetrics():
GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc* alloc) override {
    if (rec.isDrawable()) {
        glyph->setDrawable(alloc, rec.fDrawable); // Eager heavy setter bypasses strike cache
    }
    return mx;
}

// In generatePath():
std::optional<GeneratedPath> generatePath(const SkGlyph& glyph) override {
    if (rec.isDrawable()) {
        return std::nullopt; // Silently checking non-outline glyphs that shouldn't reach here
    }
    return rec.fPath;
}
```

Good:

```cpp
// In generateMetrics():
GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc* alloc) override {
    if (rec.isDrawable()) {
        // Mark that no outline path exists; drawables are fetched on demand
        mx.neverRequestPath = true; // or legacy: glyph->setPath(alloc, nullptr, false)
    }
    return mx;
}

// In generatePath():
std::optional<GeneratedPath> generatePath(const SkGlyph& glyph) override {
    SkASSERT(!rec.isDrawable());
    return rec.fPath;
}
```
