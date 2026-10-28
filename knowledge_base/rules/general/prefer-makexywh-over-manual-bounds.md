---
name: skia/prefer-makexywh-over-manual-bounds
description: When constructing an SkRect or SkIRect from an origin and dimensions, use SkRect::MakeXYWH or SkIRect::MakeXYWH instead of manually assigning bounds fields
rubric: readability
severity: nit
---

**Rule.** When constructing an SkRect or SkIRect from an origin (x, y) and
dimensions (width, height), use SkRect::MakeXYWH or SkIRect::MakeXYWH rather
than declaring an uninitialized rectangle and manually assigning bounds fields.
Avoid manual coordinate arithmetic like setting fRight to fLeft + width and
fBottom to fTop + height when standard factory methods directly express intent.

**Why.** Using SkRect::MakeXYWH or SkIRect::MakeXYWH is concise, eliminates
repetitive addition of offsets to dimensions, avoids temporary mutable state,
and makes geometric intent immediately clear.

## Examples

Bad:

```cpp
// Manually setting fields with coordinate arithmetic
SkRect subregion;
subregion.fLeft = fX.isValid() ? boundaries.fLeft : defaultSubregion.fLeft;
subregion.fTop = fY.isValid() ? boundaries.fTop : defaultSubregion.fTop;
subregion.fRight =
        subregion.fLeft + (fWidth.isValid() ? boundaries.width() : defaultSubregion.width());
subregion.fBottom =
        subregion.fTop + (fHeight.isValid() ? boundaries.height() : defaultSubregion.height());
return subregion;
```

Good:

```cpp
// Construct directly using SkRect::MakeXYWH
return SkRect::MakeXYWH(
        fX.isValid() ? boundaries.fLeft : defaultSubregion.fLeft,
        fY.isValid() ? boundaries.fTop : defaultSubregion.fTop,
        fWidth.isValid() ? boundaries.width() : defaultSubregion.width(),
        fHeight.isValid() ? boundaries.height() : defaultSubregion.height());
```
