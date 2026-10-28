---
name: skia/gm/tests-colrv1-address-glyphs-by-codepoint
description: In tests using the COLRv1 test font, address test glyphs by their designated Unicode codepoints via drawText instead of hardcoded glyph IDs or drawGlyphs
rubric: readability
severity: should_fix
---

**Rule.** In tests and GMs using the COLRv1 test font
(test_glyphs-glyf_colr_1.ttf), address test glyphs by their designated Unicode
codepoints (via drawText, drawSimpleText, or UTF-32 text APIs) rather than by
hardcoded glyph IDs (via drawGlyphs or getGlyphIDs). Separate adjacent test
glyphs using whitespace instead of calculating manual run positions.

**Why.** Glyph IDs in font files can shift when the font is regenerated or
updated from upstream test suites. The COLRv1 test font defines stable Unicode
codepoints in the Private Use Area (PUA) for each test case, allowing tests to
rely on standard text-drawing APIs and natural whitespace spacing rather than
brittle glyph IDs and manual run positioning.

## Examples

Bad:

```cpp
// Brittle: glyph IDs can shift on font regeneration, requiring manual positioning
const glyphIDs = font.getGlyphIDs('...');
const positions = calculateRun(testFont, glyphIDs);
canvas.drawGlyphs(glyphIDs, positions, 5, y, testFont, textPaint);
```

Good:

```cpp
// Robust: test font maps test cases to stable PUA codepoints separated by spaces
const testCodepoints = "\u{F0200} \u{F0100} \u{F0101}";
canvas.drawText(testCodepoints, 5, y, textPaint, testFont);
```
