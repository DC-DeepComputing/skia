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

### Path Effects, Mask Filters, and Blenders
* [`SkDashPathEffect.cpp`](SkDashPathEffect.cpp) / [`SkDashImpl.h`](SkDashImpl.h): Breaks paths into dashed intervals and reports dash parameters for GPU fast paths.
* [`SkTrimPathEffect.cpp`](SkTrimPathEffect.cpp), [`SkCornerPathEffect.cpp`](SkCornerPathEffect.cpp), [`SkDiscretePathEffect.cpp`](SkDiscretePathEffect.cpp), [`Sk1DPathEffect.cpp`](Sk1DPathEffect.cpp), & [`Sk2DPathEffect.cpp`](Sk2DPathEffect.cpp): Built-in path modifiers for trimming segments, rounding corners, jittering paths, or stamping/tiling shapes.
* [`SkTableMaskFilter.cpp`](SkTableMaskFilter.cpp), [`SkShaderMaskFilterImpl.h`](SkShaderMaskFilterImpl.h), & [`SkEmbossMaskFilter.h`](SkEmbossMaskFilter.h): Coverage mask modifiers.
* [`SkBlenders.cpp`](SkBlenders.cpp): Implements `SkBlenders::Arithmetic` for custom arithmetic source/destination blending.

### Color Filters ([`colorfilters/`](colorfilters/))
* [`SkColorFilterBase.h`](colorfilters/SkColorFilterBase.h): Internal base class (`as_CFB(filter)`) extending `SkColorFilter`. Defines `appendStages()` for CPU execution and type inspection (`SK_ALL_COLOR_FILTERS`) so GPU backends can translate color filters into fragment shader stages.
* Concrete implementations include [`SkBlendModeColorFilter.h`](colorfilters/SkBlendModeColorFilter.h), [`SkMatrixColorFilter.h`](colorfilters/SkMatrixColorFilter.h), [`SkTableColorFilter.h`](colorfilters/SkTableColorFilter.h), [`SkComposeColorFilter.h`](colorfilters/SkComposeColorFilter.h), [`SkWorkingFormatColorFilter.h`](colorfilters/SkWorkingFormatColorFilter.h), and [`SkRuntimeColorFilter.h`](colorfilters/SkRuntimeColorFilter.h).

### Image Filters ([`imagefilters/`](imagefilters/))
* Each file in [`imagefilters/`](imagefilters/) implements a factory from `SkImageFilters` (such as [`SkBlurImageFilter.cpp`](imagefilters/SkBlurImageFilter.cpp), [`SkDropShadowImageFilter.cpp`](imagefilters/SkDropShadowImageFilter.cpp), [`SkBlendImageFilter.cpp`](imagefilters/SkBlendImageFilter.cpp), [`SkCropImageFilter.cpp`](imagefilters/SkCropImageFilter.cpp), and [`SkMatrixTransformImageFilter.cpp`](imagefilters/SkMatrixTransformImageFilter.cpp)) on top of the shared `skif` evaluation framework in [`../core/`](../core/).
