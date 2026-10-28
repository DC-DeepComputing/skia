---
name: skia/gpu/preserve-vertex-padding-across-pipeline-changes
description: When padding vertex buffer allocations across pipeline transitions to guard against speculative execution, unconditionally advance allocations to preserve zeroed padding bytes
rubric: correctness
severity: should_fix
---

**Rule.** When padding and zeroing vertex buffer allocations across pipeline
state transitions (such as to guard against speculative vertex execution),
unconditionally advance and preserve the zeroed padding bytes in the buffer
manager. Do not allow subsequent draws or pipeline states to overwrite the
padding, even if the subsequent pipeline appends vertices with the same stride.

**Why.** Speculative vertex execution hardware reads data according to the
current pipeline's vertex attribute layout. If subsequent draws overwrite the
zeroed padding with their own attribute data, differing attribute layouts
between pipelines can cause speculative execution to read invalid attribute
data, leading to GPU crashes or faults.

## Examples

Bad:

```cpp
void padAndZero(SkEnumBitMask<RenderStateFlags> newRenderState) {
    ...
    fVertexWriter.zeroBytes(byteDiff);
    // Skips advancing buffer allocation if next pipeline also appends vertices
    if (!(newRenderState & RenderStateFlags::kAppendVertices)) {
        fManager->getVertexWriter(byteDiff, 1, 1);
    }
}
```

Good:

```cpp
void padAndZero() {
    ...
    // Unconditionally advance buffer allocation to preserve zeroed padding
    fVertexWriter.zeroBytes(byteDiff);
    fManager->getVertexWriter(byteDiff, 1, 1);
}
```
