---
name: skia/gm/include-open-contours-when-testing-stroke-caps
description: When authoring tests or GMs for stroke caps, include open (unclosed) contours alongside closed shapes because closed contours render joins instead of caps
rubric: testing
severity: should_fix
---

**Rule.** When authoring tests or GMs that exercise stroke caps (SkPaint::Cap),
include open (unclosed) contours alongside closed shapes. Do not rely
exclusively on closed contours when verifying cap rendering styles (kButt_Cap,
kSquare_Cap, kRound_Cap).

**Why.** In Skia path stroking, closed contours connect endpoints using stroke
joins (Paint_Stroke_Join); stroke caps only render at the endpoints of open
(unclosed) contours. Testing stroke caps on closed paths alone leaves cap
rasterization entirely unexercised.

## Examples

Bad:

```cpp
// Bad: Only closed contours; stroke caps are not rendered at closed contour endpoints
SkPaint paint;
paint.setStrokeCap(SkPaint::kRound_Cap);
paint.setStyle(SkPaint::kStroke_Style);

pb.moveTo(5.5, 5.5).lineTo(5.5, 10.5).lineTo(10.5, 10.5).lineTo(10.5, 5.5).close();
pb.moveTo(20.5, 5.5).quadTo(25, 15, 30.5, 5.5).close();
canvas->drawPath(pb.detach(), paint);
```

Good:

```cpp
// Good: Include open contours (without .close()) so stroke caps are actually rendered
SkPaint paint;
paint.setStrokeCap(SkPaint::kRound_Cap);
paint.setStyle(SkPaint::kStroke_Style);

// Open contours to exercise stroke caps (kButt_Cap, kSquare_Cap, kRound_Cap)
auto openLine = SkPathBuilder().moveTo(5.5, 45.5).lineTo(10.5, 65.5).detach();
auto openQuad = SkPathBuilder().moveTo(20.5, 45.5).quadTo(25, 15, 30.5, 45.5).detach();
canvas->drawPath(openLine, paint);
canvas->drawPath(openQuad, paint);
```
