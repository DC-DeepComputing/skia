---
name: skia/modules/preserve-linebreaker-rounding-tolerance
description: When eliminating or gating SkParagraph layout rounding hacks, do not remove or disable LineBreakerWithLittleRounding which provides essential line-fitting float tolerance
rubric: correctness
severity: should_fix
---

**Rule.** In SkParagraph line wrapping (such as in TextWrapper.cpp), do not
disable or gate LineBreakerWithLittleRounding behind layout rounding hack flags.
While position and metrics rounding (like littleRound) were introduced for test
compatibility, LineBreakerWithLittleRounding provides necessary floating-point
tolerance so text fits line widths without premature line breaks.

**Why.** Removing floating-point tolerance from line breaking causes characters
or words that should fit within a line to overflow prematurely due to slight
scalar rounding drift. Line breaker tolerance is an algorithmic requirement for
line fitting rather than a dispensable golden test metrics hack.

## Examples

Bad:

```cpp
bool breakLine(SkScalar width) const {
    if (!fApplyRoundingHack) {
        return width > fMaxWidth && !nearlyEqual(width, fMaxWidth);
    }
    if (width < fLower) {
        return false;
    }
    return width > fUpper;
}
```

Good:

```cpp
bool breakLine(SkScalar width) const {
    if (width < fLower) {
        return false;
    } else if (width > fUpper) {
        return true;
    }
    return width > fMaxWidth && !nearlyEqual(width, fMaxWidth);
}
```
