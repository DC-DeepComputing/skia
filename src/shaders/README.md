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

### Base Class and Coordinate Transforms
* [`SkShaderBase.h`](SkShaderBase.h): Internal base class (`as_SB(shader)`) extending `SkShader`. Defines the `ShaderType` dispatch enum (`SK_ALL_SHADERS`), constant-color optimization, and `SkShaders::MatrixRec` for tracking and coalescing local matrices across composed shader trees.

### Core and Composable Shaders
* **Image & Picture Shaders**: [`SkImageShader.h`](SkImageShader.h) (samples an `SkImage` with tile modes and sampling options, delegating specialized CPU bitmap sampling to [`SkBitmapProcShader.h`](SkBitmapProcShader.h)) and [`SkPictureShader.h`](SkPictureShader.h) (tiles a cached rasterization of an `SkPicture`).
* **Color & Composition Shaders**: [`SkColorShader.h`](SkColorShader.h), [`SkBlendShader.h`](SkBlendShader.h), [`SkColorFilterShader.h`](SkColorFilterShader.h), and [`SkWorkingColorSpaceShader.h`](SkWorkingColorSpaceShader.h).
* **Coordinate & Vertex Shaders**: [`SkLocalMatrixShader.h`](SkLocalMatrixShader.h), [`SkCoordClampShader.h`](SkCoordClampShader.h), [`SkTransformShader.h`](SkTransformShader.h), and [`SkTriColorShader.h`](SkTriColorShader.h).
* **Programmable, Procedural & HDR Shaders**: [`SkRuntimeShader.h`](SkRuntimeShader.h) (executes custom SkSL `SkRuntimeEffect` shaders), [`SkPerlinNoiseShaderImpl.h`](SkPerlinNoiseShaderImpl.h) (fractal noise and turbulence), and [`SkGainmapShader.cpp`](SkGainmapShader.cpp) (HDR gainmap application).

### Gradients ([`gradients/`](gradients/))
* [`SkGradientBaseShader.h`](gradients/SkGradientBaseShader.h): Shared base class handling color stops, interpolation color spaces, tile modes, and color-stop lookup tables.
* Concrete geometries: [`SkLinearGradient.h`](gradients/SkLinearGradient.h), [`SkRadialGradient.h`](gradients/SkRadialGradient.h), [`SkSweepGradient.h`](gradients/SkSweepGradient.h), and [`SkConicalGradient.h`](gradients/SkConicalGradient.h).
