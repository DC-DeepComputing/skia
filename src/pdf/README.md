# Skia PDF Backend (`src/pdf/`)

## Overview & Scope
`src/pdf/` implements Skia's vector PDF document generation backend. It allows clients to record standard `SkCanvas` drawing operations page-by-page via the `SkDocument` interface and serialize them into a Portable Document Format (PDF) stream, supporting PDF/A-2b conformance, font subsetting, vector gradients/shaders, transparency groups, and accessible Tagged PDF structure trees.

The public entrypoint is [`include/docs/SkPDFDocument.h`](../../include/docs/SkPDFDocument.h) (`SkPDF::MakeDocument` and `SkPDF::Metadata`), alongside optional JPEG callback helpers in [`include/docs/SkPDFJpegHelpers.h`](../../include/docs/SkPDFJpegHelpers.h).

## Key Abstractions & Entrypoints

### Document Lifecycle & Page Device
- **`SkPDFDocument` ([`SkPDFDocument.cpp`](SkPDFDocument.cpp), [`SkPDFDocumentPriv.h`](SkPDFDocumentPriv.h))**: Subclasses `SkDocument` and manages document-wide state across pages:
  - `SkPDF::MakeDocument(SkWStream*, const SkPDF::Metadata&)` creates the document, writes the `%PDF-1.7` header, and initializes the object serializer (`SkPDFObjectSerializer`), font cache, resource caches, and optional background `SkExecutor`.
  - `onBeginPage()` instantiates a page-level `SkPDFDevice` wrapped in an `SkCanvas`; `onEndPage()` flushes page content streams and emits the `/Page` dictionary; `onClose()` emits embedded/subsetted fonts, the structure tree, document catalog (`/Catalog`), `/Info` and XMP metadata ([`SkPDFMetadata.cpp`](SkPDFMetadata.cpp)), cross-reference table (`xref`), and trailer.
  - When PDF support is disabled at build time, [`SkDocument_PDF_None.cpp`](SkDocument_PDF_None.cpp) stubs `SkPDF::MakeDocument` to return `nullptr`.
- **`SkPDFDevice` ([`SkPDFDevice.cpp`](SkPDFDevice.cpp), [`SkPDFDevice.h`](SkPDFDevice.h))**: Subclasses `SkClipStackDevice` (`SkDevice`) to translate `SkCanvas` draw calls (`drawPaint`, `drawPoints`, `drawRect`, `drawRRect`, `drawPath`, `drawImageRect`, `drawGlyphRunList`, `drawVertices`, `drawMesh`) into PDF content stream operators (`m`, `l`, `c`, `re`, `f`, `S`, `Do`, `BT`/`ET`, `Tj`/`TJ`).
  - Tracks graphic stack state (matrix, clip, color, font, shader, graphic state index) via [`SkPDFGraphicStackState.cpp`](SkPDFGraphicStackState.cpp) and packages page resources (`/ExtGState`, `/Pattern`, `/XObject`, `/Font`) via [`SkPDFResourceDict.cpp`](SkPDFResourceDict.cpp).
  - Handles save layers (`onCreateDevice`), masks, blend modes, and image filters by creating offscreen `SkPDFDevice` instances and wrapping them as PDF Form XObjects ([`SkPDFFormXObject.cpp`](SkPDFFormXObject.cpp)).

### PDF Object Serialization & Compression
- **`SkPDFTypes` & `SkPDFUnion` ([`SkPDFTypes.h`](SkPDFTypes.h), [`SkPDFTypes.cpp`](SkPDFTypes.cpp), [`SkPDFUnion.h`](SkPDFUnion.h))**: Represents the PDF object model:
  - `SkPDFUnion`: Discriminated union for scalar/inline PDF primitives (Int,Bool, Scalar, Name, Byte/Text String, and Indirect `SkPDFIndirectPtr` object references).
  - `SkPDFArray` and `SkPDFDict`: Composite PDF containers serialized directly to `SkWStream`.
  - `SkPDFIndirectPtr`: Lightweight reference ID (`N 0 R`) allocated and serialized on-demand through `SkPDFDocument::emit(const SkPDFObject&, SkPDFIndirectPtr)` to avoid holding large object graphs in memory.
- **Stream Compression ([`SkDeflate.h`](SkDeflate.h), [`SkDeflate.cpp`](SkDeflate.cpp))**: `SkDeflateWStream` wraps `zlib` to compress PDF content and image streams (`/Filter /FlateDecode`), optionally running asynchronously on `Metadata::fExecutor`.

