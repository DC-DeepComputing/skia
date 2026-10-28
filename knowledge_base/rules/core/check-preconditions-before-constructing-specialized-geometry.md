---
name: skia/core/check-preconditions-before-constructing-specialized-geometry
description: When entering specialized draw paths in canvas dispatch, verify eligibility preconditions before constructing intermediate geometry representations like SkRRect
rubric: safety
severity: should_fix
---

**Rule.** In draw dispatch routines (such as SkCanvas::onDraw*), verify
eligibility preconditions—such as paint mask filters or path effects—before
constructing intermediate geometry objects like SkRRect or calling specialized
rendering helpers. Do not unconditionally convert shapes (e.g., via
SkRRect::MakeRect or SkRRect::MakeOval) for specialized paths that only apply to
a fraction of draw calls.

**Why.** Specialized rendering techniques such as analytic blurs only apply when
specific paint effects are active. Constructing intermediate geometry
representations and calling specialized helpers unconditionally introduces
unnecessary CPU overhead and function call indirection to common rendering
paths.

## Examples

Bad:

```cpp
// Unconditionally constructs SkRRect even when paint has no mask filter
std::optional<AutoLayerForImageFilter> layer =
        this->attemptBlurredRRectDraw(SkRRect::MakeOval(oval), paint, PredrawFlags::kNone);
if (layer) {
    this->topDevice()->drawOval(oval, layer->paint());
}
```

Good:

```cpp
// Check cheap paint preconditions before constructing intermediate geometry
std::optional<AutoLayerForImageFilter> layer;
if (const SkBlurMaskFilterImpl* blurMaskFilter = this->canAttemptBlurredRRectDraw(paint)) {
    layer = this->attemptBlurredRRectDraw(
            SkRRect::MakeOval(oval), blurMaskFilter, paint, PredrawFlags::kNone);
} else {
    layer = this->aboutToDraw(paint, &oval);
}
if (layer) {
    this->topDevice()->drawOval(oval, layer->paint());
}
```
