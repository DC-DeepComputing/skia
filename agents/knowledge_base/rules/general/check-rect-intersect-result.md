---
name: skia/check-rect-intersect-result
description: When calling SkRect or SkIRect intersect(), check the boolean return value and handle non-intersection or early-out, because non-overlapping rectangles leave the target unmodified
rubric: correctness
severity: should_fix
---

**Rule.** Do not ignore the return value of SkRect::intersect() or
SkIRect::intersect(). Check the boolean result and handle or early-out when it
returns false, because intersect() leaves the destination rectangle unmodified
(rather than setting it to empty) when rectangles do not overlap or either
rectangle is empty.

**Why.** When two rectangles do not overlap or either is empty,
SkRect::intersect() and SkIRect::intersect() return false and leave the target
rectangle completely unchanged. Ignoring the boolean return value causes the
target to retain its previous unclipped bounds rather than becoming empty,
leading to incorrect clipping, redundant rendering, or missed early-out
opportunities.

## Examples

Bad:

```cpp
void CommandBuffer::setReplayTranslationAndClip(const SkIVector& translation,
                                                const SkIRect& clip,
                                                const SkIRect& renderTargetBounds) {
    fReplayTranslation = translation;
    fRenderPassBounds = renderTargetBounds;

    if (!clip.isEmpty()) {
        // BUG: Leaves fRenderPassBounds unmodified if the rects do not overlap!
        fRenderPassBounds.intersect(clip.makeOffset(translation));
    }
}
```

Good:

```cpp
bool CommandBuffer::setReplayTranslationAndClip(const SkIVector& translation,
                                                const SkIRect& clip,
                                                const SkIRect& renderTargetBounds) {
    fReplayTranslation = translation;
    fRenderPassBounds = renderTargetBounds;

    if (!clip.isEmpty()) {
        if (!fRenderPassBounds.intersect(clip.makeOffset(translation))) {
            return false;
        }
    }
    return true;
}
```
