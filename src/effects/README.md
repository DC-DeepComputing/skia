# Skia Effects (`src/effects/`)

## Overview

`src/effects/` implements visual effects that modify vector geometry, coverage masks, pixel colors, or offscreen image layers during rendering. These effects plug into four stages of the drawing pipeline:

* **Path Effects**: Modify vector geometry (`SkPath` and stroke settings) *before* rasterization or GPU tessellation.
* **Mask Filters**: Modify 8-bit alpha coverage masks *after* scan conversion.
* **Color Filters ([`colorfilters/`](colorfilters/))**: Transform shaded RGBA colors per-pixel *before* blending with the destination surface.
* **Image Filters ([`imagefilters/`](imagefilters/))**: Evaluate directed acyclic graphs (DAGs) of 2D image processing operations (blurs, drop shadows, lighting, morphology, convolutions) on offscreen layers.

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`colorfilters/`](colorfilters/) | Per-pixel color transformations (blend mode, color matrix, lookup table, composition, color-space conversion, and SkSL runtime color filters). |
| [`imagefilters/`](imagefilters/) | Multi-pass layer and DAG image filters (blur, drop shadow, blend, crop, matrix transform, convolution, morphology, lighting, and shader/picture/image sources). |

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/effects/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Path Effects** | [`SkDashPathEffect.cpp`](SkDashPathEffect.cpp), [`SkDashImpl.h`](SkDashImpl.h), [`SkTrimPathEffect.cpp`](SkTrimPathEffect.cpp), [`SkCornerPathEffect.cpp`](SkCornerPathEffect.cpp), [`SkDiscretePathEffect.cpp`](SkDiscretePathEffect.cpp), [`Sk1DPathEffect.cpp`](Sk1DPathEffect.cpp), [`Sk2DPathEffect.cpp`](Sk2DPathEffect.cpp) | Vector path modifiers for dashing (`SkDashImpl`), trimming segments, rounding corners, jittering paths, or stamping/tiling shapes along paths and lattices. |
| **Mask Filters & Blenders** | [`SkTableMaskFilter.cpp`](SkTableMaskFilter.cpp), [`SkShaderMaskFilterImpl.h`](SkShaderMaskFilterImpl.h), [`SkEmbossMaskFilter.h`](SkEmbossMaskFilter.h), [`SkBlenders.cpp`](SkBlenders.cpp) | Coverage mask modifiers (lookup tables, shader alpha, emboss lighting) and arithmetic source/destination blending (`SkBlenders::Arithmetic`). |
| **Color Filters** | [`SkColorFilterBase.h`](colorfilters/SkColorFilterBase.h), [`SkBlendModeColorFilter.h`](colorfilters/SkBlendModeColorFilter.h), [`SkMatrixColorFilter.h`](colorfilters/SkMatrixColorFilter.h), [`SkTableColorFilter.h`](colorfilters/SkTableColorFilter.h), [`SkComposeColorFilter.h`](colorfilters/SkComposeColorFilter.h), [`SkWorkingFormatColorFilter.h`](colorfilters/SkWorkingFormatColorFilter.h), [`SkRuntimeColorFilter.h`](colorfilters/SkRuntimeColorFilter.h) | Internal base class (`SkColorFilterBase` / `as_CFB`) for CPU stage emission and GPU translation, plus concrete per-pixel RGBA filter implementations. |
| **Image Filters** | [`SkBlurImageFilter.cpp`](imagefilters/SkBlurImageFilter.cpp), [`SkDropShadowImageFilter.cpp`](imagefilters/SkDropShadowImageFilter.cpp), [`SkBlendImageFilter.cpp`](imagefilters/SkBlendImageFilter.cpp), [`SkCropImageFilter.cpp`](imagefilters/SkCropImageFilter.cpp), [`SkMatrixTransformImageFilter.cpp`](imagefilters/SkMatrixTransformImageFilter.cpp) | Multi-pass `SkImageFilters` implementations built on the shared `skif` evaluation framework in [`../core/`](../core/). |
