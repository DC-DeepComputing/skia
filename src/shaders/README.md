# Skia Shaders (`src/shaders/`)

## Overview & Scope

`src/shaders/` implements Skia's shading pipeline (`SkShader`), which computes source colors at every pixel (in local coordinates) during a draw operation when attached to an [`SkPaint`](../../include/core/SkPaint.h).

This directory bridges the public shader factories in [`include/core/SkShader.h`](../../include/core/SkShader.h), [`include/effects/SkGradient.h`](../../include/effects/SkGradient.h), [`include/effects/SkPerlinNoiseShader.h`](../../include/effects/SkPerlinNoiseShader.h), and [`include/effects/SkRuntimeEffect.h`](../../include/effects/SkRuntimeEffect.h) (plus internal [`include/private/SkGainmapShader.h`](../../include/private/SkGainmapShader.h)) with Skia's CPU and GPU execution backends:
* **CPU Execution (`SkRasterPipeline`)**: Every `SkShaderBase` subclass overrides `appendStages(const SkStageRec&, const SkShaders::MatrixRec&)` to emit SIMD stages into an [`SkRasterPipeline`](../core/SkRasterPipeline.h).
* **GPU Execution (Ganesh & Graphite)**: Each shader exposes a `ShaderType` enum (`SkShaderBase::type()`) and accessor methods so Ganesh (`GrFragmentProcessor`) and Graphite (`KeyContext` / `PaintParamsKeyBuilder`) can translate the `SkShader` tree into GPU fragment shader code without embedding GPU dependencies directly in `src/shaders/`.

## Key Abstractions & Entrypoints

### 1. Base Class & Coordinate Transformation (`SkShaderBase` & `MatrixRec`)
* [`SkShaderBase.h`](SkShaderBase.h) / [`SkShaderBase.cpp`](SkShaderBase.cpp) / [`SkShader.cpp`](SkShader.cpp): Internal base class (`as_SB(shader)`) extending `SkShader`. Defines the `ShaderType` dispatch enum (`SK_ALL_SHADERS`), serialization (`SkFlattenable`), constant-color optimization (`isConstant()`), and `appendRootStages()` / `appendStages()` for CPU rasterization.
* `SkShaders::MatrixRec` (in [`SkShaderBase.h`](SkShaderBase.h)): Tracks the CTM and pending local matrices as Skia traverses a tree of composed shaders. Coalesces adjacent matrix transforms and applies the inverse pending matrix into the pipeline (`MatrixRec::apply()`) before evaluating child shaders.

### 2. Core & Composable Shaders
* **Image & Picture Shaders**:
  * [`SkImageShader.h`](SkImageShader.h) / [`SkImageShader.cpp`](SkImageShader.cpp): Samples an `SkImage` with configurable X/Y [`SkTileMode`](../../include/core/SkTileMode.h) (`kClamp`, `kRepeat`, `kMirror`, `kDecal`) and [`SkSamplingOptions`](../../include/core/SkSamplingOptions.h) (nearest, bilinear, mipmapped, or cubic Mitchell/Catmull-Rom resamplers). Delegates specialized CPU bitmap sampling to [`SkBitmapProcShader.h`](SkBitmapProcShader.h) when applicable.
  * [`SkPictureShader.h`](SkPictureShader.h) / [`SkPictureShader.cpp`](SkPictureShader.cpp): Tiles an `SkPicture` by rasterizing it into a resolution-appropriate cached `SkImage` (`SkResourceCache`) based on the current CTM scale and delegating to `SkImageShader`.
* **Color & Blending Shaders**:
  * [`SkColorShader.h`](SkColorShader.h) & [`SkEmptyShader.h`](SkEmptyShader.h): Emit constant `SkColor` / `SkColor4f` (with color-space conversion) or transparent/no-op shading.
  * [`SkBlendShader.h`](SkBlendShader.h): Evaluates two child `SkShader`s (`dst` and `src`) and combines their outputs using an `SkBlendMode`.
  * [`SkColorFilterShader.h`](SkColorFilterShader.h): Wraps a child `SkShader` and passes its output through an `SkColorFilterBase`.
  * [`SkWorkingColorSpaceShader.h`](SkWorkingColorSpaceShader.h): Evaluates a child shader inside an explicit working `SkColorSpace` and alpha type.
