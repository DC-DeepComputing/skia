# Skia Golden Master Rendering Tests (`gm/`)

## Overview

`gm/` contains Skia's Golden Master (GM) visual rendering tests.

* **Cross-Backend Visual Verification**: Each GM draws a deterministic scene onto an `SkCanvas` across CPU raster, Ganesh, Graphite, PDF, SVG, and SKP backends.
* **Skia Gold Integration**: Rendered images are hashed and uploaded to Skia Gold (`gold.skia.org`) in CI for pixel-by-pixel regression tracking, and can be inspected interactively in `viewer` or batch-rendered with [`../dm/`](../dm/).
* **GMs vs. Unit Tests**: Write a GM in `gm/` when verifying visual rendering output across backends (shaders, blend modes, path rendering, text, image filters). Write a unit test in [`../tests/`](../tests/) when verifying non-visual APIs, math, or internal invariants with `REPORTER_ASSERT`.

## Key Classes and Entrypoints

The primary classes and registration macros in `gm/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Base Classes & Registry** | [`gm.h`](gm.h), [`gm.cpp`](gm.cpp) | `skiagm::GM` base class, `skiagm::GpuGM` Ganesh subclass, `skiagm::DrawResult` (`kOk`, `kFail`, `kSkip`), and `skiagm::GMRegistry` linked-list registration. |
| **Simple Stateless Macros** | [`gm.h`](gm.h) | `DEF_SIMPLE_GM`, `DEF_SIMPLE_GM_BG`, and `DEF_SIMPLE_GM_CAN_FAIL` for self-contained drawing functions that only require a name, canvas dimensions, and `SkCanvas*`. |
| **Ganesh GPU Macros** | [`gm.h`](gm.h) | `DEF_SIMPLE_GPU_GM`, `DEF_SIMPLE_GPU_GM_BG`, and `DEF_SIMPLE_GPU_GM_CAN_FAIL` for tests requiring a `GrRecordingContext*` alongside `SkCanvas*` (automatically skipped on non-Ganesh configs). |
| **Stateful Lifecycle Hooks** | [`gm.h`](gm.h) | Virtual hooks on `skiagm::GM` (registered via `DEF_GM`) for asset initialization (`onOnceBeforeDraw`), per-context GPU setup (`onGpuSetup` / `onGpuTeardown`), animation (`onAnimate`), and interactive input (`onChar`). |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../dm/`](../dm/) | Command-line test runner that executes GMs across backend sinks and writes Skia Gold hashes. |
| [`../tests/`](../tests/) | Non-visual C++ unit and GPU behavioral assertion tests. |
| [`../tools/viewer/`](../tools/viewer/) | Interactive GUI application for live-inspecting, animating, and debugging GMs. |

