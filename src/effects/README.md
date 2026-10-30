# Skia Effects (`src/effects/`)

## Overview & Scope

`src/effects/` implements the concrete visual effects declared in [`include/effects/`](../../include/effects/) (and base interfaces in [`include/core/`](../../include/core/)) that modify geometry, coverage masks, pixel colors, or whole image layers during rendering.

Effects in Skia plug into four distinct stages of the [`SkPaint`](../../include/core/SkPaint.h) and [`SkCanvas`](../../include/core/SkCanvas.h) pipeline:
1. **Path Effects (`SkPathEffect`)**: Modify vector geometry (`SkPath` and `SkStrokeRec`) *before* rasterization or GPU tessellation.
2. **Mask Filters (`SkMaskFilter`)**: Modify 8-bit alpha coverage masks (`SkMask`) *after* scan conversion (or convert to equivalent image filters on GPU).
3. **Color Filters (`src/effects/colorfilters/`)**: Transform shaded RGBA colors `(R, G, B, A) -> (R', G', B', A')` per-pixel *before* blending with the destination surface.
4. **Image Filters (`src/effects/imagefilters/`)**: Evaluate directed acyclic graphs (DAGs) of 2D image processing operations (blurs, drop shadows, lighting, morphology, convolutions) on offscreen layers (`saveLayer` or `SkPaint::getImageFilter()`).

Note: While [`include/effects/`](../../include/README.md) also declares public headers for gradients (`SkGradient.h`), Perlin noise (`SkPerlinNoiseShader.h`), and runtime effects (`SkRuntimeEffect.h`), their shader implementations reside in [`../shaders/`](../shaders/README.md) and [`../core/SkRuntimeEffect.cpp`](../core/README.md).

## Key Abstractions & Entrypoints

### 1. Path Effects (`SkPathEffectBase` Subclasses)
All path effects inherit from [`SkPathEffectBase`](../core/SkPathEffectBase.h) (in `src/core/`) and override `onFilterPath(SkPathBuilder* dst, const SkPath& src, SkStrokeRec*, const SkRect* cullRect, const SkMatrix& ctm)`:
* [`SkDashPathEffect.cpp`](SkDashPathEffect.cpp) / [`SkDashImpl.h`](SkDashImpl.h): Implements `SkDashPathEffect::Make`, breaking paths into dashed intervals (`SkPathMeasure` + `SkDashPath`) and reporting dash info via `onAsADash()` for GPU fast paths.
* [`SkTrimPathEffect.cpp`](SkTrimPathEffect.cpp) / [`SkTrimPE.h`](SkTrimPE.h): Extracts a `[startT, stopT]` normal or inverted segment of a path (`SkTrimPathEffect::Make`).
* [`SkCornerPathEffect.cpp`](SkCornerPathEffect.cpp) & [`SkDiscretePathEffect.cpp`](SkDiscretePathEffect.cpp): Round sharp polygon corners (`SkCornerPathEffect`) or jitter path segments with deterministic pseudo-random offsets (`SkDiscretePathEffect`).
* [`Sk1DPathEffect.cpp`](Sk1DPathEffect.cpp) & [`Sk2DPathEffect.cpp`](Sk2DPathEffect.cpp): Stamp a shape along a path (`SkPath1DPathEffect`) or tile a path across a 2D lattice (`SkPath2DPathEffect`, `SkLine2DPathEffect`).

### 2. Mask Filters & Blenders (`SkMaskFilterBase` Subclasses & `SkBlenders`)
Mask filters inherit from [`SkMaskFilterBase`](../core/SkMaskFilterBase.h) (in `src/core/`, where [`SkBlurMaskFilterImpl.h`](../core/SkBlurMaskFilterImpl.h) also resides):
* [`SkTableMaskFilter.cpp`](SkTableMaskFilter.cpp): Remaps coverage mask alpha bytes through a 256-entry lookup table (gamma, clip, or custom table).
* [`SkShaderMaskFilterImpl.h`](SkShaderMaskFilterImpl.h) / [`SkShaderMaskFilterImpl.cpp`](SkShaderMaskFilterImpl.cpp): Multiplies coverage masks by the alpha channel of an `SkShader` (or converts to an `SkImageFilter` via `asImageFilter()`).
* [`SkEmbossMaskFilter.h`](SkEmbossMaskFilter.h) / [`SkEmbossMask.h`](SkEmbossMask.h): Legacy 3D directional lighting emboss filter on alpha masks.
* [`SkBlenders.cpp`](SkBlenders.cpp): Implements `SkBlenders::Arithmetic` (`k1*src*dst + k2*src + k3*dst + k4`) using an internal SkSL runtime blender from [`SkKnownRuntimeEffects.h`](../core/SkKnownRuntimeEffects.h).

## Subdirectories & Related Modules

