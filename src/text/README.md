# Skia Text Pipeline & GPU Text (`src/text/`)

## Overview

`src/text/` implements Skia's internal positioned glyph run representation and backend-agnostic GPU text preparation pipeline:

* **Positioned Glyph Runs**: Converts UTF text strings, `SkTextBlob`s, and glyph/position arrays into immutable `sktext::GlyphRun` and `GlyphRunList` structures for CPU and GPU `SkDevice`s.
* **GPU SubRun Partitioning ([`gpu/`](gpu/))**: Analyzes glyph runs under the active canvas matrix and paint, consulting strike caches to partition glyphs into direct mask, transformed mask, signed distance field (SDFT), path, or drawable `SubRun`s.
* **Blob Caching & Slugs**: Reuses prepared GPU `SubRunContainer`s across frames via `TextBlobRedrawCoordinator` and supports serializable, matrix-pinned GPU text via `sktext::gpu::Slug`.
* **Remote Glyph Caching**: Supports out-of-process GPU glyph strike serialization (`SkStrikeServer` / `SkStrikeClient`) for sandboxed renderers such as Chromium.

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`gpu/`](gpu/) | Shared GPU text preparation pipeline (`SubRunContainer`, `TextBlob`, `Slug`, vertex filling, and remote glyph caching) used by both Ganesh and Graphite. |

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/text/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Glyph Runs & Builders** | [`GlyphRun.h`](GlyphRun.h), [`StrikeForGPU.h`](StrikeForGPU.h), [`SlugFromBuffer.cpp`](SlugFromBuffer.cpp) | Positioned glyph run views (`sktext::GlyphRun`, `GlyphRunList`, `GlyphRunBuilder`), GPU strike lookup interfaces (`StrikeForGPU`), and `Slug` buffer deserialization. |
| **GPU SubRun Partitioning** | [`SubRunContainer.h`](gpu/SubRunContainer.h), [`SubRunControl.h`](gpu/SubRunControl.h), [`SubRunAllocator.h`](gpu/SubRunAllocator.h) | Classifies glyphs into `AtlasSubRun`s (direct mask, transformed mask, SDFT), `PathSubRun`s, or `DrawableSubRun`s (`SubRunContainer::MakeInAlloc`) backed by arena allocation (`SubRunAllocator`). |
| **Atlas Geometry & SDF Text** | [`GlyphVector.h`](gpu/GlyphVector.h), [`VertexFiller.h`](gpu/VertexFiller.h), [`SDFMaskFilter.h`](gpu/SDFMaskFilter.h), [`DistanceFieldAdjustTable.h`](gpu/DistanceFieldAdjustTable.h) | Packed glyph-to-atlas handle tracking (`GlyphVector`), quad vertex/UV generation (`VertexFiller`), and signed distance field text helpers. |
| **TextBlob Caching, Slugs & Remote Cache** | [`TextBlob.h`](gpu/TextBlob.h), [`TextBlobRedrawCoordinator.h`](gpu/TextBlobRedrawCoordinator.h), [`SlugImpl.h`](gpu/SlugImpl.h), [`StrikeCache.h`](gpu/StrikeCache.h), [`SkChromeRemoteGlyphCache.cpp`](gpu/SkChromeRemoteGlyphCache.cpp) | Cross-frame `SubRunContainer` caching (`TextBlobRedrawCoordinator`), serializable GPU text (`SlugImpl`), GPU strike caching, and Chromium OOP glyph serialization (`SkStrikeServer` / `SkStrikeClient`). |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../core/`](../core/) | CPU glyph run painting (`SkGlyphRunPainter`), `SkStrikeCache`, and `SkScalerContext` strike management. |
| [`../gpu/ganesh/`](../gpu/ganesh/) | Ganesh GPU backend consuming `AtlasSubRun` and `VertexFiller` to upload glyphs and emit draw ops. |
| [`../gpu/graphite/`](../gpu/graphite/) | Graphite GPU backend consuming `AtlasSubRun` and `VertexFiller` for atlas uploads and render steps. |
| [`../sfnt/`](../sfnt/) | Low-level TrueType/OpenType (`sfnt`) binary font table definitions. |
| [`../../modules/skshaper/`](../../modules/skshaper/) | HarfBuzz/CoreText Unicode text shaping that produces positioned glyphs before entering `src/text/`. |
| [`../../modules/skparagraph/`](../../modules/skparagraph/) | Rich multi-style paragraph layout built on top of `skshaper` and `SkTextBlob`. |
