---
name: skia/avoid-temporary-containers-for-filtering-or-dedup
description: When filtering or deduplicating input arrays before copying to destination storage, avoid temporary intermediate containers by compacting in-place or pre-counting
rubric: safety
severity: should_fix
---

**Rule.** Do not allocate temporary intermediate arrays or vectors (such as
skia_private::TArray or std::vector) to filter or deduplicate input data prior
to copying into destination storage. Instead, allocate destination storage using
the input's upper-bound count and compact the surviving elements in-place with a
two-pointer pass, or pre-count surviving elements in an initial pass if exact
storage sizing is required.

**Why.** Allocating temporary heap containers solely to stage pruned, filtered,
or deduplicated data introduces unnecessary heap allocation churn and redundant
copy passes. Sizing destination storage to the worst-case upper bound and
compacting in-place eliminates all intermediate container allocations and extra
iteration steps.

## Examples

Bad:

```cpp
// Bad: Allocating temporary heap containers to filter/deduplicate before copying
auto inColors = skia_private::TArray<SkColor4f, true>();
auto inPositions = skia_private::TArray<SkScalar, true>();
// ... push to inColors and inPositions ...
fStorage.reset(inColors.size() * (sizeof(SkColor4f) + sizeof(SkScalar)));
for (int i = 0; i < inColors.size(); ++i) {
    colors[i] = inColors[i];
    positions[i] = inPositions[i];
}
```

Good:

```cpp
// Good: Allocate destination storage with upper-bound count and compact in-place
fStorage.reset(desc.fColorCount * (sizeof(SkColor4f) + sizeof(SkScalar)));
// ... populate initial fColors and fPositions ...
size_t dedupedCount = 0;
for (int i = 0; i < desc.fColorCount; ++i) {
    if (should_keep(i)) {
        fPositions[dedupedCount] = fPositions[i];
        fColors[dedupedCount] = fColors[i];
        dedupedCount++;
    }
}
fColorCount = dedupedCount;
```
