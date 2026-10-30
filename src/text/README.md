# Skia Text Pipeline & GPU Text (`src/text/`)

## Overview & Scope
`src/text/` implements Skia's internal text representation, glyph run conversion, and backend-agnostic GPU text preparation. It bridges public text drawing APIs in [`include/core/`](../../include/core/) (`SkCanvas::drawTextBlob`, `SkCanvas::drawGlyphs`, `SkFont`, `SkTextBlob`) and [`include/private/chromium/Slug.h`](../../include/private/chromium/Slug.h) (`sktext::gpu::Slug`) with Skia's CPU and GPU rendering backends.

Note on architectural boundaries:
- **`src/text/`** operates on positioned glyph IDs (`SkGlyphID` + `SkPoint` / `SkRSXform`), converting them into `GlyphRun`s and GPU `SubRun`s.
- **[`src/sfnt/`](../sfnt/)** defines low-level TrueType/OpenType (`sfnt`) binary font table structures (`head`, `hhea`, `maxp`, `name`, `OS/2`, `post`, `COLR`) used by font ports and subsetters.
- **[`modules/skshaper/`](../../modules/skshaper/) & [`modules/skparagraph/`](../../modules/skparagraph/)** handle Unicode text segmentation, BiDi, HarfBuzz glyph shaping, line breaking, and rich multi-style paragraph layout *before* handing positioned glyphs to Skia core and `src/text/`.

## Key Abstractions & Entrypoints

### Core Glyph Runs (`src/text/`)
- **[`GlyphRun.h`](GlyphRun.h) / [`GlyphRun.cpp`](GlyphRun.cpp)**:
  - `sktext::GlyphRun`: Immutable view pairing an `SkFont` with `SkZip<const SkGlyphID, const SkPoint>` positions, optional original UTF text/clusters, and scaled rotations (`SkRSXform`).
  - `sktext::GlyphRunList`: A collection of `GlyphRun`s originating from a single draw call or `SkTextBlob`, carrying source bounds and origin.
  - `sktext::GlyphRunBuilder`: Converts raw UTF-8/16/32 text strings, `SkTextBlob`s, or glyph arrays into a `GlyphRunList` for downstream devices (`SkDevice`).
- **[`StrikeForGPU.h`](StrikeForGPU.h) / [`StrikeForGPU.cpp`](StrikeForGPU.cpp)**: Defines `sktext::StrikeForGPU` and `StrikeForGPUCacheInterface`, abstracting thread-safe glyph metric/digest lookup and rounding specs (`SkStrikeSpec`) required to partition runs for GPU rendering.
- **[`SlugFromBuffer.cpp`](SlugFromBuffer.cpp)**: Deserialization entrypoint (`Slug::MakeFromBuffer`) for reconstructing serialized GPU text slugs.

### Shared GPU Text Pipeline (`src/text/gpu/`)
[`src/text/gpu/`](gpu/) is shared by both **Ganesh** (`src/gpu/ganesh/`) and **Graphite** (`src/gpu/graphite/`). It inspects a `GlyphRunList` under the current canvas matrix and paint, consults the strike cache, and partitions glyphs into specialized `SubRun`s:

- **[`SubRunContainer.h`](gpu/SubRunContainer.h) / [`SubRunContainer.cpp`](gpu/SubRunContainer.cpp)**:
  - `SubRunContainer::MakeInAlloc` is the core algorithm that classifies each glyph in a `GlyphRunList` (using `SubRunControl` thresholds in [`SubRunControl.h`](gpu/SubRunControl.h)) into one of several `SubRun` types stored in a `SubRunList`:
    - **`DirectMaskSubRun`** (`AtlasSubRun`): 1:1 device-space mask or color bitmap glyphs uploaded to the GPU glyph atlas (most common path for standard-sized text).
    - **`TransformedMaskSubRun`** (`AtlasSubRun`): Source-space bitmap/color glyphs that require a matrix transform when drawn from the atlas.
    - **`SDFTSubRun`** (`AtlasSubRun`): Signed Distance Field Text for medium-to-large single-color glyphs under scaling/rotation, utilizing [`SDFMaskFilter.h`](gpu/SDFMaskFilter.h) and [`DistanceFieldAdjustTable.h`](gpu/DistanceFieldAdjustTable.h).
    - **`PathSubRun` & `DrawableSubRun`**: Fallback paths for very large glyphs rendered as vector `SkPath`s or `SkDrawable`s (e.g., `COLRv1` / SVG glyphs) rather than atlas masks.
