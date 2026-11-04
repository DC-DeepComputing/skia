# Skia PDF Backend (`src/pdf/`)

## Overview

`src/pdf/` implements Skia's vector PDF document generation backend, backing the public document APIs in [`../../include/docs/`](../../include/docs/):

* **Page-by-Page Document Generation**: Records `SkCanvas` drawing operations via `SkDocument` and serializes them into PDF 1.4 / PDF/A-2b streams.
* **Vector Graphics & Shaders**: Translates paths, clipping, blend modes, transparency layers (Form XObjects), and linear/radial/conical/sweep gradients into native PDF operators and shading dictionaries.
* **Font Embedding & Subsetting**: Tracks per-typeface glyph usage, subsets TrueType/OpenType fonts via HarfBuzz, and emits `/ToUnicode` CMap tables for searchable text.
* **Tagged PDF Accessibility**: Emits Marked Content sequences (`BDC`/`EMC`) and `/StructTreeRoot` hierarchies for screen reader navigation and document outlines.

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/pdf/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Document & Page Device** | [`SkPDFDocument.cpp`](SkPDFDocument.cpp), [`SkPDFDocumentPriv.h`](SkPDFDocumentPriv.h), [`SkPDFDevice.h`](SkPDFDevice.h), [`SkPDFGraphicStackState.h`](SkPDFGraphicStackState.h), [`SkPDFMetadata.h`](SkPDFMetadata.h) | Multi-page lifecycle, cross-reference table, and object serialization (`SkPDFDocument`), plus the page-level `SkDevice` translating draw calls into PDF content streams (`SkPDFDevice`). |
| **PDF Object Model & Compression** | [`SkPDFTypes.h`](SkPDFTypes.h), [`SkPDFUnion.h`](SkPDFUnion.h), [`SkPDFResourceDict.h`](SkPDFResourceDict.h), [`SkDeflate.h`](SkDeflate.h) | PDF primitives, arrays, dictionaries, indirect references (`SkPDFUnion`, `SkPDFDict`), page resource dictionaries, and Flate (`zlib`) stream compression (`SkDeflateWStream`). |
| **Fonts & Subsetting** | [`SkPDFFont.h`](SkPDFFont.h), [`SkPDFSubsetFont.h`](SkPDFSubsetFont.h), [`SkPDFType1Font.h`](SkPDFType1Font.h), [`SkPDFGlyphUse.h`](SkPDFGlyphUse.h), [`SkPDFMakeToUnicodeCmap.h`](SkPDFMakeToUnicodeCmap.h), [`SkPDFMakeCIDGlyphWidthsArray.h`](SkPDFMakeCIDGlyphWidthsArray.h), [`SkClusterator.h`](SkClusterator.h) | Type0/CID, Type1, and Type3 font emission (`SkPDFFont`), HarfBuzz font subsetting, glyph width arrays, and UTF-to-glyph `/ToUnicode` CMap generation. |
| **Shaders, Images & XObjects** | [`SkPDFGradientShader.h`](SkPDFGradientShader.h), [`SkPDFShader.h`](SkPDFShader.h), [`SkPDFGraphicState.h`](SkPDFGraphicState.h), [`SkPDFFormXObject.h`](SkPDFFormXObject.h), [`SkPDFBitmap.h`](SkPDFBitmap.h), [`SkKeyedImage.h`](SkKeyedImage.h) | Native PDF gradient shadings, tiled image patterns, `/ExtGState` dictionaries, offscreen Form XObjects, and JPEG pass-through / Flate-compressed image streams. |
| **Tagged PDF & Accessibility** | [`SkPDFTag.h`](SkPDFTag.h) | Structure tree (`SkPDFStructTree`) and marked-content tag tracking for accessible Tagged PDF documents. |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../../include/docs/`](../../include/docs/) | Public PDF document creation (`SkPDF::MakeDocument`), metadata, and JPEG helper headers. |
| [`../core/`](../core/) | Core `SkDocument`, `SkDevice`, clip stack, and glyph run interfaces. |
| [`../codec/`](../codec/) | JPEG metadata inspection used when embedding raw JPEG streams into PDFs. |
| [`../encode/`](../encode/) | JPEG encoding and ICC profile generation (`SkWriteICCProfile`) for PDF color spaces. |
