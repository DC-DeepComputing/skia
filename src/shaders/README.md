# Skia Shaders (`src/shaders/`)

## Overview

`src/shaders/` implements Skia's shading pipeline (`SkShader`), which computes source colors in local coordinates across a draw operation:

* **CPU Execution**: Every `SkShaderBase` subclass overrides `appendStages()` to emit SIMD stages into an `SkRasterPipeline`.
* **GPU Execution**: Each shader exposes a `ShaderType` enum (`SkShaderBase::type()`) and accessors so the GPU backends ([`../gpu/ganesh/`](../gpu/ganesh/) and [`../gpu/graphite/`](../gpu/graphite/)) can translate the `SkShader` tree into GPU fragment shaders without embedding GPU dependencies in `src/shaders/`.

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`gradients/`](gradients/) | Linear, radial, sweep, and two-point conical gradient shaders (`SkGradientBaseShader` and geometry subclasses). |

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/shaders/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Base Class & Matrices** | [`SkShaderBase.h`](SkShaderBase.h) | Internal base class (`SkShaderBase` / `as_SB`) defining the `ShaderType` dispatch enum (`SK_ALL_SHADERS`), constant-color optimization, and `SkShaders::MatrixRec` for tracking local matrices across composed shader trees. |
| **Image & Picture Shaders** | [`SkImageShader.h`](SkImageShader.h), [`SkBitmapProcShader.h`](SkBitmapProcShader.h), [`SkPictureShader.h`](SkPictureShader.h) | Image sampling with tile modes and filtering (`SkImageShader`, `SkBitmapProcShader`) and tiled picture rasterization (`SkPictureShader`). |
| **Color & Composition** | [`SkColorShader.h`](SkColorShader.h), [`SkBlendShader.h`](SkBlendShader.h), [`SkColorFilterShader.h`](SkColorFilterShader.h), [`SkWorkingColorSpaceShader.h`](SkWorkingColorSpaceShader.h) | Constant-color shaders and composable wrapper shaders that blend two child shaders, apply a color filter, or evaluate in a working color space. |
| **Coordinate & Vertex Modifiers** | [`SkLocalMatrixShader.h`](SkLocalMatrixShader.h), [`SkCoordClampShader.h`](SkCoordClampShader.h), [`SkTransformShader.h`](SkTransformShader.h), [`SkTriColorShader.h`](SkTriColorShader.h) | Local matrix and coordinate-clamping wrappers, plus barycentric triangle color/coordinate shaders for `drawVertices`. |
| **Programmable, Procedural & HDR** | [`SkRuntimeShader.h`](SkRuntimeShader.h), [`SkPerlinNoiseShaderImpl.h`](SkPerlinNoiseShaderImpl.h), [`SkGainmapShader.cpp`](SkGainmapShader.cpp) | Custom SkSL `SkRuntimeEffect` shaders, fractal Perlin noise/turbulence, and HDR gainmap shaders. |
| **Gradients** | [`SkGradientBaseShader.h`](gradients/SkGradientBaseShader.h), [`SkLinearGradient.h`](gradients/SkLinearGradient.h), [`SkRadialGradient.h`](gradients/SkRadialGradient.h), [`SkSweepGradient.h`](gradients/SkSweepGradient.h), [`SkConicalGradient.h`](gradients/SkConicalGradient.h) | Shared color-stop and color-space interpolation base class (`SkGradientBaseShader`) and concrete gradient geometries in [`gradients/`](gradients/). |
