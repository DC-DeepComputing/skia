---
name: skia/sksl/divide-slot-offset-by-vector-stride
description: When converting pointer offsets to logical slot indices in SkRasterPipelineBuilder slot buffers, divide raw element offsets by vector stride N
rubric: correctness
severity: should_fix
---

**Rule.** When converting a raw pointer offset into a logical slot index in
SkRasterPipelineBuilder slot buffers (such as values or stack), divide the
element displacement by the SIMD vector stride N
(SkOpts::raster_pipeline_highp_stride). Verify that the displacement is a
multiple of N with an assertion before computing the index.

**Why.** In SkRasterPipelineBuilder, slot buffers allocate N contiguous elements
per logical slot to accommodate SIMD vector execution lanes. Subtracting buffer
base pointers yields an element lane displacement rather than a logical slot
index; omitting division by N calculates incorrect slot indices whenever N > 1,
a defect that passes silently when test tools like skslc run with scalar stride
1.

## Examples

Bad:

```cpp
int index = ptr - fSlots.values.data();
return 'v' + this->asRange(index, numSlots);
```

Good:

```cpp
int index = ptr - fSlots.values.data();
SkASSERT((index % N) == 0);
return 'v' + this->asRange(index / N, numSlots);
```
