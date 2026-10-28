---
name: skia/gpu/flush-before-msaa-after-input-attachment-read
description: When tracking prior draw states to gate deferred flushes in Graphite, force flushes before MSAA transitions, preserve state across no-op draws, and reset on flush
rubric: correctness
severity: should_fix
---

**Rule.** When deciding whether to flush before drawing in deferred render pass
recording (such as in Device::needsFlushBeforeDraw), check if the incoming draw
requires MSAA and force a flush if preceding draws read destination textures as
input attachments (kReadFromInput). Additionally, when tracking sequential draw
state (such as fPriorDrawDstReadStrategy) across draws, do not overwrite the
tracked state on no-op or inactive operations (such as kNoneRequired), and
ensure the tracked state is explicitly reset when a flush occurs.

**Why.** Upgrading a render pass attachment to MSAA invalidates subpass loading
shaders recorded for earlier single-sampled draws. Furthermore, intermediate
draws that require no destination reads do not eliminate the synchronization
hazards of prior draws; overwriting tracked state with 'none' erases history and
causes subsequent incompatible draws to skip flushes, while failing to reset
after a flush allows stale state to leak into new passes.

## Examples

Bad:

```cpp
bool Device::needsFlushBeforeDraw(int numNewRenderSteps, DstReadStrategy dstReadStrategy) {
    bool needsFlush =
            (DrawList::kMaxRenderSteps - fDC->pendingRenderSteps()) < numNewRenderSteps ||
            dstReadStrategy == DstReadStrategy::kTextureCopy ||
            (fPriorDrawDstReadStrategy == DstReadStrategy::kTextureCopy &&
             dstReadStrategy == DstReadStrategy::kReadFromInput);
    // Bad: overwrites tracked prior strategy unconditionally on kNoneRequired draws.
    fPriorDrawDstReadStrategy = dstReadStrategy;
    return needsFlush;
}

void Device::flushPendingWorkToRecorder() {
    // Bad: tracked strategy is not reset across flush boundaries.
    fDC->flush();
}
```

Good:

```cpp
bool Device::needsFlushBeforeDraw(int numNewRenderSteps,
                                  DstReadStrategy dstReadStrategy,
                                  bool requiresMSAA) {
    bool needsFlush =
            (DrawList::kMaxRenderSteps - fDC->pendingRenderSteps()) < numNewRenderSteps ||
            dstReadStrategy == DstReadStrategy::kTextureCopy ||
            (fPriorDrawDstReadStrategy == DstReadStrategy::kTextureCopy &&
             dstReadStrategy == DstReadStrategy::kReadFromInput) ||
            (fPriorDrawDstReadStrategy == DstReadStrategy::kReadFromInput && requiresMSAA);
    // Good: preserve prior strategy across intermediate kNoneRequired draws.
    if (dstReadStrategy != DstReadStrategy::kNoneRequired) {
        fPriorDrawDstReadStrategy = dstReadStrategy;
    }
    return needsFlush;
}

void Device::flushPendingWorkToRecorder() {
    fDC->flush();
    fPriorDrawDstReadStrategy = DstReadStrategy::kNoneRequired;
}
```