### Fonts, Shaders, Images & Tagged PDF Accessibility
- **Fonts & Subsetting ([`SkPDFFont.cpp`](SkPDFFont.cpp), [`SkPDFSubsetFont.cpp`](SkPDFSubsetFont.cpp), [`SkPDFType1Font.cpp`](SkPDFType1Font.cpp))**:
  - `SkPDFFont` partitions glyphs by advance/encoding, tracks used glyph IDs per typeface via [`SkPDFGlyphUse.h`](SkPDFGlyphUse.h), and supports Type0/CIDFontType2 (TrueType), CIDFontType0 (OpenType CFF), Type1, and Type3 fallback (including bitmap/COLR glyphs).
  - Subsets TrueType/OpenType fonts at document close via HarfBuzz (`hb_subset` in [`SkPDFSubsetFont.cpp`](SkPDFSubsetFont.cpp)), generates `/W` glyph width arrays ([`SkPDFMakeCIDGlyphWidthsArray.cpp`](SkPDFMakeCIDGlyphWidthsArray.cpp)), and builds `/ToUnicode` CMap streams ([`SkPDFMakeToUnicodeCmap.cpp`](SkPDFMakeToUnicodeCmap.cpp)) using glyph-to-text cluster mapping from [`SkClusterator.cpp`](SkClusterator.cpp) so text remains searchable/copyable.
- **Shaders, Gradients & Graphic State**: [`SkPDFGradientShader.cpp`](SkPDFGradientShader.cpp) emits native PDF Type 2/3/4/6/7 `/Shading` dictionaries and Functions for linear, radial, two-point conical, and sweep gradients; [`SkPDFShader.cpp`](SkPDFShader.cpp) handles image tiling patterns and fallback shader rasterization; [`SkPDFGraphicState.cpp`](SkPDFGraphicState.cpp) emits `/ExtGState` dictionaries (opacity, stroke parameters, blend modes, soft masks).
- **Images ([`SkPDFBitmap.cpp`](SkPDFBitmap.cpp), [`SkKeyedImage.cpp`](SkKeyedImage.cpp))**: Serializes `SkImage`s as `/XObject /Image` streams, embedding original JPEG data directly (`/DCTDecode`) when `Metadata::jpegDecoder` / `jpegEncoder` are provided or Flate-encoding pixel/alpha (`SMask`) channels.
- **Tagged PDF & Structure Tree ([`SkPDFTag.h`](SkPDFTag.h), [`SkPDFTag.cpp`](SkPDFTag.cpp))**: Implements `SkPDFStructTree` from `Metadata::fStructureElementTreeRoot` (`SkPDF::StructureElementNode`) and `SkPDF::SetNodeId(SkCanvas*, int)`. Emits Marked Content sequences (`BDC`/`EMC` with `/MCID`), `/StructTreeRoot`, `/StructElem` hierarchies, `/ParentTree`, and document outlines (`/Outlines`) for screen reader accessibility.

## Subdirectories & Related Modules
- [`../../include/docs/`](../../include/docs/): Public `SkPDFDocument.h` and `SkPDFJpegHelpers.h` headers.
- [`../codec/`](../codec/) & [`../encode/`](../encode/): Used for JPEG pass-through inspection/encoding and sRGB/ICC profile generation (`SkWriteICCProfile`).
- [`../utils/`](../utils/): `SkClipStackUtils` (resolving clip stacks into single paths via `pathops`) and `SkFloatToDecimal`.

## Build & Test
- **Bazel (`BUILD.bazel`)**: Defines `:pdf` (with `SK_PDF_USE_HARFBUZZ_SUBSET`, `@zlib`, `@harfbuzz`, and `//:pathops`) and `:jpeg_helpers`.
- **GN (`../../gn/pdf.gni` & `../../gn/skia.gni`)**: Controlled by `skia_enable_pdf`, `skia_use_zlib`, and `skia_pdf_subset_harfbuzz` (defines `SK_SUPPORT_PDF`).
- **Tests & GMs**: Covered by `tests/PDFDocumentTest.cpp`, `tests/PDFPrimitivesTest.cpp`, `tests/PDFTaggedTest.cpp`, `tests/PDFTaggedTableTest.cpp`, `tests/PDFTaggedLinkTest.cpp`, `tests/PDFGlyphsToUnicodeTest.cpp`, `tests/PDFJpegEmbedTest.cpp`, `tests/PDFMetadataAttributeTest.cpp`, `tests/PDFDeflateWStreamTest.cpp`, and `gm/pdf*.cpp`.
