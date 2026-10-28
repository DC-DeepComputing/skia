---
name: skia/reuse-in-scope-accessor-variable
description: When repeated subexpressions, accessor results, member aliases, or adjusted overrides are bound to variables, reuse them consistently across all branches
rubric: correctness
severity: should_fix
---

**Rule.** When complex expressions, coordinate differences (such as p1 - p0),
arithmetic calculations, member accesses, compound boolean checks, or container
lookups are evaluated repeatedly across branches or calls, factor them upfront
into descriptive local variables rather than duplicating expressions inline.
Furthermore, when a local variable aliases an object member, holds an accessor
result (such as surf->getCanvas()), or stores an adjusted local override of a
member or parameter (such as a sanitized SkSamplingOptions), reuse that local
variable consistently across all operations and conditional branches within
scope instead of repeating member accesses, getter calls, or referencing
unadjusted originals.

**Why.** Factoring repeated subexpressions, coordinate differences, compound
conditions, and container lookups into named local variables eliminates
redundant evaluation, reduces visual clutter, and prevents logic divergence.
Consistently reusing an existing local alias or adjusted override across all
branches prevents subtle bugs where fallback paths accidentally reference
unadjusted member state, eliminates duplicate accessor calls, and clarifies data
flow.

## Examples

Bad:

```cpp
// Mixing local alias or adjusted override with unadjusted member access:
SkSamplingOptions sampling = fSamplingOptions;
if (sampling == kDefaultSampling && is_nearly_integer_translation(fTransform)) {
    sampling = {};
}
if (fills_layer_bounds(fColorFilter.get())) {
    paint.setShader(fImage->asShader(SkTileMode::kDecal, sampling, SkMatrix::I()));
} else {
    fImage->draw(canvas, 0.f, 0.f, fSamplingOptions, &paint); // Bug: bypassed adjusted 'sampling'
}

// Repeating accessor call or re-evaluating subexpression:
auto canvas = surf->getCanvas();
surf->getCanvas()->drawPath(path, paint);
return SkToInt((innerKey.dataSize() >> 2) + extra);

// Repeating compound condition across branches:
if (fUseTextureSamplerPairs && parameter->type().isSampler()) {
    if (!(specializedParams && specializedParams->find(parameter))) { ... }
} else if (specializedParams && specializedParams->find(parameter)) { ... }
```

Good:

```cpp
// Consistently reusing local adjusted override across all branches:
SkSamplingOptions sampling = fSamplingOptions;
if (sampling == kDefaultSampling && is_nearly_integer_translation(fTransform)) {
    sampling = {};
}
if (fills_layer_bounds(fColorFilter.get())) {
    paint.setShader(fImage->asShader(SkTileMode::kDecal, sampling, SkMatrix::I()));
} else {
    fImage->draw(canvas, 0.f, 0.f, sampling, &paint);
}

// Reusing bound local variable and cached subexpression:
auto canvas = surf->getCanvas();
canvas->drawPath(path, paint);
return innerData32Cnt + extra;

// Factoring repeated compound lookup into a named boolean:
bool paramIsSpecialized = specializedParams && specializedParams->find(parameter);
if (fUseTextureSamplerPairs && parameter->type().isSampler()) {
    if (!paramIsSpecialized) { ... }
} else if (paramIsSpecialized) { ... }
```
