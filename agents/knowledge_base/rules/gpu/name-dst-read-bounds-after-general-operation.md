---
name: skia/gpu/name-dst-read-bounds-after-general-operation
description: When tracking bounds or requirements for GPU destination reads, name variables, fields, and parameters after the general operation (dstReadBounds) rather than a specific strategy (dstCopyBounds)
rubric: readability
severity: should_fix
---

**Rule.** Do not name variables, fields, parameters, or methods after a specific
implementation strategy (such as dstCopyBounds, fDstCopyBounds, or
dstCopyPixelBounds) when the underlying state represents a general requirement
shared across multiple destination-reading strategies. Name them using general
operation terminology (such as dstReadBounds, fDstReadBounds, or
dstReadPixelBounds).

**Why.** Destination target reads during rendering can be fulfilled through
multiple strategies, including texture copies, framebuffer fetch, direct texture
sampling, and input attachments. Naming tracked bounds or local rects after a
specific execution mechanism (such as copy) is misleading and inaccurate when
the bounds are tracked whenever any draw requires a destination read, and are
passed or inspected regardless of which strategy executes.

## Examples

Bad:

```cpp
// In DrawList.h:
Rect fDstCopyBounds = Rect::InfiniteInverted();
const Rect& dstCopyBounds() const { return fDstCopyBounds; }

// In CommandBuffer.h:
bool addRenderPass(..., const Texture* dstCopy, SkIRect dstCopyBounds, ...);

// In DrawContext.cpp:
SkIRect dstCopyPixelBounds = fPendingDraws->dstReadBounds().makeRoundOut().asSkIRect();
```

Good:

```cpp
// In DrawList.h:
Rect fDstReadBounds = Rect::InfiniteInverted();
const Rect& dstReadBounds() const { return fDstReadBounds; }

// In CommandBuffer.h:
bool addRenderPass(..., const Texture* dstCopy, SkIRect dstReadBounds, ...);

// In DrawContext.cpp:
SkIRect dstReadPixelBounds = fPendingDraws->dstReadBounds().makeRoundOut().asSkIRect();
```
