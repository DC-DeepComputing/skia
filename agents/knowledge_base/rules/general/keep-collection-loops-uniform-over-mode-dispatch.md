---
name: skia/keep-collection-loops-uniform-over-mode-dispatch
description: When conditional branches or capability checks execute identical loop structures differing only in inner processing, keep the loop uniform and parameterize the call
rubric: design
severity: should_fix
---

**Rule.** Do not branch around identical loop structures across conditional
branches, capability checks, or feature flags when only an inner function call,
parameter, or element processing differs. Keep the iteration loop uniform and
factor the variation into the inner callable (or delegate mode dispatch to the
per-element handler), pass the condition into the call, or select the callable
upfront before the loop.

**Why.** Duplicating loop iteration logic across conditional branches creates
redundant traversal boilerplate and maintenance hazards if loop indexing,
bounds, or element iteration changes. Unifying the loop isolates mode-specific
behavior to the call site or element handler and keeps control flow concise.

## Examples

Bad:

```cpp
// Bad: Branching around loops when per-element handlers encapsulate mode dispatch
if (fDevice->caps()->useDrawListLayer()) {
    for (auto& e : fElements.items()) {
        e.drawClip(fDevice);
    }
} else {
    for (auto& e : fElements.items()) {
        e.resetClipDraw();
    }
}

// Bad: Duplicating identical loop structure across conditional branches
if (experimentalContrast) {
    for (U8CPU i = 0; i < (1 << MAX_LUM_BITS); ++i) {
        U8CPU lum = sk_t_scale255<MAX_LUM_BITS>(i);
        SkTMaskGamma_build_correcting_lut_experimental_contrast(fGammaTables[i], lum, contrast);
    }
} else {
    for (U8CPU i = 0; i < (1 << MAX_LUM_BITS); ++i) {
        U8CPU lum = sk_t_scale255<MAX_LUM_BITS>(i);
        SkTMaskGamma_build_correcting_lut(fGammaTables[i], lum, contrast);
    }
}
```

Good:

```cpp
// Good: Keep iteration loop uniform and delegate mode handling to element method
for (auto& e : fElements.items()) {
    e.drawClip(fDevice);
}

// Good: Keep single shared loop and pass variation into the inner call or select callable upfront
for (U8CPU i = 0; i < (1 << MAX_LUM_BITS); ++i) {
    U8CPU lum = sk_t_scale255<MAX_LUM_BITS>(i);
    SkTMaskGamma_build_correcting_lut(fGammaTables[i], lum, contrast, experimentalContrast);
}
```
