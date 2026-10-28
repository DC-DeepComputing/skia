---
name: skia/modules/mirror-cluster-edge-corrections-for-rtl-runs
description: When calculating clip bounds, edge adjustments, or hit-test traversal for SkParagraph runs, branch on run->leftToRight() to handle RTL text geometry
rubric: correctness
severity: blocking
---

**Rule.** When calculating clipping bounds, cluster edge corrections, or
hit-test traversal for text runs in SkParagraph (such as in TextLine), branch on
run direction using run->leftToRight(). For right-to-left (RTL) runs, mirror
leftCorrection and rightCorrection across clip.fLeft and clip.fRight and adjust
text start offset with rightCorrection. Additionally, do not assume trailing
whitespace (fExcludedTrailingSpaces) always extends clip.fRight or that visual
scanning can terminate early assuming left-to-right progression; condition clip
adjustments and loop continuation on run direction.

**Why.** In right-to-left text runs, logical character order and trailing
whitespace are oriented oppositely from visual coordinate space. Applying
left-to-right edge adjustments or assuming unidirectional scan progression
distorts RTL run clip bounds, produces incorrect glyph hit-testing, and triggers
line-width assertions.

## Examples

Bad:

```cpp
// 1. Cluster edge corrections in measureTextInsideOneRun:
result.clip.fLeft += leftCorrection;
result.clip.fRight -= rightCorrection;
textStartInLine -= leftCorrection;

// 2. Trailing spaces and hit-test traversal:
context.clip.fRight += context.fExcludedTrailingSpaces;
if (dx < context.clip.fLeft + offsetX) {
    return keepLooking = false;
}
```

Good:

```cpp
// 1. Cluster edge corrections in measureTextInsideOneRun:
if (run->leftToRight()) {
    result.clip.fLeft += leftCorrection;
    result.clip.fRight -= rightCorrection;
    textStartInLine -= leftCorrection;
} else {
    result.clip.fRight -= leftCorrection;
    result.clip.fLeft += rightCorrection;
    textStartInLine -= rightCorrection;
}

// 2. Trailing spaces and hit-test traversal:
if (run->leftToRight()) {
    context.clip.fRight += context.fExcludedTrailingSpaces;
}
if (dx < context.clip.fLeft + offsetX) {
    return keepLooking = !run->leftToRight();
}
```
