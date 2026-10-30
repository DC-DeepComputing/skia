# Skia 2D Graphics Library

Skia is a complete, cross-platform 2D graphics library for drawing text, geometries, and images. It serves as the graphics engine for Google Chrome, ChromeOS, Android, Flutter, and many other products.

Official documentation and user guides are hosted at **[https://skia.org](https://skia.org)**. This root `README.md` serves as an architectural landing page and codebase directory map for both human contributors and AI coding agents.

---

## Table of Contents

1. [High-Level Architecture](#1-high-level-architecture)
2. [Codebase Directory Map & Subsystem READMEs](#2-codebase-directory-map--subsystem-readmes)
   - [Public API (`include/`)](#public-api-include)
   - [Core 2D Engine (`src/`)](#core-2d-engine-src)
   - [GPU Backends & Shading Language (`src/gpu/`, `src/sksl/`)](#gpu-backends--shading-language-srcgpu-srcsksl)
   - [Platform Ports, Text, Codecs, PDF & Rust](#platform-ports-text-codecs-pdf--rust)
   - [High-Level Modules (`modules/`)](#high-level-modules-modules)
   - [Testing, Tools, Build Systems & Infra](#testing-tools-build-systems--infra)
3. [Quickstart: Syncing, Building & Testing](#3-quickstart-syncing-building--testing)
4. [Coding Conventions & Contributing](#4-coding-conventions--contributing)

---

## 1. High-Level Architecture

All drawing in Skia flows through a unified front-end API (`SkCanvas`, `SkPaint`, `SkPath`, `SkFont`, `SkShader`, `SkImageFilter`) backed by pluggable rendering backends (`SkDevice` subclasses):

```text
           Public Drawing API (include/core/)
     SkCanvas / SkPaint / SkPath / SkFont / SkSurface
                           │
                           ▼
             Backend Dispatch (SkDevice)
     ┌──────────────┬──────────────┬──────────────┬──────────────┐
     ▼              ▼              ▼              ▼              ▼
CPU Raster     Graphite GPU    Ganesh GPU    PDF Backend    SkPicture
(src/core/)  (src/gpu/graphite) (src/gpu/ganesh) (src/pdf/)  (Display List)
```

* **Public vs. Private Headers**: Supported public APIs live exclusively under [`include/`](include/README.md) (excluding `include/private/`). Internal headers and implementations live under `include/private/` and `src/`.
* **Dual GPU Backends**: Skia maintains two GPU backends sharing common infrastructure in [`src/gpu/`](src/gpu/README.md) and shader compilation in [`src/sksl/`](src/sksl/README.md):
  * **Graphite** ([`src/gpu/graphite/`](src/gpu/graphite/README.md)): Skia's modern GPU backend designed around explicit command recording (`Recorder` / `Recording`), pipeline precompilation, and low CPU overhead across Dawn/WebGPU, Metal, and Vulkan.
  * **Ganesh** ([`src/gpu/ganesh/`](src/gpu/ganesh/README.md)): Skia's established immediate-mode GPU backend (`GrDirectContext` / `GrRecordingContext`) supporting OpenGL/ES, Vulkan, Metal, and Direct3D 12.

---

## 2. Codebase Directory Map & Subsystem READMEs

### Public API (`include/`)
| Directory | README | Description |
| :--- | :--- | :--- |
| `include/` | [`include/README.md`](include/README.md) | Overview of Skia's public C++ API headers (`core/`, `effects/`, `gpu/`, `codec/`, `encode/`, `ports/`, `sksl/`, `utils/`, `docs/`), visibility contracts, and `include/private/` boundaries. |

### Core 2D Engine (`src/`)
| Directory | README | Description |
| :--- | :--- | :--- |
| `src/core/` | [`src/core/README.md`](src/core/README.md) | Core 2D rendering engine (`SkCanvas`, `SkDevice`, `SkDraw`), CPU scan conversion & blitting (`SkBlitter`, `SkRasterPipeline`), `SkPicture` recording, geometry, caches, and runtime SIMD dispatch (`src/opts/`). |
| `src/image/` | [`src/image/README.md`](src/image/README.md) | Internal `SkImage_Base` and `SkSurface_Base` hierarchies bridging CPU raster, lazy/deferred generators, picture-backed images, and GPU surfaces. |
| `src/shaders/` | [`src/shaders/README.md`](src/shaders/README.md) | `SkShaderBase` implementations (image, color, blend, runtime, Perlin noise, gainmap) and linear/radial/sweep/conical gradients (`src/shaders/gradients/`). |
| `src/effects/` | [`src/effects/README.md`](src/effects/README.md) | Path effects (`SkDashPathEffect`, `SkTrimPathEffect`, etc.), mask filters, color filters (`src/effects/colorfilters/`), and image filters (`src/effects/imagefilters/`). |

### GPU Backends & Shading Language (`src/gpu/`, `src/sksl/`)
| Directory | README | Description |
| :--- | :--- | :--- |
| `src/gpu/` | [`src/gpu/README.md`](src/gpu/README.md) | Shared `skgpu` utilities used by both Ganesh and Graphite (`ResourceKey`, `Blend`, `BlurUtils`, `Swizzle`, `Rectanizer`, `tessellate/`, shared `vk/` and `mtl/` helpers). |
| `src/gpu/graphite/` | [`src/gpu/graphite/README.md`](src/gpu/graphite/README.md) | Graphite GPU backend: `Context`, `Recorder`, `Recording`, `Device`, `DrawPass`, `PaintParamsKey` / `ShaderCodeDictionary`, compute pipelines, and Dawn/Metal/Vulkan backends. |
| `src/gpu/ganesh/` | [`src/gpu/ganesh/README.md`](src/gpu/ganesh/README.md) | Ganesh GPU backend: `GrDirectContext`, `SurfaceDrawContext`, `GrOp` batching, `GrFragmentProcessor` shader stages, and GL/Vulkan/Metal/D3D12 backends. |
| `src/sksl/` | [`src/sksl/README.md`](src/sksl/README.md) | Skia Shading Language (SkSL) compiler pipeline (`Lexer`, `Parser`, `ir/`, `analysis/`, `transform/`), codegen backends (SPIR-V, MSL, WGSL, GLSL, `SkRasterPipeline`), and language reference. |

### Platform Ports, Text, Codecs, PDF & Rust
| Directory | README | Description |
| :--- | :--- | :--- |
| `src/ports/` | [`src/ports/README.md`](src/ports/README.md) | Platform abstraction layer: OS font managers (`SkFontMgr`) and font scalers (CoreText, DirectWrite/GDI, FreeType/FontConfig, Fontations), file I/O, memory allocation, and logging. |
| `src/text/` | [`src/text/README.md`](src/text/README.md) | Internal text primitives (`GlyphRun`, `GlyphRunBuilder`, `Slug`) and shared GPU glyph atlas / subrun rendering (`src/text/gpu/`). |
| `src/codec/` | [`src/codec/README.md`](src/codec/README.md) | Image decoding (`SkCodec`, `SkAndroidCodec`), scanline/animated decoding, HDR gainmaps, and decoders for PNG, JPEG, WebP, GIF, AVIF, JPEG XL, BMP, ICO, and RAW. |
| `src/encode/` | [`src/encode/README.md`](src/encode/README.md) | Image encoders (`SkPngEncoder`, `SkPngRustEncoder`, `SkJpegEncoder`, `SkWebpEncoder`), ICC profile generation, and gainmap encoding. |
| `src/pdf/` | [`src/pdf/README.md`](src/pdf/README.md) | PDF document backend (`SkPDFDocument`, `SkPDFDevice`), PDF object serialization, font subsetting, and tagged/accessible PDF structure trees. |
| `rust/` | [`rust/README.md`](rust/README.md) | Memory-safe Rust components and Cxx FFI bridges (`rust/png/`, `rust/bmp/`, `rust/icc/`). |

### High-Level Modules (`modules/`)
| Directory | README | Description |
| :--- | :--- | :--- |
| `modules/` | [`modules/README.md`](modules/README.md) | Overview and dependency map of all 14 modular libraries layered above core `libskia` (`skcms`, `skunicode`, `sksg`, `skresources`, `svg`, `bentleyottmann`, `pathops`, etc.). |
| `modules/skparagraph/` | [`modules/skparagraph/README.md`](modules/skparagraph/README.md) | Rich multi-line paragraph layout engine (BiDi, line breaking, font fallback, selection, and styling). |
| `modules/skshaper/` | [`modules/skshaper/README.md`](modules/skshaper/README.md) | Text shaping module wrapping HarfBuzz and CoreText (`SkShaper`, `RunHandler`, BiDi/Script/Font run iterators). |
| `modules/skottie/` | [`modules/skottie/README.md`](modules/skottie/README.md) | Lottie JSON animation player built on top of `modules/sksg` and `modules/skresources`. |
| `modules/canvaskit/` | [`modules/canvaskit/README.md`](modules/canvaskit/README.md) | WebAssembly + JavaScript bindings exposing Skia, Skottie, and SkParagraph to the web. |

### Testing, Tools, Build Systems & Infra
| Directory | README | Description |
| :--- | :--- | :--- |
| `tests/` | [`tests/README.md`](tests/README.md) | C++ unit and GPU test suite (`DEF_TEST`, `DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS`, `DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS`). |
| `gm/` | [`gm/README.md`](gm/README.md) | Golden Master (GM) visual rendering tests (`skiagm::GM`, `DEF_SIMPLE_GM`) tracked by Skia Gold. |
| `dm/` | [`dm/README.md`](dm/README.md) | Diamond Master (`dm`) test runner executing the `Src` (GMs, tests, SKPs, images) × `Sink` (CPU, Ganesh, Graphite, PDF) matrix. |
| `tools/` | [`tools/README.md`](tools/README.md) | Developer apps (`viewer`, `debugger`), GPU test contexts (`tools/gpu/`), SkSL CLI tools (`skslc`, `sksl-minify`), and `skpbench`. |
| `fuzz/` | [`fuzz/README.md`](fuzz/README.md) | Fuzzing harnesses and OSS-Fuzz reproduction targets. |
| `gn/` | [`gn/README.md`](gn/README.md) | GN build configurations, `.gni` source lists, and Ninja workflows. |
| `bazel/` | [`bazel/README.md`](bazel/README.md) | Bazel build rules (`skia_rules.bzl`), RBE setup, and the Bazel-to-GN exporter (`make -C bazel generate_gni`). |
| `infra/` | [`infra/README.md`](infra/README.md) | CI/CD infrastructure, LUCI bot task drivers, and recipe definitions ([`infra/bots/README.md`](infra/bots/README.md)). |

---

## 3. Quickstart: Syncing, Building & Testing

### 1. Sync Third-Party Dependencies
```bash
python3 tools/git-sync-deps
```

### 2. Build & Test with GN + Ninja
```bash
# Generate a build directory (e.g., out/Debug or out/Release)
./bin/gn gen out/Debug --args='is_debug=true skia_enable_graphite=true'

# Compile the primary test runner (dm) or interactive viewer
ninja -C out/Debug dm viewer

# Run a specific unit test or GM by substring match
./out/Debug/dm --match MyTestName
```
See [`gn/README.md`](gn/README.md), [`dm/README.md`](dm/README.md), and [`tests/README.md`](tests/README.md) for backend flags (`--config grmtl`, `grvk`, `gl`, `vk`, `mtl`).

### 3. Build & Test with Bazel
```bash
# Build core Skia
bazelisk build //:core

# Run a specific Bazel test target
bazelisk test //tests:core_unit_tests
```
> **Important (Bazel ↔ GN Sync)**: Many `gn/*.gni` file lists are generated from `BUILD.bazel` files. After adding, removing, or renaming source files in `BUILD.bazel`, regenerate the `.gni` files with:
> ```bash
> make -C bazel generate_gni
> ```
See [`bazel/README.md`](bazel/README.md) and [`gn/README.md`](gn/README.md) for details on which `.gni` files are auto-generated vs. manually maintained.

---

## 4. Coding Conventions & Contributing

* **C++ Style Guide**: [`site/docs/dev/contrib/style.md`](site/docs/dev/contrib/style.md) is the source of truth for Skia C++ naming, smart pointers (`sk_sp<T>`, `std::unique_ptr<T>`), and formatting (`git cl format`).
* **Submitting Changes**: See [`CONTRIBUTING`](CONTRIBUTING) and `https://skia.org/docs/dev/contrib/submit/` for Gerrit code review workflows.
* **Release Notes**: User-facing API changes or notable bug fixes should include a release note entry in [`relnotes/`](relnotes/README.md).
