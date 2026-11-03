# Skia Developer Tools & Utilities (`tools/`)

## Overview

`tools/` contains Skia's developer applications, GPU test context factories, command-line utilities, and shared test infrastructure used by [`../dm/`](../dm/), [`../tests/`](../tests/), [`../gm/`](../gm/), and [`../bench/`](../bench/).

* **Non-Production Scope**: Code in `tools/` is built only for testing, benchmarking, debugging, and developer workflows (`testonly = True` in Bazel; guarded by `skia_enable_tools` in GN) and is never linked into production client builds of Skia.
* **Cross-Backend Test Contexts**: Provides windowing, surface creation, and GPU context factories for both Ganesh (`ganesh/`) and Graphite (`graphite/`) across OpenGL, Vulkan, Metal, Direct3D, and Dawn.
* **Shared Test Primitives**: Houses the global test/GM registry template, resource loaders for `resources/`, deterministic test fonts, CLI flag parsers, and Skia Gold PNG/MD5 hashing utilities.

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`viewer/`](viewer/) | Interactive GUI application (`viewer`) for inspecting GMs, SKPs, SVGs, and Skottie animations across backends. |
| [`window/`](window/) | Cross-platform window and swapchain creation backing `viewer` and interactive tools. |
| [`sk_app/`](sk_app/) | Application loop and window lifecycle framework used by `viewer`. |
| [`skui/`](skui/) | Input event and modifier state abstractions for interactive applications. |
| [`gpu/`](gpu/) | Shared GPU test utilities (`ContextType`, `BackendSurfaceFactory`, `ManagedBackendTexture`, YUV helpers) common to Ganesh and Graphite. |
| [`ganesh/`](ganesh/) | Ganesh `GrContextFactory` and backend `TestContext` implementations (`gl/`, `vk/`, `mtl/`, `d3d/`, `mock/`). |
| [`graphite/`](graphite/) | Graphite `ContextFactory`, `GraphiteTestContext` implementations (`dawn/`, `mtl/`, `vk/`), and [`graphite/precompile/`](graphite/precompile/) helpers. |
| [`flags/`](flags/) | Command-line flag parser (`CommandLineFlags.h`) and shared `--config`, Ganesh, and Graphite flag definitions. |
| [`fonts/`](fonts/) | Portable test `SkFontMgr` and synthesized typefaces (`FontToolUtils.h`) for deterministic text rendering. |
| [`skslc/`](skslc/) | Standalone SkSL compiler that translates `.sksl` files into SPIR-V, Metal, WGSL, GLSL, or `SkRasterPipeline` stages. |
| [`sksl-minify/`](sksl-minify/) | Offline minifier that compiles internal SkSL modules into generated C++ headers under [`../src/sksl/generated/`](../src/sksl/generated/). |
| [`sksltrace/`](sksltrace/) | Utilities for recording and dumping SkSL shader execution traces. |
| [`raster_pipeline/`](raster_pipeline/) | Profiling toolkit and `llvm-mca` cycle-analysis scripts for `SkRasterPipeline` stages. |
| [`debugger/`](debugger/) | Command-recording canvas (`DebugCanvas`, `DrawCommand`) and JSON serialization powering `debugger.skia.org`. |
| [`fiddle/`](fiddle/) | Backend harness and sandboxing utilities for `fiddle.skia.org` code snippets. |
| [`skdiff/`](skdiff/) | Standalone utility (`skdiff`) for generating visual HTML diffs between two image directories. |
| [`skpbench/`](skpbench/) | Low-variance frame-rate benchmarking tool for replaying `.skp` and `.mskp` files. |
| [`calmbench/`](calmbench/) | Multi-process wrapper for running `nanobench` with reduced thermal and CPU variance. |
| [`timer/`](timer/) | High-resolution wall and GPU timers used by benchmarking tools. |
| [`trace/`](trace/) | Event tracing backends (`ChromeTracingTracer`, Perfetto, `SkDebugfTracer`) for profiling. |
| [`skqp/`](skqp/) | Skia Quality Program (Android CTS) test runner and APK generator for validating device GPU drivers. |
| [`perf-canvaskit-puppeteer/`](perf-canvaskit-puppeteer/) | Puppeteer harness for CanvasKit WebAssembly performance testing. |
| [`lottiecap/`](lottiecap/) | Puppeteer utility for capturing Lottie animation frames. |
| [`skp/`](skp/) | SKP page-set playback and inspection utilities. |
| [`svg/`](svg/) | SVG test parsing helpers and reference resources. |
| [`text/`](text/) | GPU text atlas and glyph rendering test utilities. |
| [`doxygen/`](doxygen/) | Doxygen configuration for generating C++ API reference documentation. |
| [`unicode_comparison/`](unicode_comparison/) | Benchmarks comparing Unicode backend implementations. |

## Key Classes and Entrypoints

The primary shared headers and utilities at the root of `tools/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Registration & Resources** | [`Registry.h`](Registry.h), [`Resources.h`](Resources.h), [`ToolUtils.h`](ToolUtils.h) | Linked-list registry template (`sk_tools::Registry`) backing `DEF_TEST`/`DEF_GM`, resource loading from `--resourcePath` (`GetResourceAsData`), and pixel/color-type test helpers (`ToolUtils`). |
| **Decoding, Encoding & Gold Hashing** | [`CodecUtils.h`](CodecUtils.h), [`DecodeUtils.h`](DecodeUtils.h), [`EncodeUtils.h`](EncodeUtils.h), [`HashAndEncode.h`](HashAndEncode.h) | Image decoding/encoding helpers and `HashAndEncode` for writing PNGs and MD5 hashes with Skia Gold metadata. |
| **SKP & MSKP Playback** | [`MSKPPlayer.h`](MSKPPlayer.h), [`SkSharingProc.h`](SkSharingProc.h), [`UrlDataManager.h`](UrlDataManager.h) | Multi-frame `.mskp` playback (`MSKPPlayer`), cross-frame resource deduplication, and data URI management. |
| **Diagnostics & Crash Handling** | [`CrashHandler.h`](CrashHandler.h), [`ProcStats.h`](ProcStats.h), [`Stats.h`](Stats.h) | Stack-trace crash handler (`SetupCrashHandler`), process memory queries (`sk_tools::getMaxResidentSetSizeMB`), and statistical summaries. |
| **Dependency Sync & Header Checks** | [`git-sync-deps`](git-sync-deps), [`check-headers-self-sufficient`](check-headers-self-sufficient) | Synchronizes `DEPS` repositories and CIPD packages (`git-sync-deps`), and verifies header self-sufficiency. |