### [`colorfilters/`](colorfilters/) — Per-Pixel Color Transformations
Defines [`SkColorFilterBase.h`](colorfilters/SkColorFilterBase.h) (`as_CFB(filter)`), which extends `SkColorFilter` with `appendStages(const SkStageRec&, bool shaderIsOpaque)` for CPU `SkRasterPipeline` execution and `Type` inspection (`SK_ALL_COLOR_FILTERS`) for GPU translation:
* [`SkBlendModeColorFilter.h`](colorfilters/SkBlendModeColorFilter.h): Blends a constant `SkColor4f` with incoming pixel colors using an `SkBlendMode`.
* [`SkMatrixColorFilter.h`](colorfilters/SkMatrixColorFilter.h) (paired with [`SkColorMatrix.cpp`](SkColorMatrix.cpp) and [`SkColorMatrixFilter.cpp`](SkColorMatrixFilter.cpp)): Applies a 4x5 RGBA or HSLA color matrix.
* [`SkTableColorFilter.h`](colorfilters/SkTableColorFilter.h): Applies independent 256-entry per-channel `(A, R, G, B)` lookup tables.
* [`SkComposeColorFilter.h`](colorfilters/SkComposeColorFilter.h): Evaluates an inner `SkColorFilter` followed by an outer `SkColorFilter`.
* [`SkColorSpaceXformColorFilter.h`](colorfilters/SkColorSpaceXformColorFilter.h) & [`SkWorkingFormatColorFilter.h`](colorfilters/SkWorkingFormatColorFilter.h): Convert colors between `SkColorSpace` / `SkAlphaType` formats via `SkColorSpaceXformSteps`.
* [`SkRuntimeColorFilter.h`](colorfilters/SkRuntimeColorFilter.h) & [`SkHighContrastFilter.cpp`](SkHighContrastFilter.cpp): Execute custom SkSL `SkRuntimeEffect` color filters (including `SkLumaColorFilter`, `SkOverdrawColorFilter`, and accessibility high-contrast filters).
* [`SkGaussianColorFilter.h`](colorfilters/SkGaussianColorFilter.h): Applies a Gaussian function to coverage alpha (used for shadow rendering).

### [`imagefilters/`](imagefilters/) — Multi-Pass Layer & DAG Image Filters
Implements all public factories declared in [`include/effects/SkImageFilters.h`](../../include/effects/SkImageFilters.h). Each filter subclasses [`SkImageFilter_Base`](../core/SkImageFilter_Base.h) and operates on `skif::Context` and `skif::FilterResult` (defined in [`../core/SkImageFilterTypes.h`](../core/SkImageFilterTypes.h)) across CPU, Ganesh, and Graphite backends:
* **Blur & Shadows**: [`SkBlurImageFilter.cpp`](imagefilters/SkBlurImageFilter.cpp) (delegates to `skif::Context`'s [`SkBlurEngine`](../core/SkBlurEngine.h)) and [`SkDropShadowImageFilter.cpp`](imagefilters/SkDropShadowImageFilter.cpp).
* **Composition & Color**: [`SkBlendImageFilter.cpp`](imagefilters/SkBlendImageFilter.cpp) (blend modes or `SkBlender`), [`SkMergeImageFilter.cpp`](imagefilters/SkMergeImageFilter.cpp), [`SkComposeImageFilter.cpp`](imagefilters/SkComposeImageFilter.cpp), and [`SkColorFilterImageFilter.cpp`](imagefilters/SkColorFilterImageFilter.cpp).
* **Geometry & Cropping**: [`SkCropImageFilter.cpp`](imagefilters/SkCropImageFilter.cpp) (handles crop rects and tile modes), [`SkMatrixTransformImageFilter.cpp`](imagefilters/SkMatrixTransformImageFilter.cpp), [`SkMagnifierImageFilter.cpp`](imagefilters/SkMagnifierImageFilter.cpp), and [`SkDisplacementMapImageFilter.cpp`](imagefilters/SkDisplacementMapImageFilter.cpp).
* **Kernel & Lighting Operations**: [`SkMatrixConvolutionImageFilter.cpp`](imagefilters/SkMatrixConvolutionImageFilter.cpp), [`SkMorphologyImageFilter.cpp`](imagefilters/SkMorphologyImageFilter.cpp) (dilate/erode), and [`SkLightingImageFilter.cpp`](imagefilters/SkLightingImageFilter.cpp) (point, distant, and spot diffuse/specular lighting).
* **Leaf Sources & Programmable**: [`SkImageImageFilter.cpp`](imagefilters/SkImageImageFilter.cpp), [`SkPictureImageFilter.cpp`](imagefilters/SkPictureImageFilter.cpp), [`SkShaderImageFilter.cpp`](imagefilters/SkShaderImageFilter.cpp), and [`SkRuntimeImageFilter.cpp`](imagefilters/SkRuntimeImageFilter.cpp) (SkSL-backed image filters).

## Build & Test

* **Bazel**: [`BUILD.bazel`](BUILD.bazel), [`colorfilters/BUILD.bazel`](colorfilters/BUILD.bazel), and [`imagefilters/BUILD.bazel`](imagefilters/BUILD.bazel) export `:core_srcs` and `:core_priv_hdrs` into `//src/core:core` and `//src/core:core_priv`.
* **GN**: Exported to `gn/effects.gni` (`skia_effects_sources`, `skia_colorfilters_sources`) and `gn/effects_imagefilters.gni` (`skia_effects_imagefilter_sources`). Run `make -C bazel generate_gni` after updating any `BUILD.bazel` file lists.
* **Testing**: Unit tests live in `tests/` (`DashPathEffectTest.cpp`, `ColorFilterTest.cpp`, `ImageFilterTest.cpp`, `FilterResultTest.cpp`, `HighContrastFilterTest.cpp`) and visual rendering tests in `gm/` (`dashing.cpp`, `colorfilterimagefilter.cpp`, `imagefilters*.cpp`, `lighting.cpp`, `morphology.cpp`).
