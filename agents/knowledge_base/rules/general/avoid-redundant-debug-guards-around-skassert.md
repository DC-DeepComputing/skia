---
name: skia/avoid-redundant-debug-guards-around-skassert
description: 'When asserting debug invariants, omit redundant guards around standalone SkASSERT, but wrap assertion-only declarations and validation blocks in SkDEBUGCODE instead of [[maybe_unused]] or #if 1'
rubric: readability
severity: should_fix
---

**Rule.** Do not wrap standalone SkASSERT, SkASSERTF, or SkDEBUGFAIL calls in
#ifdef SK_DEBUG, #if defined(SK_DEBUG), or SkDEBUGCODE(...) guards, and evaluate
pure, side-effect-free expressions directly inside SkASSERT rather than
declaring separate debug-only temporaries. Conversely, when debug checks require
assertion-only declarations, enclosing conditional logic, or multi-line
validation blocks that must not run in release builds, wrap them in
SkDEBUGCODE(...) rather than annotating variables with [[maybe_unused]] or
leaving temporary #if 1 directives.

**Why.** SkASSERT and SkDEBUGFAIL expand to no-ops in release builds, making
preprocessor guards around single assertions redundant and pure temporaries
unnecessary clutter. However, [[maybe_unused]] declarations and enclosing
statements still execute at runtime in release builds; wrapping assertion-only
variables and debug blocks in SkDEBUGCODE(...) completely elides them in release
builds without unused-symbol warnings or temporary #if 1 directives.

## Examples

Bad:

```cpp
// Variant 1: redundant #ifdef SK_DEBUG preprocessor guard around assertion
#ifdef SK_DEBUG
SkASSERT(SkRect::Bounds(points).has_value());
#endif

// Variant 2: debug temporary declared solely to feed SkASSERT
SkDEBUGCODE(bool isFinite = origSrcPath.isFinite();)
raw = SkPathPriv::Raw(origSrcPath);
SkASSERT(isFinite == raw.bounds().isFinite());

// Variant 3: temporary #if 1 directive guarding debug-only validation logic
#if 1
if (auto info = dst->fPathRef->isRect()) {
    SkASSERT(SkPathDirectionToFirst(info->fDirection) == dst->getFirstDirection());
}
#endif

// Variant 4: assertion-only declaration silenced with [[maybe_unused]]
[[maybe_unused]] const auto& colorInfo = rpDesc.fColorAttachment;
SkASSERT(colorTexture ? colorInfo.isCompatible(colorTexture->textureInfo())
                      : colorInfo.fFormat == TextureFormat::kUnsupported);
```

Good:

```cpp
// Variant 1: assertion invoked directly without preprocessor guard
SkASSERT(SkRect::Bounds(points).has_value());

// Variant 2: pure expression evaluated directly inside SkASSERT
raw = SkPathPriv::Raw(origSrcPath);
SkASSERT(origSrcPath.isFinite() == raw.bounds().isFinite());

// Variant 3: debug-only validation block wrapped in SkDEBUGCODE
SkDEBUGCODE(
    if (auto info = dst->fPathRef->isRect()) {
        SkASSERT(SkPathDirectionToFirst(info->fDirection) == dst->getFirstDirection());
    }
)

// Variant 4: assertion-only variable elided with SkDEBUGCODE
SkDEBUGCODE(const auto& colorInfo = rpDesc.fColorAttachment;)
SkASSERT(colorTexture ? colorInfo.isCompatible(colorTexture->textureInfo())
                      : colorInfo.fFormat == TextureFormat::kUnsupported);
```
