---
name: skia/gpu/match-indexed-instanced-draw-parameter-order
description: When calling or dispatching drawIndexedInstanced in Graphite, pass baseIndex and indexCount before baseVertex rather than matching non-indexed drawInstanced parameter order
rubric: correctness
severity: should_fix
---

**Rule.** When issuing or forwarding indexed instanced draw commands
(`drawIndexedInstanced`) in Graphite command lists or command buffers, verify
that argument positions follow the indexed signature `(type, baseIndex,
indexCount, baseVertex, baseInstance, instanceCount)`. Do not transpose
`baseVertex` into the second parameter slot by analogy with non-indexed
`drawInstanced`.

**Why.** In Graphite, index, vertex, and instance offset and count parameters
are untyped unsigned integers. Placing `baseVertex` before `indexCount` compiles
without warnings but silently swaps vertex and index offsets, corrupting
geometry fetch and index buffer lookups during GPU execution.

## Examples

Bad:

```cpp
// Transposing baseVertex into arg 2 slot by analogy with drawInstanced
fCommandList->drawIndexedInstanced(fPrimitiveType,
                                   fPendingBaseVertex,
                                   realIndexCount,
                                   0,
                                   fPendingBaseInstance,
                                   fPendingCount);
```

Good:

```cpp
// baseIndex and indexCount precede baseVertex
fCommandList->drawIndexedInstanced(fPrimitiveType,
                                   0,
                                   realIndexCount,
                                   fPendingBaseVertex,
                                   fPendingBaseInstance,
                                   fPendingCount);
```
