# SkShaper (`modules/skshaper/`)

## Overview & Scope
`skshaper` is Skia's low-level text shaping module. It converts UTF-8 text strings and font/script/BiDi run iterators into positioned glyph sequences (`SkGlyphID`, `SkPoint` positions/offsets, and UTF-8 cluster indices) with basic greedy line-wrapping support.

It lives in `modules/skshaper/` rather than core `libskia` because complex script shaping relies on external platform or third-party engines—primarily **HarfBuzz** (`//third_party/harfbuzz`), **Apple CoreText**, and [`modules/skunicode`](file:///Users/jmbetancourt/skia/modules/skunicode)—which core Skia avoids depending on directly. `skshaper` serves as the shaping foundation for [`skparagraph`](file:///Users/jmbetancourt/skia/modules/skparagraph/README.md), [`skottie`](file:///Users/jmbetancourt/skia/modules/skottie/README.md), and [`svg`](file:///Users/jmbetancourt/skia/modules/svg).

## Key Abstractions & Entrypoints
Public headers are located in [`include/`](file:///Users/jmbetancourt/skia/modules/skshaper/include):

- **[`SkShaper`](file:///Users/jmbetancourt/skia/modules/skshaper/include/SkShaper.h)**: Abstract interface whose `shape(...)` methods take a UTF-8 buffer, four `RunIterator`s, optional OpenType `SkShaper::Feature` spans (e.g., `liga`, `kern`), layout width / tracking `Options`, and a `RunHandler*` callback sink.
- **[`SkShaper::RunHandler`](file:///Users/jmbetancourt/skia/modules/skshaper/include/SkShaper.h#L197-L248)**: Two-pass per-line callback interface driven by `SkShaper::shape()`:
  1. `beginLine()` $\rightarrow$ `runInfo(const RunInfo&)` (for each visual run) $\rightarrow$ `commitRunInfo()` (allows the client to compute line metrics and baselines).
  2. `runBuffer(const RunInfo&)` (returns a `Buffer` of caller-allocated arrays for `glyphs`, `positions`, `offsets`, and `clusters`) $\rightarrow$ `commitRunBuffer(const RunInfo&)` $\rightarrow$ `commitLine()`.
- **[`SkTextBlobBuilderRunHandler`](file:///Users/jmbetancourt/skia/modules/skshaper/include/SkShaper.h#L304-L330)**: Concrete `RunHandler` helper that accumulates shaped glyph runs directly into an `SkTextBlob` via `makeBlob()`.
- **Run Iterators (`SkShaper::RunIterator` subclasses)**: Partition the input UTF-8 byte stream into homogeneous runs:
  - [`FontRunIterator`](file:///Users/jmbetancourt/skia/modules/skshaper/include/SkShaper.h#L87-L90): `SkShaper::MakeFontMgrRunIterator(...)` (queries an `SkFontMgr` for fallback fonts when the primary `SkFont` lacks glyphs) or `TrivialFontRunIterator`.
  - [`BiDiRunIterator`](file:///Users/jmbetancourt/skia/modules/skshaper/include/SkShaper.h#L91-L95): `SkShapers::unicode::BidiRunIterator(...)` (in [`SkShaper_skunicode.h`](file:///Users/jmbetancourt/skia/modules/skshaper/include/SkShaper_skunicode.h)) or `TrivialBiDiRunIterator`.
  - [`ScriptRunIterator`](file:///Users/jmbetancourt/skia/modules/skshaper/include/SkShaper.h#L96-L100): `SkShapers::HB::ScriptRunIterator(...)` (ISO 15924 four-byte script tags) or `TrivialScriptRunIterator`.
  - [`LanguageRunIterator`](file:///Users/jmbetancourt/skia/modules/skshaper/include/SkShaper.h#L101-L105): `SkShaper::MakeStdLanguageRunIterator(...)` (BCP-47 locale tags) or `TrivialLanguageRunIterator`.
- **[`SkShapers::Factory`](file:///Users/jmbetancourt/skia/modules/skshaper/include/SkShaper_factory.h)** & **[`FactoryHelpers.h`](file:///Users/jmbetancourt/skia/modules/skshaper/utils/FactoryHelpers.h)**: Dependency-injection factory interface (`makeShaper`, `makeBidiRunIterator`, `makeScriptRunIterator`) used by downstream modules like `skottie` to decouple clients from specific shaping backends.

### Backend Implementations
1. **HarfBuzz ([`SkShaper_harfbuzz.h`](file:///Users/jmbetancourt/skia/modules/skshaper/include/SkShaper_harfbuzz.h) / [`SkShaper_harfbuzz.cpp`](file:///Users/jmbetancourt/skia/modules/skshaper/src/SkShaper_harfbuzz.cpp))**: Full OpenType shaping in `namespace SkShapers::HB`:
   - `ShaperDrivenWrapper(unicode, fallback)`: Shapes and wraps lines using HarfBuzz buffer itemization and `SkUnicode` line-break opportunities (re-shaping across break points for accurate kerning/ligatures).
   - `ShapeThenWrap(unicode, fallback)`: Shapes the paragraph once and wraps glyph clusters greedily.
   - `ShapeDontWrapOrReorder(unicode, fallback)`: Single-run shaping without line wrapping or BiDi visual reordering (used by `skparagraph`, which manages its own BiDi ordering and line breaking).
2. **CoreText ([`SkShaper_coretext.h`](file:///Users/jmbetancourt/skia/modules/skshaper/include/SkShaper_coretext.h) / [`SkShaper_coretext.cpp`](file:///Users/jmbetancourt/skia/modules/skshaper/src/SkShaper_coretext.cpp))**: `SkShapers::CT::CoreText(LineBreakMode)` shapes text on macOS/iOS using `CTTypesetter` and `CTLine`.
3. **Primitive Fallback ([`SkShaper.h`](file:///Users/jmbetancourt/skia/modules/skshaper/include/SkShaper.h#L332-L339) / [`SkShaper_primitive.cpp`](file:///Users/jmbetancourt/skia/modules/skshaper/src/SkShaper_primitive.cpp))**: `SkShapers::Primitive::PrimitiveText()` performs naive 1:1 codepoint-to-glyph mapping via `SkFont::textToGlyphs` when neither HarfBuzz nor CoreText is compiled in.

## Subdirectories & Dependencies
- [`include/`](file:///Users/jmbetancourt/skia/modules/skshaper/include): `SkShaper.h`, `SkShaper_harfbuzz.h`, `SkShaper_coretext.h`, `SkShaper_skunicode.h`, `SkShaper_factory.h`.
- [`src/`](file:///Users/jmbetancourt/skia/modules/skshaper/src): Core implementation (`SkShaper.cpp`, `SkShaper_primitive.cpp`, `SkShaper_harfbuzz.cpp`, `SkShaper_coretext.cpp`, `SkShaper_skunicode.cpp`, `SkShaper_factory.cpp`).
- [`utils/`](file:///Users/jmbetancourt/skia/modules/skshaper/utils): `FactoryHelpers.h` convenience factories (`SkShapers::BestAvailable()`).
- [`tests/`](file:///Users/jmbetancourt/skia/modules/skshaper/tests): Unit tests (`ShaperTest.cpp`).
- **Dependencies**: Core Skia (`//:core`), optional [`modules/skunicode`](file:///Users/jmbetancourt/skia/modules/skunicode), `//third_party/harfbuzz`, or Apple `CoreText.framework`.

## Build & Test
- **GN**: Target `//modules/skshaper:skshaper`. Controlled by `skia_enable_skshaper = true`, `skia_use_harfbuzz`, `skia_enable_skunicode`, and `skia_use_fonthost_mac`. Defines `SK_SHAPER_PRIMITIVE_AVAILABLE`, `SK_SHAPER_HARFBUZZ_AVAILABLE`, `SK_SHAPER_CORETEXT_AVAILABLE`, and `SK_SHAPER_UNICODE_AVAILABLE`.
- **Bazel**: Modular targets `//modules/skshaper:skshaper_core`, `:skshaper_harfbuzz`, `:skshaper_unicode`, `:skshaper_coretext`, and `:skshaper_core_and_utils`.
- **Running Tests**:
  ```bash
  ninja -C out/Release dm
  ./out/Release/dm --match Shaper
  ```
