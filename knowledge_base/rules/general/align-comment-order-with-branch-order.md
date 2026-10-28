---
name: skia/align-comment-order-with-branch-order
description: When documenting conditional if-else statements, align introductory comment order with branch execution sequence or place branch-specific comments directly inside each block
rubric: readability
severity: nit
---

**Rule.** When documenting conditional if-else branches, ensure explanatory
comments correspond directly to the execution flow. When writing an introductory
comment preceding an if statement, align the sequence of described cases with
the execution order of the branches (describe the primary if condition first,
not the fallback or else case). Alternatively, place branch-specific
explanations directly inside their respective if or else blocks rather than
explaining alternative branches above the initial condition.

**Why.** Readers naturally assume a comment immediately preceding an if
statement describes the primary condition for entering the true branch.
Describing fallback or alternative cases before the initial condition, or
placing an else explanation directly above an if check, causes confusion and
cognitive friction. Co-locating comments inside branch blocks or matching
execution order keeps documentation aligned with control flow.

## Examples

Bad:

```cpp
// Variant 1: Describing fallback before primary branch in introductory comment
// If this isn't the first element, draw inverse shape with 0 coverage.
// But if we are the first element, draw directly with coverage 1.
if (isFirstElement) {
    alpha = 0xFF;
} else {
    alpha = 0x00;
}

// Variant 2: Placing else-branch rationale directly above the if statement
// When no effects are visible, visibleLayerBounds represents a smaller image.
if (canDeferTiling && (analysis & BoundsAnalysis::kEffectsVisible)) {
    srcRect = bounds;
} else {
    srcRect = visibleLayerBounds;
}
```

Good:

```cpp
// Variant 1: Align introductory comment order with execution order
// If this is the first element, draw directly with coverage 1.
// Otherwise, draw the inverse-filled shape with 0 coverage.
if (isFirstElement) {
    alpha = 0xFF;
} else {
    alpha = 0x00;
}

// Variant 2: Place branch-specific explanations inside respective blocks
if (canDeferTiling && (analysis & BoundsAnalysis::kEffectsVisible)) {
    // When effects are visible, rescaling uses smaller textures.
    srcRect = bounds;
} else {
    // Otherwise, visibleLayerBounds represents a smaller effective image.
    srcRect = visibleLayerBounds;
}
```
