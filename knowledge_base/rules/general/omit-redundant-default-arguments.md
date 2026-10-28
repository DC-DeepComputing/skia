---
name: skia/omit-redundant-default-arguments
description: When invoking functions, constructors, or container resize methods with default parameters or value-initialization, omit explicit trailing arguments duplicating default values
rubric: readability
severity: should_fix
---

**Rule.** Do not pass explicit trailing arguments at function call sites,
constructor invocations, container resize calls (such as std::vector::resize),
or in aggregate braced initializers when they duplicate default parameter values
or standard zero-initialization (such as passing explicit nullptr, false, true,
0, 0x00, or {}). Rely on default arguments and container value-initialization to
keep call sites concise, readable, and idiomatic.

**Why.** Passing arguments that duplicate already-defaulted parameter values or
implicit zero-initialization adds visual clutter and boilerplate without
altering program semantics. Standard container resize methods already
value-initialize newly inserted elements to zero for fundamental and scalar
types.

## Examples

Bad:

```cpp
// Redundantly passing explicit nullptr when the trailing parameter defaults to nullptr
return SkGradientShader::MakeLinear(pts, {colors, positions, this->getTileMode(), nullptr});

// Explicitly passing default boolean flag at call site
canvas->saveLayer(bounds, paint, false);
skgpu::TiledTextureUtils::DrawAsTiledImageRect(canvas, img, src, dst, flags, p, c, /*sharpenMM=*/true, cache, max, /*renderLazyPictureTilesOnGPU=*/true);

// Redundantly passing zero initializer argument to vector resize
fullWidthRow.resize(this->getEncodedRowBytes(), 0x00);
```

Good:

```cpp
// Omitting redundant trailing argument that matches default value
return SkGradientShader::MakeLinear(pts, {colors, positions, this->getTileMode()});

// Relying on default boolean parameter
canvas->saveLayer(bounds, paint);
skgpu::TiledTextureUtils::DrawAsTiledImageRect(canvas, img, src, dst, flags, p, c, /*sharpenMM=*/true, cache, max);

// Relying on resize() default value-initialization to zero-initialize elements
fullWidthRow.resize(this->getEncodedRowBytes());
```
