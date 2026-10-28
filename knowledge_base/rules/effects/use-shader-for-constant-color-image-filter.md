---
name: skia/effects/use-shader-for-constant-color-image-filter
description: When introducing a constant solid color or procedural layer in an SkImageFilter graph, use SkImageFilters::Shader instead of filtering the source graphic with SkImageFilters::ColorFilter
rubric: correctness
severity: should_fix
---

**Rule.** When introducing a constant solid color or procedural layer into an
SkImageFilter graph, create the node using
SkImageFilters::Shader(SkShaders::Color(...)). Do not use
SkImageFilters::ColorFilter(..., nullptr) to generate a constant color, as a
null input filter binds to the DAG's source graphic and preserves its geometry
and alpha coverage instead of producing a uniform color layer.

**Why.** In SkImageFilter DAG semantics, passing nullptr as the input filter
implicitly binds the node to the DAG's source graphic (such as an input bitmap
or coverage mask). Filtering the source graphic transforms its pixel colors but
retains its coverage shape and bounds, corrupting subsequent blend steps that
expect an independent, uniform layer.

## Examples

Bad:

```cpp
// Passing nullptr binds to the source graphic, carrying source coverage.
sk_sp<SkColorFilter> ambientCF = SkColorFilters::TableARGB(alphaTable, rgbTable, rgbTable, rgbTable);
sk_sp<SkImageFilter> ambient = SkImageFilters::ColorFilter(ambientCF, nullptr);
```

Good:

```cpp
// Introducing an independent solid color layer via SkImageFilters::Shader.
SkColor4f ambientColor = {ambientf, ambientf, ambientf, 1.0f};
sk_sp<SkImageFilter> ambient = SkImageFilters::Shader(SkShaders::Color(ambientColor, nullptr));
```
