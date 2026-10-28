---
name: skia/gpu/intersect-scissor-before-recording-draw
description: When recording draws or depth-only clips in Graphite, intersect and snap scissor bounds to target dimensions instead of relaxing DrawContext assertions
rubric: correctness
severity: should_fix
---

**Rule.** When recording draws or depth-only clip operations in Graphite, ensure
all draws have their scissor rects intersected and snapped to the device or
render-target dimensions before calling DrawContext::recordDraw. Do not weaken
or bypass scissor containment assertions (such as checking
SkIRect::MakeSize(...).contains(clip.scissor())) to accommodate un-clipped
bounds or depth-only draws; enforce valid, clamped scissor geometry in callers.

**Why.** Graphite requires that every recorded draw's scissor rect strictly lies
within the destination render pass or image dimensions. Weakening assertions for
depth-only or unpainted draws allows out-of-bounds scissor rects to reach
backend command buffers, triggering GPU validation failures or pipeline crashes.

## Examples

Bad:

```cpp
// DrawContext.cpp: Relaxing assertion for depth-only draws
SkASSERTF(paintID == UniquePaintParamsID::Invalid() ||
          SkIRect::MakeSize(this->imageInfo().dimensions()).contains(clip.scissor()),
          "clip scissor out of bounds");

// ClipStack.cpp: Passing raw or un-intersected bounds
this->drawClipImmediate(device, fOuterBounds);
```

Good:

```cpp
// DrawContext.cpp: Strictly asserting scissor containment for all draws
SkASSERTF(SkIRect::MakeSize(this->imageInfo().dimensions()).contains(clip.scissor()),
          "clip scissor out of bounds");

// ClipStack.cpp: Intersecting and snapping scissor to target dimensions
Rect snappedOuterBounds = snap_scissor(fOuterBounds, deviceBounds);
this->drawClipImmediate(device, snappedOuterBounds);
```
