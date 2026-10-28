---
name: skia/gpu/enforce-disjoint-layers-for-barrier-draws
description: When recording barrier-requiring draws in layer-based draw lists, enforce strict layer disjointness during bounds testing instead of relying on painter's order comparisons to gate flushes
rubric: correctness
severity: should_fix
---

**Rule.** In Graphite layer-based draw list recording (such as DrawListLayer),
do not rely on CompressedPaintersOrder to detect draw overlaps or gate barrier
flushes in snapDrawPass. Require draws with execution barriers
(barrierBeforeDraws != BarrierType::kNone) to be strictly disjoint during layer
bounds testing—disabling compatible-overlap shortcuts—so snapDrawPass can flush
on any barrier draw at the layer boundary without comparing painter's order.

**Why.** Layer-based draw lists bypass spatial BoundsManager tracking to
eliminate CPU overhead, meaning CompressedPaintersOrder does not reliably
indicate spatial overlaps across draws. Enforcing layer disjointness during
layer bounds testing guarantees barrier-dependent draws execute without
overlapping hazards and keeps pass-snapping flush logic simple and correct.

## Examples

Bad:

```cpp
// In snapDrawPass, relying on painter's order comparisons for barrier flushes:
} else if (drawParams.barrierBeforeDraws() != BarrierType::kNone &&
           priorDrawPaintOrder != drawParams.order().paintOrder()) {
    drawWriter.flush();
}
```

Good:

```cpp
// In recordDraw / bounds test, enforce disjointness when a barrier is required:
if (barrierBeforeDraws != BarrierType::kNone || renderer->numRenderSteps() > 1) {
    baseLayerMask |= BoundsFlags::kMustBeDisjoint;
}

// In snapDrawPass, flush without comparing painter's order:
} else if (drawParams.barrierBeforeDraws() != BarrierType::kNone && startOfLayer) {
    drawWriter.flush();
}
```
