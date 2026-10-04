`SkGlyphID` is now a 32-bit unsigned integer, enabling fonts with more than
65,536 glyphs when supported by the font backend. This changes the C++ ABI
and the element size of `SkTextEncoding::kGlyphID` text. CanvasKit glyph ID
arrays and `MallocGlyphIDs` now use `Uint32Array`. Existing serialized pictures
and text blobs with 16-bit glyph arrays remain readable. Fonts too large for
PDF's embedded-font encoding use Type3 fonts.
