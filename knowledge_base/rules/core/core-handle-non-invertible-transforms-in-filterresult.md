---
name: skia/core/core-handle-non-invertible-transforms-in-filterresult
description: When applying layer-space transformations to image filters, rely on FilterResult::applyTransform to handle non-invertible matrices centrally rather than checking invertibility in individual filters
rubric: design
severity: should_fix
---

**Rule.** When applying layer-space transforms to image filters, handle
non-invertible matrices centrally inside skif::FilterResult::applyTransform by
returning an empty FilterResult. Do not add ad-hoc matrix invertibility checks
(such as transform.invert()) in individual SkImageFilter subclasses or caller
call sites.

**Why.** Individual image filter factories validate that their input transforms
are invertible, but layer-space matrix concatenation can still produce
non-invertible matrices under extreme floating-point inputs. Handling
non-invertible transforms centrally in FilterResult::applyTransform protects all
filter implementations uniformly without duplicating ad-hoc bailout boilerplate
across subclasses.

## Examples

Bad:

```cpp
// In SkImageFilter subclass onFilterImage():
skif::LayerSpace<SkMatrix> transform = context.mapping().paramToLayer(fTransform);
if (!transform.invert(nullptr)) {
    return {};
}
return childOutput.applyTransform(context, transform, fSampling);
```

Good:

```cpp
// In SkImageFilter subclass onFilterImage():
skif::LayerSpace<SkMatrix> transform = context.mapping().paramToLayer(fTransform);
return childOutput.applyTransform(context, transform, fSampling);

// Handled centrally in FilterResult::applyTransform (SkImageFilterTypes.cpp):
if (!transform.invert(nullptr)) {
    return {};
}
```
