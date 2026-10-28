---
name: skia/gpu/reuse-existing-capabilities-over-adhoc-flags
description: When implementing GPU device workarounds or capability-based fallbacks, query root hardware capabilities instead of ad-hoc flags and document the underlying constraint
rubric: readability
severity: should_fix
---

**Rule.** When implementing hardware workarounds or gating GPU features, query
existing capability and shader flags (such as
GrShaderCaps::fHasLowFragmentPrecision or standard feature limits) that model
the underlying technical limitation directly, rather than introducing ad-hoc
capability flags or hardcoding specific renderer checks. Additionally, document
the underlying mathematical, precision, or hardware constraint (such as
intermediate coordinate precision requirements when subsetting pictures into
tiles) necessitating the workaround or fallback to CPU rendering.

**Why.** Hardcoding bespoke capability flags or specific renderer checks
duplicates existing capability definitions, adds maintenance overhead (including
JSON dumping, test overrides, and cross-backend parity), and obscures the true
technical limitation motivating the fallback. Documenting the underlying
technical constraint clarifies why the fallback or workaround is necessary and
prevents maintainers from inadvertently removing it.

## Examples

Bad:

```cpp
// In GrGLCaps.cpp: introduces an ad-hoc flag tied to a specific renderer
fUseSubsettedTileGradients = ctxInfo.renderer() != GrGLRenderer::kMali4xx;

// In GrCaps.h
bool useSubsettedTileGradients() const { return fUseSubsettedTileGradients; }

// At fallback call site: undocumented fallback
if (renderLazyPictureTilesOnGPU &&
    as_IB(image)->type() == SkImage_Base::Type::kLazyPicture) {
    ...
}
```

Good:

```cpp
// Query existing shader capability representing the root constraint:
bool renderLazyPictureTilesOnGPU =
        !rCtx->priv().caps()->shaderCaps()->fHasLowFragmentPrecision;

// At fallback call site: document the intermediate coordinate precision constraint
// If we render a subset of a very large SkPicture into a GPU texture "tile", it'll
// require intermediate coordinates that need high precision. If we don't have that,
// bite the bullet and render the lazy picture tiles on the CPU and upload the data instead.
if (renderLazyPictureTilesOnGPU &&
    as_IB(image)->type() == SkImage_Base::Type::kLazyPicture) {
    ...
}
```
