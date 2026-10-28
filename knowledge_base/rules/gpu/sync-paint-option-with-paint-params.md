---
name: skia/gpu/sync-paint-option-with-paint-params
description: When modifying or optimizing shader key generation stages in PaintParams or ShadingParams, update counterpart methods in PaintOption to prevent pipeline key divergence
rubric: correctness
severity: should_fix
---

**Rule.** When adding, modifying, or optimizing shader and pipeline key
generation stages in PaintParams or ShadingParams (such as handlePrimitiveColor,
handleClipping, handlePaintAlpha, handleColorFilter, or handleDithering), update
the counterpart helper methods in PaintOption in lockstep. Ensure precompiled
key building reproduces the exact same snippet sequence, fixed blend modes, and
conditional branches emitted during runtime draws.

**Why.** Graphite's precompilation system in PaintOption directly mirrors
runtime pipeline key assembly in PaintParams. Divergence between runtime key
generation and precompile helpers causes precompiled pipeline keys to mismatch
draw-time keys, triggering cache misses and runtime shader compilation jank.

## Examples

Bad:

```cpp
// In PaintParams.cpp / ShadingParams.cpp:
bool ShadingParams::handlePrimitiveColor(const KeyContext& keyContext) const {
    if (fPrimitiveBlendMode == SkBlendMode::kDst) {
        AddFixedBlendMode(keyContext, keyBuilder, gatherer, SkBlendMode::kDst);
    } ...
}

// In PaintOption.cpp: left unchanged using unoptimized blender key generation
bool PaintOption::handlePrimitiveColor(const KeyContext& keyContext) const {
    AddToKey(keyContext, keyBuilder, gatherer, SkBlender::Mode(fPrimitiveBlendMode).get());
}
```

Good:

```cpp
// In PaintOption.cpp: updated in lockstep to mirror ShadingParams logic
bool PaintOption::handlePrimitiveColor(const KeyContext& keyContext) const {
    if (fPrimitiveBlendMode == SkBlendMode::kDst) {
        AddFixedBlendMode(keyContext, keyBuilder, gatherer, SkBlendMode::kDst);
    } else {
        AddToKey(keyContext, keyBuilder, gatherer, SkBlender::Mode(fPrimitiveBlendMode).get());
    }
}
```
