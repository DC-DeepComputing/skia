---
name: skia/avoid-confusing-makexywh-with-makeltrb
description: When positioning rectangles of known size at an offset, use MakeSize().makeOffset() or MakePtSize() instead of passing offset-added coordinates to MakeXYWH
rubric: correctness
severity: should_fix
---

**Rule.** Do not pass right/bottom edge coordinates (such as x + width or
padding + width) into SkIRect::MakeXYWH or SkRect::MakeXYWH, which expects width
and height dimensions rather than boundary coordinates. When constructing a
rectangle of known size at an offset or padding, use
SkIRect::MakeSize(size).makeOffset(dx, dy) or SkIRect::MakePtSize(pt, size) to
decouple dimensions from position and avoid confusing MakeXYWH with MakeLTRB.

**Why.** SkIRect::MakeXYWH(x, y, w, h) internally computes right and bottom as x
+ w and y + h. Passing offset + width or right-edge coordinates into the width
and height arguments inadvertently adds the offset twice, creating an oversized
rectangle that leads to texture atlas padding bugs, clipping inaccuracies, or
out-of-bounds reads.

## Examples

Bad:

```cpp
// Bad: Passing offset + width double-adds the offset in MakeXYWH.
SkIRect entry = SkIRect::MakeXYWH(kEntryPadding,
                                  kEntryPadding,
                                  maskSize.x() + kEntryPadding,
                                  maskSize.y() + kEntryPadding);
```

Good:

```cpp
// Good: Decouple dimensions from offset using MakeSize().makeOffset().
SkIRect entry = SkIRect::MakeSize({maskSize.x(), maskSize.y()})
                        .makeOffset(kEntryPadding, kEntryPadding);
// Or use MakePtSize when starting from a point and size.
SkIRect entry2 = SkIRect::MakePtSize({kEntryPadding, kEntryPadding},
                                     {maskSize.x(), maskSize.y()});
```
