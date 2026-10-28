---
name: skia/gpu/issue-barriers-per-dst-read-draw
description: When recording GPU draw passes with destination reads, issue pipeline barriers before each individual draw reading dst rather than once per pass
rubric: correctness
severity: should_fix
---

**Rule.** When recording GPU draw or render passes where draws read from the
destination texture (such as via Vulkan input attachments or framebuffer fetch),
emit barrier commands before each individual draw that reads from the
destination rather than emitting a single barrier for the entire pass. Ensure
intra-pass draw synchronization guards every destination-reading draw or
pipeline flush to prevent read-after-write hazards from preceding draws.

**Why.** Multiple draws within a single render pass can modify the destination
texture. A barrier emitted only once at pass begin synchronizes work prior to
the pass but fails to protect against read-after-write hazards caused by earlier
draws within the same pass.

## Examples

Bad:

```cpp
// Emitting a single barrier outside the draw loop
if (dstReadStrategy == DstReadStrategy::kReadFromInput) {
    commandList->addBarrier(BarrierType::kReadDstFromInput);
}

for (const Draw& draw : draws) {
    drawWriter.draw(draw);
}
```

Good:

```cpp
// Emitting a barrier before each draw that reads from dst
for (const Draw& draw : draws) {
    if (dstReadStrategy == DstReadStrategy::kReadFromInput && draw.readsFromDst()) {
        commandList->addBarrier(BarrierType::kReadDstFromInput);
    }
    drawWriter.draw(draw);
}
```
