---
name: skia/ports/ensure-scalercontext-drawable-thread-safety
description: When implementing SkScalerContext for drawable-backed glyphs, implement generateDrawable() transformed by strike matrix, and ensure returned SkDrawable state is thread-safe
rubric: correctness
severity: should_fix
---

**Rule.** In SkScalerContext implementations supporting drawable-backed glyphs,
implement generateDrawable() (transforming the drawable by the strike matrix)
rather than returning nullptr and falling back only to rasterized
generateImage(). Additionally, ensure that any scaler context state or scratch
buffer accessed by the returned SkDrawable is thread-safe across concurrent
onDraw() calls by guarding mutable state with a mutex or pre-rendering into an
SkPicture.

**Why.** Skia renders drawable-backed glyphs directly via generateDrawable()
rather than rasterizing through generateImage(), especially at large sizes,
avoiding unnecessary rasterization and improving rendering quality. Furthermore,
SkDrawable::draw() runs outside the sequential SkStrike lock and executes
concurrently across multiple raster threads; accessing un-synchronized scaler
context state during onDraw() causes data races.

## Examples

Bad:

```cpp
// 1. Returning nullptr fallback for drawable glyphs:
sk_sp<SkDrawable> generateDrawable(const SkGlyph& glyph) override {
    // TODO: fallback only to generateImage()
    return nullptr;
}

// 2. Accessing un-synchronized scaler context scratch buffers in SkDrawable:
bool generateYScalePathForGlyphId(uint16_t glyphId, SkPath* path, ...) {
    fontations_ffi::BridgeScalerMetrics scalerMetrics;
    if (!fontations_ffi::get_path_verbs_points(fOutlines, glyphId, ..., fPathVerbs, fPathPoints, scalerMetrics)) {
        return false;
    }
    return true;
}
```

Good:

```cpp
// 1. Returning drawable transformed by strike matrix:
sk_sp<SkDrawable> generateDrawable(const SkGlyph& glyph) override {
    const auto& rec = this->userTF()->fGlyphRecs[glyph.getGlyphID()];
    return rec.fDrawable ? sk_make_sp<DrawableMatrixWrapper>(rec.fDrawable, fMatrix) : nullptr;
}

// 2. Synchronizing scaler context state with a mutex (or pre-recording into SkPicture):
bool generateYScalePathForGlyphId(uint16_t glyphId, SkPath* path, ...) {
    fontations_ffi::BridgeScalerMetrics scalerMetrics;
    SkAutoMutexExclusive l(fPathMutex);
    if (!fontations_ffi::get_path_verbs_points(fOutlines, glyphId, ..., fPathVerbs, fPathPoints, scalerMetrics)) {
        return false;
    }
    return true;
}
```
