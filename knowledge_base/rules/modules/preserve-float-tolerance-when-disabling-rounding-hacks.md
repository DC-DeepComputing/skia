---
name: skia/modules/preserve-float-tolerance-when-disabling-rounding-hacks
description: When providing options to disable coordinate snapping or rounding hacks, preserve floating-point tolerance comparisons unconditionally to prevent layout precision regressions
rubric: correctness
severity: should_fix
---

**Rule.** Do not bypass or disable floating-point tolerance or fuzzy equality
checks (such as relative difference or nearlyEqual comparisons) when disabling
coordinate rounding or snapping hacks. Keep numerical tolerance checks active
across all rounding modes so accumulated floating-point error does not trigger
false inequality or overflow.

**Why.** Coordinate snapping and rounding hacks (such as integer or subpixel
alignment for legacy compatibility) operate at a coarser scale than
floating-point precision tolerances. Floating-point inaccuracies from cumulative
scaling, advances, and letter spacing accumulate regardless of whether rounding
hacks are active, so bypassing tolerances when rounding is turned off causes
spurious layout line-breaking and boundary failures.

## Examples

Bad:

```cpp
// Bypassing tolerance comparisons when rounding hacks are disabled:
const bool exceedsLineWidth = fOwner->getApplyRoundingHack()
    ? compareRound(result.clip.fRight, fAdvance.fX) > 0
    : result.clip.fRight > fAdvance.fX;
```

Good:

```cpp
// Preserve tolerance comparison unconditionally, gating only the rounding hack:
int compareRound(SkScalar a, SkScalar b, bool applyRoundingHack) {
    if (nearlyEqual(a, b)) {
        return 0;
    }
    SkScalar ra = applyRoundingHack ? littleRound(a) : a;
    SkScalar rb = applyRoundingHack ? littleRound(b) : b;
    return ra < rb ? -1 : (ra > rb ? 1 : 0);
}
```
