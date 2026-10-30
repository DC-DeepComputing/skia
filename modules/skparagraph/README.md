# SkParagraph (`modules/skparagraph/`)

## Overview & Scope
`skparagraph` (`namespace skia::textlayout`) is Skia's rich, multi-line paragraph layout and rendering engine, originally designed to power Flutter's text stack and CanvasKit. It lives in `modules/skparagraph/` because multi-styled paragraph layout requires heavy external text dependencies—specifically [`skunicode`](file:///Users/jmbetancourt/skia/modules/skunicode) (ICU / libgrapheme / ICU4X) for Unicode segmentation and [`skshaper`](file:///Users/jmbetancourt/skia/modules/skshaper/README.md) (HarfBuzz) for OpenType glyph shaping—that lie outside core `libskia`'s scope.

`skparagraph` handles bidirectional (BiDi) text, multi-script font fallback, line breaking, ellipsis truncation, inline widget placeholders, text decorations (underline, overline, wavy/dotted line-through), text shadows, strut metrics, and rich hit-testing/editing queries.

## Key Abstractions & Entrypoints
All public headers reside in [`include/`](file:///Users/jmbetancourt/skia/modules/skparagraph/include) under `namespace skia::textlayout`:

- **[`ParagraphBuilder`](file:///Users/jmbetancourt/skia/modules/skparagraph/include/ParagraphBuilder.h)** ([`ParagraphBuilderImpl`](file:///Users/jmbetancourt/skia/modules/skparagraph/src/ParagraphBuilderImpl.h)): Constructed via `ParagraphBuilder::make(style, fontCollection, unicode)`. Maintains a stack of `TextStyle`s (`pushStyle()`, `pop()`), accepts UTF-8 or UTF-16 text (`addText()`) and inline `PlaceholderStyle` blocks (`addPlaceholder()`), and produces a `std::unique_ptr<Paragraph>` via `Build()`.
- **[`Paragraph`](file:///Users/jmbetancourt/skia/modules/skparagraph/include/Paragraph.h)** ([`ParagraphImpl`](file:///Users/jmbetancourt/skia/modules/skparagraph/src/ParagraphImpl.h)): The primary layout and rendering object. Key methods:
  - `layout(SkScalar width)`: Computes shaped runs, line breaks, and intrinsic widths (`getMaxIntrinsicWidth()`, `getMinIntrinsicWidth()`, `getHeight()`, `getAlphabeticBaseline()`).
  - `paint(SkCanvas*, x, y)` / `paint(ParagraphPainter*, x, y)`: Draws laid-out lines, backgrounds, shadows, and decorations onto an `SkCanvas` or custom [`ParagraphPainter`](file:///Users/jmbetancourt/skia/modules/skparagraph/include/ParagraphPainter.h).
  - Query & Editing APIs: `getRectsForRange()`, `getRectsForPlaceholders()`, `getGlyphPositionAtCoordinate()`, `getWordBoundary()`, `getLineMetrics()`, `getGlyphClusterAt()`, and `visit()`.
- **[`ParagraphStyle`](file:///Users/jmbetancourt/skia/modules/skparagraph/include/ParagraphStyle.h) & [`StrutStyle`](file:///Users/jmbetancourt/skia/modules/skparagraph/include/ParagraphStyle.h#L21-L76)**: Configures paragraph-wide alignment (`TextAlign`), direction (`TextDirection`), `maxLines`, ellipsis string, soft hyphenation, and minimum line height struts.
- **[`TextStyle`](file:///Users/jmbetancourt/skia/modules/skparagraph/include/TextStyle.h)**: Configures span-level font families, `SkFontStyle`, font size, OpenType `FontFeature`s, `FontArguments` (variable font axes), foreground/background `SkPaint` or `PaintID`, `Decoration`, `TextShadow`, letter/word spacing, and locale.
- **[`FontCollection`](file:///Users/jmbetancourt/skia/modules/skparagraph/include/FontCollection.h) & [`TypefaceFontProvider`](file:///Users/jmbetancourt/skia/modules/skparagraph/include/TypefaceFontProvider.h)**: Manages ordered `SkFontMgr` instances (asset, dynamic, test, default fallback), resolves font family lists and codepoint fallbacks, and owns the [`ParagraphCache`](file:///Users/jmbetancourt/skia/modules/skparagraph/include/ParagraphCache.h).
- **[`ParagraphCache`](file:///Users/jmbetancourt/skia/modules/skparagraph/include/ParagraphCache.h)**: Thread-safe LRU cache storing shaped `Run`s across `ParagraphImpl` instances to bypass redundant HarfBuzz shaping when text and styles match.

### Layout Pipeline
1. **Unicode Analysis**: `ParagraphImpl` uses `SkUnicode` to compute UTF-8/UTF-16 index mappings, BiDi embedding levels, grapheme clusters, word boundaries, and soft/hard line break opportunities.
2. **Shaping ([`OneLineShaper`](file:///Users/jmbetancourt/skia/modules/skparagraph/src/OneLineShaper.h))**: Iterates over style blocks and BiDi/script runs, resolving typefaces via `FontCollection` (falling back per unresolved cluster) and invoking `SkShaper` to produce [`Run`](file:///Users/jmbetancourt/skia/modules/skparagraph/src/Run.h) objects (cached in `ParagraphCache`).
3. **Wrapping ([`TextWrapper`](file:///Users/jmbetancourt/skia/modules/skparagraph/src/TextWrapper.h))**: Breaks shaped `Run`s into [`TextLine`](file:///Users/jmbetancourt/skia/modules/skparagraph/src/TextLine.h) instances according to the target width, applying `maxLines`, ellipsis shaping, and `StrutStyle` vertical metrics.
4. **Painting ([`ParagraphPainterImpl`](file:///Users/jmbetancourt/skia/modules/skparagraph/src/ParagraphPainterImpl.h) & [`Decorations`](file:///Users/jmbetancourt/skia/modules/skparagraph/src/Decorations.h))**: Each `TextLine` paints backgrounds, text shadows, `SkTextBlob` glyph runs, and underline/strikethrough decorations in visual BiDi order.

## Subdirectories & Dependencies
- [`include/`](file:///Users/jmbetancourt/skia/modules/skparagraph/include): Public API headers (`Paragraph.h`, `ParagraphBuilder.h`, `ParagraphStyle.h`, `TextStyle.h`, `FontCollection.h`, `ParagraphCache.h`, `ParagraphPainter.h`, `Metrics.h`, `DartTypes.h`).
- [`src/`](file:///Users/jmbetancourt/skia/modules/skparagraph/src): Implementation (`ParagraphImpl.*`, `OneLineShaper.*`, `TextWrapper.*`, `TextLine.*`, `Run.*`, `Decorations.*`).
- [`gm/`](file:///Users/jmbetancourt/skia/modules/skparagraph/gm), [`tests/`](file:///Users/jmbetancourt/skia/modules/skparagraph/tests), [`bench/`](file:///Users/jmbetancourt/skia/modules/skparagraph/bench), [`slides/`](file:///Users/jmbetancourt/skia/modules/skparagraph/slides), [`utils/`](file:///Users/jmbetancourt/skia/modules/skparagraph/utils): GMs, unit tests (`SkParagraphTest.cpp`), benchmarks, Viewer slides, and `TestFontCollection`.
- **Dependencies**: Core Skia (`//:core`), [`modules/skshaper`](file:///Users/jmbetancourt/skia/modules/skshaper/README.md), and [`modules/skunicode`](file:///Users/jmbetancourt/skia/modules/skunicode).

## Build & Test
- **GN**: Target `//modules/skparagraph:skparagraph` (defines `SK_ENABLE_PARAGRAPH`). Enabled when `skia_enable_skparagraph = true`, `skia_enable_skshaper = true`, `skia_enable_skunicode = true`, and `skia_use_harfbuzz = true` (plus a Unicode backend such as `skia_use_icu = true`). Additional targets: `:tests`, `:gm`, `:bench`, `:utils`.
- **Bazel**: Target `//modules/skparagraph:skparagraph_harfbuzz_skunicode`.
- **Running Tests & GMs**:
  ```bash
  ninja -C out/Release dm
  ./out/Release/dm --match SkParagraph Paragraph
  ```
