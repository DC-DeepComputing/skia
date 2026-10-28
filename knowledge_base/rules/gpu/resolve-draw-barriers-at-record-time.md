---
name: skia/gpu/resolve-draw-barriers-at-record-time
description: When recording draws in Graphite, keep DrawList decoupled from Caps and DstReadStrategy, accumulating attachment properties unconditionally and resolving strategies in DrawContext
rubric: design
severity: should_fix
---

**Rule.** When recording draws in Graphite, keep DrawList decoupled from Caps
and backend execution strategies (such as DstReadStrategy) by resolving
capability-dependent requirements (such as BarrierType) in DrawContext before
passing them to DrawList::recordDraw, and unconditionally accumulating
attachment and renderer properties (such as depth/stencil flags, MSAA, and
dst-read bounds) on DrawList. Do not pass Caps or DstReadStrategy into DrawList,
defer barrier logic to snapDrawPass, or loop over draws in DrawPass::Make. Check
execution strategies alongside accumulated bounds in DrawContext at flush time
before scheduling strategy-specific tasks.

**Why.** DrawContext manages surface state, Caps, and execution strategies,
whereas DrawList is a low-level command collector. Resolving barriers and
checking execution strategies in DrawContext keeps DrawList decoupled and
strategy-agnostic, while unconditionally accumulating bounds and pass properties
during recording avoids redundant per-draw loops during pass snapping.

## Examples

Bad:

```cpp
// DrawList is coupled with Caps or DstReadStrategy:
DrawList::DrawList(const Caps* caps, DstReadStrategy strategy)
        : fSupportsHardwareAdvancedBlend(caps->supportsHardwareAdvancedBlending())
        , fStrategy(strategy) {}

void DrawList::recordDraw(..., DstUsage dstUsage, bool dstReadsFromInput) {
    BarrierType barrier = dstReadsFromInput && dstUsage.dstReadRequired()
            ? BarrierType::kReadDstFromInput : BarrierType::kNone;
    fDraws.emplace_back(..., barrier);
}

// DrawContext assumes non-empty bounds implies texture copy is needed:
if (!dstReadPixelBounds.isEmpty()) {
    fCurrentDrawTask->addTask(CopyTextureToTextureTask::Make(...));
}
```

Good:

```cpp
// DrawList is decoupled and accumulates properties unconditionally:
void DrawList::recordDraw(const Renderer* renderer, ..., BarrierType barrier) {
    fDepthStencilFlags |= renderer->depthStencilFlags();
    fRequiresMSAA |= renderer->requiresMSAA();
    fDstReadBounds.join(dstUsageBounds);
    fDraws.emplace_back(renderer, ..., barrier);
}

// DrawContext resolves barriers and checks strategies at flush time:
BarrierType barrier = (fDstReadStrategy == DstReadStrategy::kReadFromInput &&
                       (dstUsage & DstUsage::kDstReadRequired))
        ? BarrierType::kReadDstFromInput : BarrierType::kNone;
fPendingDraws->recordDraw(renderer, ..., barrier);

if (!dstReadPixelBounds.isEmpty() && fDstReadStrategy == DstReadStrategy::kTextureCopy) {
    fCurrentDrawTask->addTask(CopyTextureToTextureTask::Make(...));
}
```
