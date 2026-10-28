---
name: skia/gm/test-hairlines-at-pixel-boundaries-and-half-pixel-offsets
description: When testing hairlines or stroke caps in GMs, include both integer-pixel and half-pixel alignments separated into distinct comparison rows
rubric: testing
severity: should_fix
---

**Rule.** When authoring visual GM tests or rendering tests for hairline strokes
(width 0) or stroke caps, include geometry aligned on integer pixel coordinates
as well as at half-pixel (0.5) offsets. Separate integer-aligned and
fractional-aligned geometry into distinct rows or test cases rather than mixing
alignments across shapes within the same row.

**Why.** Hairline strokes render with an effective width of 1 device pixel and
interact differently with antialiasing depending on grid alignment: centered at
half-pixels (0.5), hairlines occupy a single pixel row or column with full
coverage, whereas at integer coordinates coverage splits across adjacent pixels.
Stroke caps extend endpoints by up to half a pixel, making both alignments
necessary. Mixing integer and fractional alignments in the same comparison row
confounds visual inspection, making stroke caps appear inconsistent across
geometry types; separating them into distinct rows isolates subpixel
antialiasing differences and ensures both modes are systematically covered.

## Examples

Bad:

```cpp
// Bad: Mixing half-pixel coordinates for lines with integer coordinates for curves in the same row,
// or testing only a single grid alignment mode.
auto line = SkPathBuilder().moveTo(5.5, 45.5).lineTo(5.5, 50.5).detach();
auto quad = SkPathBuilder().moveTo(20, 45).quadTo(40, 55, 20, 60).detach();
canvas->drawPath(line, paint);
canvas->drawPath(quad, paint);
```

Good:

```cpp
// Good: Draw integer-pixel contours on one row, and half-pixel offset contours on a separate row
auto lineOn = SkPathBuilder().lineTo(0, 5).lineTo(5, 5).detach().makeOffset(5, 45);
auto quadOn = SkPathBuilder().moveTo(0, 0).quadTo(20, 10, 0, 15).detach().makeOffset(20, 45);
canvas->drawPath(lineOn, paint);
canvas->drawPath(quadOn, paint);

canvas->translate(0, 60);
auto lineOff = lineOn.makeOffset(0.5, 0.5);
auto quadOff = quadOn.makeOffset(0.5, 0.5);
canvas->drawPath(lineOff, paint);
canvas->drawPath(quadOff, paint);
```