* **Coordinate & Vertex Modifiers**:
  * [`SkLocalMatrixShader.h`](SkLocalMatrixShader.h), [`SkCTMShader`](SkLocalMatrixShader.h), & [`SkCoordClampShader.h`](SkCoordClampShader.h): Pre-concatenate a local `SkMatrix`, override the CTM, or clamp sample coordinates to a subset `SkRect` before evaluating a child shader.
  * [`SkTransformShader.h`](SkTransformShader.h) & [`SkTriColorShader.h`](SkTriColorShader.h): Internal shaders used by `SkDraw_vertices.cpp` to interpolate per-vertex colors and texture coordinates across triangles using barycentric matrices.
* **Procedural, Programmable & HDR Shaders**:
  * [`SkRuntimeShader.h`](SkRuntimeShader.h) / [`SkRuntimeShader.cpp`](SkRuntimeShader.cpp): Executes user-supplied SkSL `SkRuntimeEffect` programs with bound uniforms and child shaders/color filters/blenders via `SkRasterPipeline` callbacks (`SkRP`) or GPU fragment processors.
  * [`SkPerlinNoiseShaderImpl.h`](SkPerlinNoiseShaderImpl.h) / [`SkPerlinNoiseShaderImpl.cpp`](SkPerlinNoiseShaderImpl.cpp): Implements fractal Perlin noise and turbulence procedural shaders (`SkShaders::MakeFractalNoise`, `MakeTurbulence`).
  * [`SkGainmapShader.cpp`](SkGainmapShader.cpp): Applies ISO 21496-1 / UltraHDR gainmap images on top of base SDR/HDR images to render into an target HDR headroom.

## Subdirectories & Related Modules

* [`gradients/`](gradients/): Implements all gradient shaders declared in [`include/effects/SkGradient.h`](../../include/effects/SkGradient.h):
  * [`SkGradientBaseShader.h`](gradients/SkGradientBaseShader.h) / [`SkGradientBaseShader.cpp`](gradients/SkGradientBaseShader.cpp): Shared base class handling color stops, interpolation spaces (premuliplied, unpremultiplied, Oklab, LCH, HSL, sRGB), tile modes, and color-stop LUT/stage generation.
  * Concrete gradient geometries: [`SkLinearGradient.h`](gradients/SkLinearGradient.h), [`SkRadialGradient.h`](gradients/SkRadialGradient.h), [`SkSweepGradient.h`](gradients/SkSweepGradient.h), and [`SkConicalGradient.h`](gradients/SkConicalGradient.h) (two-point conical gradients).
* **Related Modules**:
  * [`../core/`](../core/README.md): Hosts `SkRasterPipeline`, `SkArenaAlloc`, and `SkKnownRuntimeEffects`.
  * [`../effects/`](../effects/README.md): Hosts companion `SkColorFilter`, `SkMaskFilter`, `SkPathEffect`, and `SkImageFilter` implementations.
  * [`../gpu/ganesh/`](../gpu/ganesh/README.md) & [`../gpu/graphite/`](../gpu/graphite/README.md): Translate `SkShaderBase` trees into GPU shading pipelines.

## Build & Test

* **Bazel**: [`BUILD.bazel`](BUILD.bazel) and [`gradients/BUILD.bazel`](gradients/BUILD.bazel) export `:core_srcs` and `:core_priv_hdrs`, which are compiled into `//src/core:core` and `//src/core:core_priv`. [`SkPerlinNoiseShaderType.h`](SkPerlinNoiseShaderType.h) is also exported via `:opts_srcs` to `//src/opts`.
* **GN**: Shader sources are exported to `gn/core.gni` (`skia_core_sources`) and gradient sources to `gn/effects.gni` (`skia_effects_sources`). Run `make -C bazel generate_gni` after editing `BUILD.bazel`.
* **Testing**: Unit tests live in `tests/` (`ShaderTest.cpp`, `GradientTest.cpp`, `SkRuntimeEffectTest.cpp`, `ImageFilterShaderTest.cpp`) and visual coverage in `gm/` (`gradients.cpp`, `perlinnoise.cpp`, `pictureshader.cpp`, `runtimefunctions.cpp`).
