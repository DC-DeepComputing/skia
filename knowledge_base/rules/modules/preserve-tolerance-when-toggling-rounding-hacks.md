---
name: skia/modules/preserve-tolerance-when-toggling-rounding-hacks
description: When making coordinate rounding or snapping hacks conditional, preserve underlying floating-point tolerance comparisons across all modes to prevent false assertion failures
rubric: correctness
severity: should_fix
---

**Rule.** When making legacy rounding hacks or coordinate snapping
transformations conditional behind configuration flags, preserve floating-point
tolerance comparisons (such as relative or absolute epsilon checks) across all
execution modes. Do not bypass tolerance comparisons or revert to exact equality
when the rounding hack is disabled, as accumulated floating-point error still
requires tolerance.

**Why.** Floating-point summation across runs or segments is non-associative and
accumulates precision errors regardless of whether artificial coordinate
snapping is active. Bypassing tolerance comparisons when disabling rounding
causes spurious assertion failures and false inequality detections on valid
layouts.

## Examples

Bad:

```cpp
// Bypassing comparison helper and tolerance when rounding hack is disabled
const bool notNearlyEqual = fOwner->getApplyRoundingHack()
    ? compareRound(totalWidth, this->width()) != 0
    : !nearlyEqual(totalWidth, this->width());
if (!includingGhostSpaces && notNearlyEqual) {
    SkASSERT(false);
}
```

Good:

```cpp
// Preserve the tolerance comparison across all modes; gate only the rounding step
int compareRound(SkScalar a, SkScalar b, bool applyRoundingHack) {
    auto base = std::max(SkScalarAbs(a), SkScalarAbs(b));
    auto diff = SkScalarAbs(a - b);
    if (nearlyZero(base) || diff / base < 0.001f) {
        return 0;
    }
    auto ra = a, rb = b;
    if (applyRoundingHack) {
        ra = littleRound(a);
        rb = littleRound(b);
    }
    return ra < rb ? -1 : (ra > rb ? 1 : 0);
}
...
if (!includingGhostSpaces && compareRound(totalWidth, this->width(), fOwner->getApplyRoundingHack()) != 0) {
    SkASSERT(false);
}
```
