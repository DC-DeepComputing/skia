---
name: skia/enumerate-pipeline-stages-in-overview-comments
description: When writing overview comments summarizing multi-stage execution pipelines or composite processes, enumerate all constituent stages and sub-pipelines in order without omitting intermediate steps
rubric: readability
severity: nit
---

**Rule.** When documenting a composite pipeline, multi-stage execution flow, or
chained processing sequence in overview comments, completely enumerate all
constituent stages and sub-pipelines in their actual execution order. Do not
skip intermediate phases (such as color pipelines, destination reading,
blending, coverage, or dithering) or describe an incomplete subset.

**Why.** Incomplete overview comments mislead readers about data flow and
intermediate dependencies across multi-stage pipelines. Fully enumerating
constituent sub-pipelines and stages in execution sequence ensures comments
accurately reflect architectural flow and clarifies where intermediate
transformations occur.

## Examples

Bad:

```cpp
// The full blit pipelines are each constructed lazily on first use, and include reading and
// writing the dst, coverage, dithering.
```

Good:

```cpp
// The full blit pipelines are each constructed lazily on first use, and include the color
// pipeline, reading the dst, the blend pipeline, coverage, dithering, and writing the dst.
```
