---
name: skia/core/fold-color-filter-into-shader-image-filter
description: When creating an SkImageFilters::Shader from an SkPaint or SkShader with an SkColorFilter, apply the color filter via makeWithColorFilter instead of SkImageFilters::ColorFilter
rubric: readability
severity: should_fix
---

**Rule.** When constructing an SkImageFilters::Shader from an SkPaint or
SkShader that has an SkColorFilter, apply the color filter directly to the
SkShader using shader->makeWithColorFilter(...) before passing it to
SkImageFilters::Shader. Do not construct an intermediate
SkImageFilters::ColorFilter node to wrap the shader filter.

**Why.** Applying the color filter directly onto the SkShader evaluates the
color filter inline during shader sampling in a single rasterization step.
Wrapping an SkImageFilters::Shader inside an SkImageFilters::ColorFilter adds an
unnecessary intermediate node and pass to the image filter DAG.

## Examples

Bad:

```cpp
sk_sp<SkImageFilter> srcShader = SkImageFilters::Shader(paint.refShader());
sk_sp<SkImageFilter> srcColor = SkImageFilters::ColorFilter(paint.refColorFilter(),
                                                            std::move(srcShader));
```

Good:

```cpp
sk_sp<SkShader> shader = paint.refShader();
if (paint.getColorFilter()) {
    shader = shader->makeWithColorFilter(paint.refColorFilter());
}
sk_sp<SkImageFilter> srcColor = SkImageFilters::Shader(
        std::move(shader),
        paint.isDither() ? SkImageFilters::Dither::kYes : SkImageFilters::Dither::kNo);
```
