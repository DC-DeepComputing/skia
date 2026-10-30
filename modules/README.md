# Skia High-Level Modules (`modules/`)

## Overview & Scope
The `modules/` directory contains semi-independent, higher-level feature libraries layered on top of core `libskia` (`include/core`, `src/core`). Unlike core Skia—which maintains a strict, minimal dependency footprint and focused 2D drawing primitives—modules encapsulate complex domain features (such as rich text layout, vector animations, SVG DOM rendering, and WebAssembly bindings) that often require optional third-party libraries (e.g., HarfBuzz, ICU, Expat) or standalone scene-graph abstractions.

Each module is organized with its own `include/`, `src/`, `tests/`, `gm/`, and `BUILD.gn` / `BUILD.bazel` definitions, enforcing clear layering boundaries above `//:core`.

## Key Abstractions & Entrypoints

### Module Directory Table

| Module | Purpose & Key Entrypoints | Internal & External Dependencies | Documentation |
| :--- | :--- | :--- | :--- |
| [`skcms`](file:///Users/jmbetancourt/skia/modules/skcms) | Standalone C11 ICC color profile parser and color space transform engine (`skcms_ICCProfile`, `skcms_Transform`). | None (zero-dependency; linked directly by core Skia). | [README.chromium](file:///Users/jmbetancourt/skia/modules/skcms/README.chromium) |
| [`skunicode`](file:///Users/jmbetancourt/skia/modules/skunicode) | Unicode text segmentation, BiDi reordering, and grapheme/word/line breaking (`SkUnicode`, `SkBidiIterator`). | `//:core`, plus ICU, ICU4X, libgrapheme, or Apple/Client ICU backends. | — |
| [`skshaper`](file:///Users/jmbetancourt/skia/modules/skshaper) | Low-level text shaping converting UTF-8 runs into positioned glyphs (`SkShaper`, `SkShaper::RunHandler`). | `//:core`, `skunicode` (optional), HarfBuzz / CoreText. | [README.md](file:///Users/jmbetancourt/skia/modules/skshaper/README.md) |
| [`skparagraph`](file:///Users/jmbetancourt/skia/modules/skparagraph) | Rich multi-line paragraph layout, styling, hit-testing, and painting (`ParagraphBuilder`, `Paragraph`, `FontCollection`). | `//:core`, `skshaper`, `skunicode`. | [README.md](file:///Users/jmbetancourt/skia/modules/skparagraph/README.md) |
| [`sksg`](file:///Users/jmbetancourt/skia/modules/sksg) | Retained-mode 2D scene graph with dirty-region invalidation (`sksg::Scene`, `sksg::RenderNode`, `sksg::InvalidationController`). | `//:core`, `//:pathops`. | — |
| [`skresources`](file:///Users/jmbetancourt/skia/modules/skresources) | Asset loading interfaces for external images, audio, and fonts (`skresources::ResourceProvider`, `ImageAsset`). | `//:core`, optional `experimental/ffmpeg`. | — |
| [`jsonreader`](file:///Users/jmbetancourt/skia/modules/jsonreader) | Fast, zero-copy DOM-style JSON parser (`skjson::DOM`, `skjson::Value`). | `//:core`. | — |
| [`skottie`](file:///Users/jmbetancourt/skia/modules/skottie) | Lottie (Bodymovin) JSON vector animation player (`skottie::Animation`, `Animation::Builder`, `SlotManager`). | `//:core`, `sksg`, `skresources`, `jsonreader`, `skshaper`, `skunicode`. | [README.md](file:///Users/jmbetancourt/skia/modules/skottie/README.md) |
| [`svg`](file:///Users/jmbetancourt/skia/modules/svg) | SVG 1.1 DOM parser and renderer (`SkSVGDOM`, `SkSVGSVG`). | `//:core`, `skresources`, `skshaper`, `skunicode`, Expat (`skia_use_expat`). | — |
| [`canvaskit`](file:///Users/jmbetancourt/skia/modules/canvaskit) | Emscripten/WebAssembly & JavaScript bindings exposing Skia, `skottie`, and `skparagraph` to the web. | `//:core`, `skottie`, `skparagraph`, `skshaper`, `skunicode`, `svg`. | [README.md](file:///Users/jmbetancourt/skia/modules/canvaskit/README.md) |
| [`bentleyottmann`](file:///Users/jmbetancourt/skia/modules/bentleyottmann) | Bentley-Ottmann sweep-line algorithm for finding polygon segment intersections (`bentleyottmann::find_intersections`). | `//:core`. | — |
| [`pathops`](file:///Users/jmbetancourt/skia/modules/pathops) | Legacy GN placeholder (`pathops.gni`) for client migration; actual PathOps implementation lives in `src/pathops`. | — | — |
| [`audioplayer`](file:///Users/jmbetancourt/skia/modules/audioplayer) | Cross-platform audio playback helper (`SkAudioPlayer`) using AVFoundation (macOS), Oboe (Android), or SFML (Linux). | `//:core`, platform audio frameworks. | — |
| [`skplaintexteditor`](file:///Users/jmbetancourt/skia/modules/skplaintexteditor) | Experimental plain-text editor widget and demo app (`SkPlainTextEditor::Editor`). | `//:core`, `skshaper`, `skunicode`. | [README.md](file:///Users/jmbetancourt/skia/modules/skplaintexteditor/README.md) |

## Subdirectories & Dependencies
Most modules follow a standard internal layout:
- `include/`: Public C++ headers exported to Skia embedders and downstream modules.
- `src/`: Private implementation files, internal adapters, and helper classes.
- `tests/` & `gm/`: Unit tests and Golden Master rendering tests compiled into Skia's `dm` test runner.
- `utils/`, `bench/`, `fuzz/`, `slides/`: Module-specific CLI tools, microbenchmarks, fuzzers, and Viewer slides.

**Key Dependency Chains**:
- **Text Stack**: [`skparagraph`](file:///Users/jmbetancourt/skia/modules/skparagraph/README.md) $\rightarrow$ [`skshaper`](file:///Users/jmbetancourt/skia/modules/skshaper/README.md) + [`skunicode`](file:///Users/jmbetancourt/skia/modules/skunicode) $\rightarrow$ `//:core` (+ HarfBuzz / ICU).
- **Animation & Vector Stack**: [`skottie`](file:///Users/jmbetancourt/skia/modules/skottie/README.md) $\rightarrow$ [`sksg`](file:///Users/jmbetancourt/skia/modules/sksg) + [`skresources`](file:///Users/jmbetancourt/skia/modules/skresources) + [`jsonreader`](file:///Users/jmbetancourt/skia/modules/jsonreader) + [`skshaper`](file:///Users/jmbetancourt/skia/modules/skshaper/README.md) + [`skunicode`](file:///Users/jmbetancourt/skia/modules/skunicode) $\rightarrow$ `//:core`.
- **Web Stack**: [`canvaskit`](file:///Users/jmbetancourt/skia/modules/canvaskit/README.md) bundles core Skia alongside `skparagraph`, `skottie`, `skshaper`, `skunicode`, and `svg` into a unified WASM binary.

## Build & Test
In **GN** (`BUILD.gn`), modules are controlled via feature flags in `gn/skia.gni` or module `BUILD.gn` files:
- `skia_enable_skottie`, `skia_enable_skparagraph`, `skia_enable_skshaper`, `skia_enable_skunicode`, `skia_enable_svg`, `skia_enable_bentleyottmann`
- External library toggles: `skia_use_harfbuzz`, `skia_use_icu`, `skia_use_libgrapheme`, `skia_use_icu4x`, `skia_use_expat`

In **Bazel** (`BUILD.bazel`), modules expose granular targets with `layering_check` enabled (e.g., `//modules/skottie:skottie`, `//modules/skparagraph:skparagraph_harfbuzz_skunicode`, `//modules/skshaper:skshaper_harfbuzz`).

To build and run module unit tests and GMs via `dm`:
```bash
ninja -C out/Release dm
./out/Release/dm --match Paragraph Skottie Shaper SG SVG
```
