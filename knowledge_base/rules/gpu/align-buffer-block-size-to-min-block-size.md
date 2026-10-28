---
name: skia/gpu/align-buffer-block-size-to-min-block-size
description: When sizing or allocating dynamic GPU buffer blocks for pooling, round sizes to coarse block granularity like minBlockSize instead of fine-grained binding alignment
rubric: safety
severity: should_fix
---

**Rule.** When sizing or allocating dynamic GPU buffer blocks, do not round the
block allocation size to the buffer's binding start or transfer alignment. Round
block allocations to a coarse block granularity (such as minBlockSize or a
dedicated block-size granularity constant) before clamping to maximum bounds.

**Why.** Buffer binding start alignments can be as small as 4 bytes (e.g., for
vertex or index buffers). Aligning dynamic block allocations to fine-grained
binding alignments generates arbitrary, irregular buffer sizes that defeat
buffer pooling and reuse in the GPU resource cache, increasing memory
fragmentation and driver allocation overhead.

## Examples

Bad:

```cpp
// Bad: Rounding block size to 4-byte binding alignment yields irregular block sizes
const uint32_t reqSize = SkAlignTo(info.fUsedSize / 4, info.fStartAlignment);
info.fCurBlockSize = std::clamp(reqSize, info.fMinBlockSize, info.fMaxBlockSize);
```

Good:

```cpp
// Good: Rounding block size to coarse minBlockSize ensures reusable pooled buffer chunks
const uint32_t reqSize = SkAlignTo(info.fUsedSize + info.fOffset, info.fMinBlockSize);
info.fCurBlockSize = std::min(reqSize, info.fMaxBlockSize);
```
