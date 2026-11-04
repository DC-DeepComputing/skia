# SkParagraph (`modules/skparagraph/`)

## Overview

`modules/skparagraph/` (`namespace skia::textlayout`) is Skia's rich, multi-line paragraph layout and rendering engine, used by Flutter and CanvasKit. It orchestrates [`../skunicode/`](../skunicode/) and [`../skshaper/`](../skshaper/) to layout, measure, query, and paint multi-styled paragraphs:

* **Multi-Style Paragraph Construction**: Builds styled text blocks and inline widget placeholders from a stack of `TextStyle`s (`ParagraphBuilder`).
* **Unicode Analysis & Shaping**: Computes UTF-8/UTF-16 index mappings, BiDi embedding levels, grapheme clusters, word boundaries, and multi-script font fallback runs (`OneLineShaper`), caching shaped runs in `ParagraphCache`.
* **Line Breaking & Formatting**: Wraps shaped runs into lines (`TextWrapper`, `TextLine`) with support for `maxLines`, custom ellipsis truncation, soft hyphenation, and `StrutStyle` vertical metrics.
* **Painting & Hit-Testing**: Renders backgrounds, text shadows, glyph runs, and underline/overline/strikethrough decorations (`ParagraphPainter`, `Decorations`), and supports glyph range box queries, cursor affinity hit-testing, and word boundary lookup.

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`include/`](include/) | Public `skia::textlayout` API headers for paragraph building, styling, font resolution, caching, metrics, and painting. |
| [`src/`](src/) | Internal layout pipeline implementation (`ParagraphImpl`, `OneLineShaper`, `TextWrapper`, `TextLine`, `Run`, `Decorations`). |
| [`utils/`](utils/) | Testing font collection helpers (`TestFontCollection`). |
| [`gm/`](gm/) | Golden Master rendering tests for paragraph layout, BiDi text, and decorations. |
| [`tests/`](tests/) | Unit tests covering paragraph layout, shaping, line breaking, caching, and hit-testing queries. |
| [`bench/`](bench/) | Microbenchmarks for paragraph layout and shaping performance. |
| [`slides/`](slides/) | Interactive Viewer slides demonstrating `skparagraph` capabilities. |

## Key Classes and Entrypoints

The primary classes and entrypoints in `modules/skparagraph/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Paragraph Building & Layout** | [`ParagraphBuilder.h`](include/ParagraphBuilder.h), [`Paragraph.h`](include/Paragraph.h), [`ParagraphBuilderImpl.h`](src/ParagraphBuilderImpl.h), [`ParagraphImpl.h`](src/ParagraphImpl.h) | Style-stack builder (`ParagraphBuilder`) and primary layout, measurement, hit-testing, and rendering interface (`Paragraph`, `ParagraphImpl`). |
| **Styling & Metrics** | [`ParagraphStyle.h`](include/ParagraphStyle.h), [`TextStyle.h`](include/TextStyle.h), [`Metrics.h`](include/Metrics.h), [`DartTypes.h`](include/DartTypes.h) | Paragraph-level alignment, direction, ellipsis, and strut configuration (`ParagraphStyle`, `StrutStyle`), span-level styling (`TextStyle`, `PlaceholderStyle`), and line/glyph metrics (`LineMetrics`, `TextBox`). |
| **Font Resolution & Caching** | [`FontCollection.h`](include/FontCollection.h), [`TypefaceFontProvider.h`](include/TypefaceFontProvider.h), [`ParagraphCache.h`](include/ParagraphCache.h), [`FontArguments.h`](include/FontArguments.h) | Prioritized `SkFontMgr` lookup and multi-script fallback (`FontCollection`), custom typeface registration (`TypefaceFontProvider`), and thread-safe shaped run caching (`ParagraphCache`). |
| **Shaping & Line Wrapping** | [`OneLineShaper.h`](src/OneLineShaper.h), [`Run.h`](src/Run.h), [`TextWrapper.h`](src/TextWrapper.h), [`TextLine.h`](src/TextLine.h) | BiDi and font-fallback itemization (`OneLineShaper`), shaped glyph clusters (`Run`, `Cluster`), width-constrained line breaking (`TextWrapper`), and laid-out line representation (`TextLine`). |
| **Painting & Decorations** | [`ParagraphPainter.h`](include/ParagraphPainter.h), [`ParagraphPainterImpl.h`](src/ParagraphPainterImpl.h), [`Decorations.h`](src/Decorations.h), [`TextShadow.h`](include/TextShadow.h) | Abstract drawing backend (`ParagraphPainter`), `SkCanvas` implementation (`CanvasParagraphPainter`), underline/strikethrough/wavy decoration rendering (`Decorations`), and drop shadow parameters (`TextShadow`). |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../skshaper/`](../skshaper/) | Low-level OpenType text shaping invoked by `OneLineShaper`. |
| [`../skunicode/`](../skunicode/) | Unicode BiDi reordering, grapheme clustering, and line/word boundary analysis. |
| [`../canvaskit/`](../canvaskit/) | WebAssembly bindings exposing `skparagraph` to JavaScript and web applications. |
