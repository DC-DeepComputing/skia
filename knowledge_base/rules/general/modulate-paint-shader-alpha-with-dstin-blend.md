---
name: skia/modulate-paint-shader-alpha-with-dstin-blend
description: When converting SkPaint shading to an SkShader or SkImageFilter, modulate the shader with paint alpha using SkShaders::Blend and kDstIn instead of composing filters
rubric: correctness
severity: should_fix
---

**Rule.** When converting or emulating SkPaint shading into an SkShader or
SkImageFilter, modulate the paint shader by the paint alpha using
SkShaders::Blend(SkBlendMode::kDstIn, paint.refShader(), paintColorShader) or a
kDstIn color filter. Do not attempt to combine paint color and shader using
SkImageFilters::Compose, as SkImageFilters::Shader ignores its input image
filter.

**Why.** In SkPaint shading semantics, when an SkShader is present, the paint
color's RGB values are ignored while its alpha channel modulates the shader.
SkImageFilters::Shader generates an image solely from its shader and ignores
input image filters, so composing them with SkImageFilters::Compose fails to
modulate the shader.

## Examples

Bad:

```cpp
sk_sp<SkImageFilter> paintColor = SkImageFilters::Shader(SkShaders::Color(paint.getColor()));
sk_sp<SkImageFilter> srcShader = SkImageFilters::Compose(
    SkImageFilters::Shader(paint.refShader()), std::move(paintColor));
```

Good:

```cpp
sk_sp<SkShader> srcShader = SkShaders::Color(paint.getColor4f(), /*cs=*/nullptr);
if (paint.getShader()) {
    srcShader = SkShaders::Blend(SkBlendMode::kDstIn, paint.refShader(), std::move(srcShader));
}
sk_sp<SkImageFilter> srcFilter = SkImageFilters::Shader(std::move(srcShader));
```