- **[`TextBlob.h`](gpu/TextBlob.h) & [`TextBlobRedrawCoordinator.h`](gpu/TextBlobRedrawCoordinator.h)**: Caches `SubRunContainer` instances across frames (`canReuse`) keyed by matrix/paint state so repeated `SkTextBlob` draws bypass glyph strike lookups.
- **[`Slug.cpp`](gpu/Slug.cpp) & [`SlugImpl.h`](gpu/SlugImpl.h)**: Implements `sktext::gpu::Slug`, an immutable, serializable encapsulation of a `SubRunContainer` pinned to a specific device matrix and paint.
- **[`GlyphVector.h`](gpu/GlyphVector.h) & [`VertexFiller.h`](gpu/VertexFiller.h)**: `GlyphVector` tracks packed glyph IDs ([`PackedGPUGlyphID.h`](gpu/PackedGPUGlyphID.h)) and resolves them into GPU atlas plot locations (`PlotLocator`); `VertexFiller` generates quad geometry and UV coordinates for `AtlasSubRun`s.
- **[`StrikeCache.h`](gpu/StrikeCache.h) & [`SkChromeRemoteGlyphCache.cpp`](gpu/SkChromeRemoteGlyphCache.cpp)**: Manages GPU-side strike caching (`sktext::gpu::StrikeCache`) and Chromium's out-of-process GPU glyph serialization (`SkStrikeServer` / `SkStrikeClient`, using `SubRunCreationBehavior::kStrikeCalculationsOnly`).
- **[`SubRunAllocator.h`](gpu/SubRunAllocator.h)**: Arena allocator (`SkBagOfBytes`) backing low-overhead `SubRun` and `GlyphVector` allocations.

## Subdirectories & Related Modules
- [`gpu/`](gpu/): Backend-agnostic GPU text preparation, `SubRunContainer`, `Slug`, and remote glyph cache.
- [`../core/`](../core/): `SkScalerContext`, `SkStrike`, `SkStrikeCache`, `SkStrikeSpec`, and `SkGlyph` CPU strike management.
- [`../gpu/ganesh/`](../gpu/ganesh/) & [`../gpu/graphite/`](../gpu/graphite/): Consume `AtlasSubRun` and `VertexFiller` to upload glyphs into `GrAtlasManager` / `AtlasProvider` and emit draw ops/render steps.
- [`../sfnt/`](../sfnt/): Raw OpenType/TrueType binary table definitions.
- [`../../modules/skshaper/`](../../modules/skshaper/) & [`../../modules/skparagraph/`](../../modules/skparagraph/): High-level text shaping and paragraph layout.

## Build & Test
- **Bazel**: [`BUILD.bazel`](BUILD.bazel) exports `:text` (`split_srcs_and_hdrs`) to `//src/core:__pkg__`. [`gpu/BUILD.bazel`](gpu/BUILD.bazel) exports `:gpu` (`split_srcs_and_hdrs`) to `//src/gpu/ganesh:__pkg__` and `//src/gpu/graphite:__pkg__`.
- **GN**: File lists are exported to [`../../gn/core.gni`](../../gn/core.gni) and [`../../gn/gpu.gni`](../../gn/gpu.gni) (`skia_text_gpu_sources`).
- **Tests & GMs**: Covered by `tests/TextBlobTest.cpp`, `tests/TextBlobCacheTest.cpp`, `tests/GrTextBlobTest.cpp`, `tests/SlugTest.cpp`, `tests/SkRemoteGlyphCacheTest.cpp`, `tests/DrawTextTest.cpp`, and GMs in `gm/textblob*.cpp` and `gm/slug.cpp`.
