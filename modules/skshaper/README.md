# SkShaper (`modules/skshaper/`)

## Overview

`modules/skshaper/` is Skia's low-level text shaping module. It converts UTF-8 text strings and run iterators into positioned glyph sequences (`SkGlyphID`, `SkPoint` positions/offsets, and UTF-8 cluster indices) with optional line-wrapping support:

* **Run Itemization**: Partitions UTF-8 text into homogeneous font fallback, bidirectional (BiDi) level, OpenType script, and language runs via `SkShaper::RunIterator` subclasses.
* **Two-Pass Callback Sink**: Streams shaped line and run metrics (`runInfo`, `commitRunInfo`) followed by caller-allocated glyph/position/cluster buffers (`runBuffer`, `commitRunBuffer`) through `SkShaper::RunHandler`.
* **Pluggable Shaping Backends**: Supports full OpenType shaping via HarfBuzz (`namespace SkShapers::HB`), macOS/iOS shaping via Apple CoreText (`namespace SkShapers::CT`), and a dependency-free 1:1 codepoint fallback (`namespace SkShapers::Primitive`).

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`include/`](include/) | Public `SkShaper` interface, `RunIterator` and `RunHandler` classes, backend headers (`SkShaper_harfbuzz.h`, `SkShaper_coretext.h`, `SkShaper_skunicode.h`), and `SkShapers::Factory`. |
| [`src/`](src/) | Core shaper and run-iterator implementations for HarfBuzz, CoreText, `SkUnicode`, and primitive shaping. |
| [`utils/`](utils/) | Inline client factory helpers (`FactoryHelpers.h`) selecting the best compiled-in shaping backend (`SkShapers::BestAvailable`). |
| [`tests/`](tests/) | Unit tests for shaping, run iterators, and cluster mapping. |

## Key Classes and Entrypoints

The primary classes and entrypoints in `modules/skshaper/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Core Interface & Run Handlers** | [`SkShaper.h`](include/SkShaper.h), [`SkShaper.cpp`](src/SkShaper.cpp) | Abstract shaping entrypoint (`SkShaper`), two-pass line/run callback interface (`SkShaper::RunHandler`), `SkTextBlob` builder sink (`SkTextBlobBuilderRunHandler`), and base run iterators (`FontRunIterator`, `BiDiRunIterator`, `ScriptRunIterator`, `LanguageRunIterator`). |
| **HarfBuzz & Unicode Backends** | [`SkShaper_harfbuzz.h`](include/SkShaper_harfbuzz.h), [`SkShaper_harfbuzz.cpp`](src/SkShaper_harfbuzz.cpp), [`SkShaper_skunicode.h`](include/SkShaper_skunicode.h), [`SkShaper_skunicode.cpp`](src/SkShaper_skunicode.cpp) | OpenType shaping with line wrapping (`SkShapers::HB::ShaperDrivenWrapper`, `ShapeThenWrap`), single-run shaping (`ShapeDontWrapOrReorder`), HarfBuzz script itemization (`SkShapers::HB::ScriptRunIterator`), and `SkUnicode`-backed BiDi itemization (`SkShapers::unicode::BidiRunIterator`). |
| **CoreText & Primitive Backends** | [`SkShaper_coretext.h`](include/SkShaper_coretext.h), [`SkShaper_coretext.cpp`](src/SkShaper_coretext.cpp), [`SkShaper_primitive.cpp`](src/SkShaper_primitive.cpp) | Apple CoreText shaper (`SkShapers::CT::CoreText`) and fallback 1:1 glyph mapper (`SkShapers::Primitive::PrimitiveText`). |
| **Backend Factories** | [`SkShaper_factory.h`](include/SkShaper_factory.h), [`SkShaper_factory.cpp`](src/SkShaper_factory.cpp), [`FactoryHelpers.h`](utils/FactoryHelpers.h) | Dependency-injection factory interface (`SkShapers::Factory`) and compile-time helper (`SkShapers::BestAvailable`, `HarfbuzzFactory`, `CoreTextFactory`). |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../skunicode/`](../skunicode/) | Unicode BiDi level and line-break analysis used by HarfBuzz and `SkUnicode` run iterators. |
| [`../skparagraph/`](../skparagraph/) | Multi-line rich paragraph layout engine built on top of `SkShaper`. |
| [`../skottie/`](../skottie/) | Lottie vector animation player using `SkShaper` and `SkShapers::Factory` for dynamic text layers. |
| [`../svg/`](../svg/) | SVG DOM renderer using `SkShaper` to layout `<text>` elements. |
