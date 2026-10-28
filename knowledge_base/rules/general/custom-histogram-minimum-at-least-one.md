---
name: skia/custom-histogram-minimum-at-least-one
description: When defining custom UMA count or linear histograms with SK_HISTOGRAM_CUSTOM_* macros, specify a minimum value of at least 1 instead of 0
rubric: correctness
severity: nit
---

**Rule.** When defining custom UMA histograms using SK_HISTOGRAM_CUSTOM_COUNTS,
SK_HISTOGRAM_CUSTOM_EXACT_LINEAR, or SK_HISTOGRAM_CUSTOM_MICROSECONDS_TIMES, do
not set the minimum value (countMin, value_min, or minUSec) to 0; specify a
minimum of at least 1. Samples with values less than the minimum are
automatically captured by the underflow bucket (bucket 0).

**Why.** Chromium's underlying UMA histogram implementation reserves bucket 0 as
an implicit underflow bucket for samples smaller than the minimum value
(capturing 0 and unexpected negative values). Setting the minimum to 0 violates
Chromium UMA constraints and triggers debug runtime assertions.

## Examples

Bad:

```cpp
SK_HISTOGRAM_CUSTOM_COUNTS("Skia.Graphite.Precompile.NormalPreemptedByPrecompile",
                           fStats.fNormalPreemptedByPrecompile,
                           /* countMin= */ 0,
                           /* countMax= */ 50,
                           /* bucketCount= */ 50);
```

Good:

```cpp
SK_HISTOGRAM_CUSTOM_COUNTS("Skia.Graphite.Precompile.NormalPreemptedByPrecompile",
                           fStats.fNormalPreemptedByPrecompile,
                           /* countMin= */ 1,
                           /* countMax= */ 51,
                           /* bucketCount= */ 50);
```
