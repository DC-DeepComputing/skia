# Skia 2D Graphics Library

Skia is a complete, cross-platform 2D graphics library for drawing text, geometries, and images.
It serves as the graphics engine for Google Chrome, ChromeOS, Android, Flutter, and many other products.

Official documentation and user guides are hosted at **[https://skia.org](https://skia.org)**.
This root `README.md` serves as an architectural landing page and codebase directory map.

---

## High-Level Architecture

All drawing in Skia flows through a unified front-end API backed by pluggable rendering backends:

```text
                Public Drawing API (include/core/)
                                │
                                ▼
                     Backend Dispatch (src/core/)
      ┌──────────────┬──────────────┬──────────────┬──────────────┐
      ▼              ▼              ▼              ▼              ▼
 CPU Raster     Graphite GPU    Ganesh GPU     Document     Display List
(src/core/)  (src/gpu/graphite) (src/gpu/ganesh) (src/pdf/...) (src/core/)
```

* **Public vs. Private Headers**: Supported public APIs live exclusively under [`include/`](include/) (excluding [`include/private/`](include/private/)).
  Internal headers and implementations live under [`include/private/`](include/private/) and [`src/`](src/).
* **Dual GPU Backends**: Skia maintains two GPU backends sharing common infrastructure in [`src/gpu/`](src/gpu/):
  * **Graphite** ([`src/gpu/graphite/`](src/gpu/graphite/)): Skia's modern GPU backend designed around explicit command recording, pipeline precompilation, and low CPU overhead across Dawn/WebGPU, Metal, and Vulkan.
  * **Ganesh** ([`src/gpu/ganesh/`](src/gpu/ganesh/)): Skia's original GPU backend, designed around older GPU APIs, supporting OpenGL/ES, Vulkan, Metal, and Direct3D 12.
---

## Codebase Directory Map

### Public API

| Directory | Description |
| :--- | :--- |
| [`include/`](include/) | Public C++ API headers and visibility contracts. Note: Most `src/` components outlined in the following tables expose their public C++ API in a matching `include/<component>/` subdirectory (`src/core` and `include/core/`, `src/gpu` and `include/gpu/`, etc.). The following are subdirectories that do not mirror a single `src/`. |
| [`include/config/`](include/config/) | Compile-time configuration template (`SkUserConfig.h`) and build flags. |
| [`include/docs/`](include/docs/) | Public document factories (`SkPDFDocument.h`, `SkXPSDocument.h`, `SkMultiPictureDocument.h`). |
| [`include/cpu/`](include/cpu/) | CPU recorder context interface (`Recorder.h`). |
| [`include/utils/`](include/utils/) | Utility canvas subclasses (`SkNWayCanvas.h`, `SkPaintFilterCanvas.h`, `SkNoDrawCanvas.h`), shadow helpers, 3D camera, and SVG path parsing (implemented in [`src/utils/`](src/utils/)). |
| [`include/android/`](include/android/) | Android framework and NDK `AHardwareBuffer` interop headers (implemented in [`src/android/`](src/android/) and GPU backends). |
| [`include/private/`](include/private/) | Non-public foundational templates, containers, and restricted friend-only APIs (`private/chromium/`, `private/gpu/`). |
| [`include/third_party/`](include/third_party/) | Third-party headers exposed for public backend interop (`third_party/vulkan/`). |

### Core 2D Engine

| Directory | Description |
| :--- | :--- |
| [`src/core/`](src/core/) | Core 2D rendering engine, CPU rasterization, display-list recording, geometry, caches, and SIMD dispatch. |
| [`src/image/`](src/image/) | Internal image and surface hierarchies bridging CPU raster, deferred generators, picture-backed images, and GPU surfaces. |
| [`src/shaders/`](src/shaders/) | Built-in shaders (image, color, blend, runtime, noise, gainmap) and gradients. |
| [`src/effects/`](src/effects/) | Path effects, mask filters, color filters, and multi-pass image filters. |
| [`src/pathops/`](src/pathops/) | Boolean path operations, path simplification, and winding conversion. |
| [`src/sksl/`](src/sksl/) | Skia Shading Language (SkSL) compiler, intermediate representation, and code generators for CPU and GPU backends. |

### GPU Backends

| Directory | Description |
| :--- | :--- |
| [`src/gpu/`](src/gpu/) | Shared GPU utilities, tessellation math, and backend helpers used by both Ganesh and Graphite. |
| [`src/gpu/graphite/`](src/gpu/graphite/) | Graphite GPU backend (Dawn/WebGPU, Metal, and Vulkan). |
| [`src/gpu/ganesh/`](src/gpu/ganesh/) | Ganesh GPU backend (OpenGL/ES, Vulkan, Metal, and Direct3D 12). |

### Platform Ports, Text, and Codecs

| Directory | Description |
| :--- | :--- |
| [`src/ports/`](src/ports/) | Platform abstraction layer for OS font managers, font scalers, file I/O, memory allocation, and logging. |
| [`src/opts/`](src/opts/) | Architecture-specific SIMD specializations (x86 SSSE3/AVX/AVX2/AVX-512, ARM NEON, LoongArch LASX) dispatched by [`src/core/SkOpts.h`](src/core/SkOpts.h). |
| [`src/text/`](src/text/) | Internal text primitives, glyph runs, and shared GPU glyph atlas management. |
| [`src/sfnt/`](src/sfnt/) | OpenType and TrueType (`sfnt`) font table structures and parsing utilities. |
| [`src/codec/`](src/codec/) | Image decoding and format-specific decoders (PNG, JPEG, WebP, GIF, AVIF, JPEG XL, BMP, ICO, RAW). |
| [`src/encode/`](src/encode/) | Image encoders (PNG, JPEG, WebP), ICC profile generation, and gainmap encoding. |
| [`rust/`](rust/) | Memory-safe Rust implementations and Cxx FFI bridges for codecs and metadata parsing (`png`, `bmp`, `exif`, `icc`). |

### Document-Based Rendering

| Directory | Description |
| :--- | :--- |
| [`src/pdf/`](src/pdf/) | PDF document backend (`SkPDFDocument`), object serialization, font subsetting, and tagged PDF structure trees. |
| [`src/svg/`](src/svg/) | SVG rendering backend (`SkSVGCanvas`, `SkSVGDevice`) for recording draw commands into SVG XML streams. |
| [`src/xps/`](src/xps/) | XPS document backend (`SkXPSDocument`, `SkXPSDevice`) for Windows XML Paper Specification output. |

### High-Level Modules

| Directory | Description |
| :--- | :--- |
| [`modules/`](modules/) | Standalone and higher-level modular libraries layered on top of core Skia. |
| [`modules/skparagraph/`](modules/skparagraph/) | Rich multi-line paragraph layout engine (BiDi, line breaking, font fallback, and styling). |
| [`modules/skshaper/`](modules/skshaper/) | Text shaping module wrapping HarfBuzz and CoreText. |
| [`modules/skottie/`](modules/skottie/) | Lottie JSON animation player. |
| [`modules/canvaskit/`](modules/canvaskit/) | WebAssembly and JavaScript bindings exposing Skia to the web. |

### Testing, Tools, Build Systems, and Infrastructure

| Directory | Description |
| :--- | :--- |
| [`_agents/`](_agents/) | Skills and configuration for agent-based development and testing. |
| [`tests/`](tests/) | C++ unit and GPU test suite. |
| [`gm/`](gm/) | Golden Master (GM) visual rendering tests tracked by Skia Gold. |
| [`dm/`](dm/) | Diamond Master (`dm`) test runner executing GMs, unit tests, SKPs, and image decoding across backends. |
| [`tools/`](tools/) | Developer applications (`viewer`, `debugger`), GPU test contexts, benchmarks, and CLI utilities. |
| [`fuzz/`](fuzz/) | Fuzzing harnesses and OSS-Fuzz reproduction targets. |
| [`gn/`](gn/) | GN build configurations, build arguments, and source file lists. |
| [`bazel/`](bazel/) | Bazel build rules, toolchain configurations, and the Bazel-to-GN exporter tool. |
| [`infra/`](infra/) | CI/CD infrastructure, LUCI task drivers, and bot recipes. |

---

## Building and Testing

* **Building**: Skia maintains two build systems, GN and Bazel; see [`gn/`](gn/) and [`bazel/`](bazel/) for more information.
  A common core Bazel build target is `//:core` and the equivalent GN target is `:skia`.
  Core Skia is defined in [`BUILD.bazel`](BUILD.bazel) and exported to `gn/*.gni` via `//bazel/exporter_tool`.
  See [`bazel/`](bazel/) and [`gn/`](gn/) for which `.gni` lists are generated from Bazel vs. manually maintained.
* **Testing**: Testing is supported by both unit tests ([`tests/`](tests/)) run through [`dm/`](dm/) and image tests defined in [`gm/`](gm/).
  Testing information is found in the [`tests/`](tests/), [`gm/`](gm/), and [`dm/`](dm/) subdirectories.

---

## Coding Conventions and Contributing

* **C++ Style Guide and Formatting**: See [`site/docs/dev/contrib/style.md`](site/docs/dev/contrib/style.md) and [`_agents/skills/skia-style-check/SKILL.md`](_agents/skills/skia-style-check/SKILL.md).
* **Submitting Changes**: See [`CONTRIBUTING`](CONTRIBUTING) and [https://skia.org/docs/dev/contrib/submit/](https://skia.org/docs/dev/contrib/submit/) for Gerrit code review workflows.
* **Release Notes**: User-facing API changes or notable bug fixes should include an entry in [`relnotes/`](relnotes/).
