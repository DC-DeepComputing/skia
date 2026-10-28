---
name: skia/effects/accurately-document-specular-lighting-models
description: When documenting lighting filter operations and specular calculations, accurately describe the Blinn-Phong half-vector model (dot(N, H)) rather than classic Phong reflection (dot(R, V))
rubric: readability
severity: nit
---

**Rule.** When documenting specular lighting calculations or image filter
operations (such as SkImageFilters::*LitSpecular), ensure explanatory comments
accurately state the underlying lighting model. Specifically, document
Blinn-Phong lighting based on the surface normal and half-vector dot product
(dot(N, H)) rather than classic Phong reflection (dot(R, V) or dot(R, E)).

**Why.** Skia's specular lighting image filters implement Blinn-Phong lighting
using the half-vector rather than classic Phong reflection. Documenting the
incorrect mathematical formula creates confusion when comparing image filter
output against legacy mask filters or other shading algorithms.

## Examples

Bad:

```cpp
// specular = (pow(dot(R, V), fLight.fSpecular))
sk_sp<SkImageFilter> specular = SkImageFilters::DistantLitSpecular(
        specularDirection, SK_ColorWHITE, surfaceScale, ks, shininess, coverageBlurred);
```

Good:

```cpp
// specular = (pow(dot(N, H), fLight.fSpecular))
sk_sp<SkImageFilter> specular = SkImageFilters::DistantLitSpecular(
        specularDirection, SK_ColorWHITE, surfaceScale, ks, shininess, coverageBlurred);
```
