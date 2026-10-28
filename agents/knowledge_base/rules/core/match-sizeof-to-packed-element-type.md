---
name: skia/core/match-sizeof-to-packed-element-type
description: When computing buffer capacity for packing multiple variable-length arrays, verify each element count is multiplied by the sizeof its exact element type
rubric: correctness
severity: blocking
---

**Rule.** When calculating total buffer capacity or memory footprint for packing
multiple arrays into contiguous storage, multiply each element count by the
exact sizeof of its corresponding element type. Do not copy-paste sizeof
expressions from adjacent fields or assume adjacent arrays share identical
element sizes.

**Why.** Copy-pasting sizeof across distinct array fields causes buffer capacity
mismatches. Under-allocating causes out-of-bounds writes or partition
misalignment when carving the buffer into typed spans, while over-allocating
wastes memory.

## Examples

Bad:

```cpp
// Bug: conic weights are float (4 bytes), not SkPoint (8 bytes).
accum.addMul(pointCount, sizeof(SkPoint))
     .addMul(conicCount, sizeof(SkPoint))
     .addMul(verbCount, sizeof(SkPathVerb));
```

Good:

```cpp
accum.addMul(pointCount, sizeof(SkPoint))
     .addMul(conicCount, sizeof(float))
     .addMul(verbCount, sizeof(SkPathVerb));
```
