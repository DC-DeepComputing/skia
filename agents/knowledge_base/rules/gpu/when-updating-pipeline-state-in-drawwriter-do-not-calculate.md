---
name: skia/gpu/when-updating-pipeline-state-in-drawwriter-do-not-calculate
description: When updating pipeline state in DrawWriter, do not calculate or carry over pending base offsets in newPipelineState; centralize offset derivations at flush time
rubric: design
severity: should_fix
---

**Rule.** In draw batching managers like DrawWriter, do not calculate or carry
over pending base offsets across pipeline state changes in newPipelineState().
Because pipeline transitions already flush pending accumulated draw data,
centralize base offset derivations where draw buffers are bound or flushed.

**Why.** Calculating and realigning pending base offsets across pipelines in
newPipelineState() duplicates buffer management logic and fragments state
tracking across lifecycle methods. Since pipeline switches flush pending draws,
deriving base offsets at flush or bind time keeps state transitions predictable
and avoids stale or desynchronized offset tracking.

## Examples

Bad:

```cpp
void newPipelineState(PrimitiveType type, size_t vertexStride, size_t instanceStride,
                      SkEnumBitMask<RenderStateFlags> renderState) {
    this->flush();
    auto [prevStride, prevBase] = (fRenderState & RenderStateFlags::kAppendVertices)
        ? std::make_pair(fVertexStride, &fPendingBaseVertex)
        : std::make_pair(fInstanceStride, &fPendingBaseInstance);
    auto [currStride, currBase] = (renderState & RenderStateFlags::kAppendVertices)
        ? std::make_pair(vertexStride, &fPendingBaseVertex)
        : std::make_pair(instanceStride, &fPendingBaseInstance);
    if (prevStride && currStride) {
        const size_t prevBytes = *prevBase * prevStride;
        *currBase = SkTo<uint32_t>((prevBytes + currStride - 1) / currStride);
    } else {
        *currBase = 0;
    }
}
```

Good:

```cpp
void newPipelineState(PrimitiveType type, size_t vertexStride, size_t instanceStride,
                      SkEnumBitMask<RenderStateFlags> newRenderState) {
    this->flush();
    SkASSERT(fPendingCount == 0);
    SkASSERT(fTemplateCount == 0);

    fPrimitiveType = type;
    fVertexStride = vertexStride;
    fInstanceStride = instanceStride;
    fRenderState = newRenderState;
}
```
