Add a callback-receiving iterator API to SkStrikeRef `void getWidths(int count,
const uint32_t* glyphs, size_t stride, WidthsCallback callback) const` for
retrieving strikes into an application defined output buffer. This enables
client to provide and define a glyph-advances processing and output function to
avoid collecting into temporarily allocated vectors.
