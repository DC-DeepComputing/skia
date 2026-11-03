# Skia Golden Master Rendering Tests (`gm/`)

## Overview & Scope
`gm/` contains Skia's Golden Master (GM) rendering tests. Each GM draws a deterministic scene onto an `SkCanvas` across multiple rendering backends (CPU raster, Ganesh, Graphite, PDF, SVG, SKP). In CI, the resulting images are hashed and uploaded to **Skia Gold** (`gold.skia.org`) for pixel-by-pixel visual regression tracking, while local developers can inspect and debug them interactively in `viewer` or batch-render them with `dm`.

## Key Concepts & Entrypoints

### When to Write a GM vs. a Unit Test (`tests/`)
* **Write a GM (`gm/`)** when you want to verify **visual rendering output** across backends (e.g., new shaders, blend modes, path rendering, text/font effects, image filters, or visual bug repros) and track pixel diffs in Skia Gold.
* **Write a Unit Test ([`tests/`](../tests/README.md))** when testing non-visual APIs, data structures, math/geometry, return values, error handling, or GPU resource/cache invariants that can be checked with `REPORTER_ASSERT`.

### GM Macros & Classes (`gm/gm.h`)
All GMs include [`gm/gm.h`](gm.h) and register factories into `skiagm::GMRegistry`:

* **Simple Stateless GMs (`DEF_SIMPLE_GM`)**:
  For tests that only need a name, canvas dimensions, and a drawing function:
  * `DEF_SIMPLE_GM(Name, canvas, width, height) { ... }`: Draws onto `canvas` with a default `SK_ColorWHITE` background.
  * `DEF_SIMPLE_GM_BG(Name, canvas, width, height, bgColor) { ... }`: Specifies a custom background color.
  * `DEF_SIMPLE_GM_CAN_FAIL(Name, canvas, errorMsg, width, height) { ... }`: Returns a `skiagm::DrawResult` (`kOk`, `kFail`, or `kSkip`) and populates `SkString* errorMsg` on failure/skip.
* **Ganesh-Specific GMs (`DEF_SIMPLE_GPU_GM`)**:
  * `DEF_SIMPLE_GPU_GM(Name, rContext, canvas, width, height) { ... }` (and `_BG` / `_CAN_FAIL` variants): Passes `GrRecordingContext*` alongside `SkCanvas*`. Automatically skips non-GPU configs.
* **Stateful / Custom GMs (`skiagm::GM` & `skiagm::GpuGM`)**:
  Subclass `skiagm::GM` (and register with `DEF_GM(return new MyGM();)`) when you need lifecycle hooks or interactive controls:
  * `onOnceBeforeDraw()`: Lazy initialization of heavy assets (images, typefaces, paths) once before the first draw.
  * `onGpuSetup(SkCanvas*, SkString*, GraphiteTestContext*)` / `onGpuTeardown()`: Setup and teardown of GPU-specific resources once per backend context.
  * `onDraw(SkCanvas*, SkString* errorMsg)`: Renders the frame and returns `DrawResult::kOk`, `DrawResult::kFail`, or `DrawResult::kSkip`.
  * `onAnimate(double nanos)` / `onChar(SkUnichar)`: Enables animation in `viewer` and keyboard interaction while debugging.
  * `modifyGrContextOptions(GrContextOptions*)` / `modifyGraphiteContextOptions(ContextOptions*)`: Customizes GPU context options for this GM.

## Build Configuration
GM source files are listed in [`gn/gm.gni`](../gn/gm.gni) (`gm_sources`, manually maintained; can be updated via `python3 gn/make_gm_gni.py`) and in [`gm/BUILD.bazel`](BUILD.bazel) (`:gm` and `:gm_subset`).
