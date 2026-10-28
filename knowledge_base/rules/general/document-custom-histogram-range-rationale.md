---
name: skia/document-custom-histogram-range-rationale
description: When defining custom UMA histograms, document empirical range rationale and underflow/overflow expectations, and align units and bucket bounds across related metrics
rubric: testing
severity: should_fix
---

**Rule.** When defining custom histogram metrics (such as
SK_HISTOGRAM_CUSTOM_COUNTS, SK_HISTOGRAM_CUSTOM_EXACT_LINEAR, or
SK_HISTOGRAM_CUSTOM_MICROSECONDS_TIMES) with empirical bounds, document the
expected sample range and how overflow or underflow buckets will be interpreted,
explaining workload assumptions so readers recognize unexpected saturation.
Additionally, when defining groups of related telemetry histograms (e.g.
comparing feature benefits vs. costs), align underlying counters to use
consistent units and scales (such as unique instances rather than mixing
per-event usage with unique counts) so bucket ranges align predictably, or
document the rationale for any outlier range.

**Why.** Custom histogram bounds appear arbitrary without contextual comments
explaining operating envelopes and saturation limits. Mismatched counter units
across related metrics make direct comparisons difficult and distort bucket
distributions; aligning scales ensures bucket bounds align meaningfully while
documented outlier derivations prevent confusion during telemetry analysis.

## Examples

Bad:

```cpp
// Missing range rationale and underflow/overflow saturation interpretation:
SK_HISTOGRAM_CUSTOM_COUNTS("Skia.Graphite.Precompile.NormalPreemptedByPrecompile",
                           fStats.fNormalPreemptedByPrecompile,
                           /* countMin= */ 1,
                           /* countMax= */ 51,
                           /* bucketCount= */ 50);

// Mismatched counter scales (per-event usage vs. unique instances) across related metrics:
SK_HISTOGRAM_CUSTOM_COUNTS("Skia.Graphite.Precompile.NormalPreemptedByPrecompile",
                           fStats.fNormalPreemptedByPrecompile,
                           /* countMin= */ 1,
                           /* countMax= */ 6000,
                           /* bucketCount= */ 30);
SK_HISTOGRAM_CUSTOM_COUNTS("Skia.Graphite.Precompile.UnpreemptedPrecompilePipelines",
                           fStats.fUnpreemptedPrecompilePipelines,
                           /* countMin= */ 1,
                           /* countMax= */ 51,
                           /* bucketCount= */ 50);
```

Good:

```cpp
// Range rationale documented; counters aligned to unique pipelines with consistent bounds:
// From local testing we expect these stats to fit in the specified ranges.
// Frequent overflow or underflow signals unexpected pipeline saturation.
SK_HISTOGRAM_CUSTOM_COUNTS("Skia.Graphite.Precompile.NormalPreemptedByPrecompile",
                           fStats.fNormalPreemptedByPrecompile,
                           /* countMin= */ 1,
                           /* countMax= */ 51,
                           /* bucketCount= */ 50);
SK_HISTOGRAM_CUSTOM_COUNTS("Skia.Graphite.Precompile.UnpreemptedPrecompilePipelines",
                           fStats.fUnpreemptedPrecompilePipelines,
                           /* countMin= */ 1,
                           /* countMax= */ 51,
                           /* bucketCount= */ 50);
```
