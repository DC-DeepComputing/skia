# Skia Developer Tools & Utilities (`tools/`)

## Overview & Scope
`tools/` contains Skia's developer applications, test harnesses, GPU context factories, command-line utilities, and shared helper libraries used by [`dm/`](../dm/README.md), [`tests/`](../tests/README.md), [`gm/`](../gm/README.md), and `bench/`. Code in `tools/` is for testing, benchmarking, debugging, and build tooling (`testonly = True` in Bazel) and is never linked into production client builds of Skia.

## Key Concepts & Shared Infrastructure
Top-level files in `tools/` provide core test/tool primitives (see [`tools/BUILD.bazel`](BUILD.bazel)):
* **Test & Resource Helpers**: [`Registry.h`](Registry.h) (`sk_tools::Registry` linked-list registration used by `DEF_TEST` and `DEF_GM`), [`Resources.h`](Resources.h) (`GetResourceAsData`, `GetResourceAsImage` for loading files from `resources/` via `--resourcePath`), and [`ToolUtils.h`](ToolUtils.h) (pixel comparisons, checkerboard/gradient helpers, color-type formatting).
* **Codec & Hashing Utilities**: [`CodecUtils.h`](CodecUtils.h), [`DecodeUtils.h`](DecodeUtils.h), [`EncodeUtils.h`](EncodeUtils.h), and [`HashAndEncode.h`](HashAndEncode.h) (MD5 hashing and PNG encoding with Skia Gold metadata).
* **Dependency Management**: [`git-sync-deps`](git-sync-deps) synchronizes `DEPS` third-party repositories and CIPD packages for local builds.

## Directory Map

### Interactive Applications & Debugging
* [`viewer/`](viewer/): Interactive GUI application (`viewer`) for inspecting GMs, SKPs, SVGs, and Skottie animations in real time, toggling backends (Raster, Ganesh, Graphite), viewing shader code, and profiling frame times.
* [`window/`](window/) & [`sk_app/`](sk_app/) & [`skui/`](skui/): Cross-platform windowing, SwapChain/Surface creation, and input event abstractions backing `viewer` across macOS, Linux, Windows, Android, and iOS.
* [`debugger/`](debugger/): Command-recording and JSON serialization utilities (`SkDebugCanvas`, `DrawCommand`) powering the Skia Debugger (`debugger.skia.org`).
* [`fiddle/`](fiddle/): Backend harness and sandboxing utilities for `fiddle.skia.org` code snippets.
* [`skdiff/`](skdiff/): Standalone tool (`skdiff`) for generating visual HTML diffs between two directories of images.

### GPU Test Contexts (`gpu/`, `ganesh/`, `graphite/`)
* [`gpu/`](gpu/): Shared GPU testing abstractions (`ContextType`, `BackendSurfaceFactory`, `BackendTextureImageFactory`, `FenceSync`, `FlushFinishTracker`, `ProtectedUtils`, YUV/mandoline helpers) common to both Ganesh and Graphite.
* [`ganesh/`](ganesh/): Ganesh `GrContextFactory` and backend-specific `TestContext` implementations (`gl/`, `vk/`, `mtl/`, `d3d/`, `dawn/`, `mock/`), plus `GrAtlasTools` and DDL testing helpers. See [`tools/ganesh/gl/interface/README.md`](ganesh/gl/interface/README.md) for GL interface generation.
* [`graphite/`](graphite/): Graphite `ContextFactory` and `GraphiteTestContext` implementations (`dawn/`, `mtl/`, `vk/`), pipeline storage/callback handlers, and [`precompile/`](graphite/precompile/) test utilities.

### Shaders & CPU Pipeline Tools
* [`skslc/`](skslc/): Standalone SkSL compiler that translates `.sksl` files into SPIR-V, Metal, WGSL, GLSL, or SkVM/SkRP stages, and compiles golden test outputs in `tests/sksl/`.
* [`sksl-minify/`](sksl-minify/): Offline minifier that compiles and shrinks internal SkSL modules into generated C++ headers (`src/sksl/generated/`).
* [`sksltrace/`](sksltrace/): Utilities for recording and dumping SkSL shader execution traces.
* [`raster_pipeline/`](raster_pipeline/): Toolkit and `llvm-mca` cycle-analysis scripts for profiling `SkRasterPipeline` SIMD stages. See [`tools/raster_pipeline/README.md`](raster_pipeline/README.md).

### Benchmarking, Profiling & CTS
* [`skpbench/`](skpbench/): Low-variance frame-rate benchmarking tool for replaying `.skp` and `.mskp` files on Android/desktop devices. See [`tools/skpbench/README.md`](skpbench/README.md).
* [`calmbench/`](calmbench/): Multi-process wrapper for running `nanobench` with reduced thermal/CPU variance.
* [`timer/`](timer/) & [`trace/`](trace/): High-resolution wall/GPU timers and event tracing (`ChromeTracingTracer`, Perfetto, `SkDebugfTracer`, `EventTracingPriv.h`).
* [`skqp/`](skqp/): Skia Quality Program—Android CTS test runner and APK generator for validating device OpenGL ES and Vulkan drivers. See [`tools/skqp/README.md`](skqp/README.md).
* [`perf-canvaskit-puppeteer/`](perf-canvaskit-puppeteer/) & [`lottiecap/`](lottiecap/): Puppeteer harnesses for CanvasKit WebAssembly performance testing and Lottie frame capture. See [`tools/perf-canvaskit-puppeteer/README.md`](perf-canvaskit-puppeteer/README.md) and [`tools/lottiecap/README.md`](lottiecap/README.md).

### CLI Flags, Fonts & Format Utilities
* [`flags/`](flags/): Command-line flag parser (`DEFINE_string`, `DEFINE_bool`, `CommandLineFlags::Parse` in `CommandLineFlags.h`) and shared `--config` / GPU / Graphite flag definitions (`CommonFlagsConfig.h`, `CommonFlagsGanesh.h`, `CommonFlagsGraphite.h`).
* [`fonts/`](fonts/): Portable test `SkFontMgr` and synthesized test typefaces (`FontToolUtils.h`, `TestFontMgr`, `TestSVGTypeface`) ensuring deterministic text rendering across OSes.
* [`skp/`](skp/) & [`svg/`](svg/): SKP page-set utilities and SVG test resources (see [`tools/svg/README.md`](svg/README.md)).
* [`doxygen/`](doxygen/) & [`unicode_comparison/`](unicode_comparison/): API doc configuration ([`tools/doxygen/README.md`](doxygen/README.md)) and Unicode implementation benchmarks ([`tools/unicode_comparison/README.md`](unicode_comparison/README.md)).
