---
name: skia/modules/test-ltr-and-rtl-in-skparagraph
description: When modifying SkParagraph text layout, positioning, or hit-testing logic, author unit tests covering both left-to-right and right-to-left text flows
rubric: testing
severity: should_fix
---

**Rule.** When adding or modifying text layout, shaping, run measurement, or
hit-testing logic in SkParagraph, author unit tests (via UNIX_ONLY_TEST)
covering both left-to-right (LTR) and right-to-left (RTL) text flows. Configure
test paragraph styles with both TextDirection::kLtr and TextDirection::kRtl to
verify that coordinate calculations and glyph boundaries are correct in both
directions.

**Why.** Bidirectional text layout and hit-testing have mirrored geometry and
ordering rules. Tests that only exercise left-to-right text easily overlook
coordinate sign inversions, edge correction bugs, or trailing whitespace
positioning errors that manifest specifically under right-to-left flow.

## Examples

Bad:

```cpp
// Only verifying LTR text positioning:
UNIX_ONLY_TEST(SkParagraph_LTRGlyphPositionsForTrailingSpaces, reporter) {
    ParagraphStyle style;
    style.setTextDirection(TextDirection::kLtr);
    // ...
}
```

Good:

```cpp
UNIX_ONLY_TEST(SkParagraph_LTRGlyphPositionsForTrailingSpaces, reporter) {
    ParagraphStyle style;
    style.setTextDirection(TextDirection::kLtr);
    // ...
}

UNIX_ONLY_TEST(SkParagraph_RTLGlyphPositionsForTrailingSpaces, reporter) {
    ParagraphStyle style;
    style.setTextDirection(TextDirection::kRtl);
    // ...
}
```
