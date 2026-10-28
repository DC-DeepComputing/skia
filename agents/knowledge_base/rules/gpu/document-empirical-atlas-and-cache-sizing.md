---
name: skia/gpu/document-empirical-atlas-and-cache-sizing
description: When configuring texture atlases or GPU caches with non-standard, reduced, or empirically tuned dimensions, document the motivating workload characteristics or benchmarks in an inline comment
rubric: readability
severity: nit
---

**Rule.** When initializing texture atlases (such as DrawAtlas or DrawAtlasMgr)
or GPU resource caches with non-standard, scaled-down, or empirically tuned
dimensions and plot subdivisions, add an inline comment justifying the sizing
choice. Explicitly cite the workload characteristics, empirical profiling
observations (such as typical asset sizes or low frequency in target workloads),
or benchmark trade-offs motivating the non-default dimensions.

**Why.** Non-standard, fractional, or scaled-down dimensions look like arbitrary
magic numbers or potential copy-paste bugs to reviewers and maintainers. Inline
documentation explaining empirical workload distributions or performance
trade-offs clarifies why smaller or custom bounds were chosen and prevents
future refactorings from unintentionally resizing or standardizing the cache.

## Examples

Bad:

```cpp
// Bad: Undocumented reduced atlas dimensions appear to be arbitrary magic numbers
fSaveRecordKeyAtlasMgr(kClipAtlasWidth / 2, kClipAtlasHeight / 2,
                       /*plotWidth=*/kClipAtlasWidth / 2, /*plotHeight=*/kClipAtlasHeight / 2,
                       DrawAtlas::UseStorageTextures::kNo,
                       "SaveRecordKeyClipAtlas", recorder->priv().caps())
```

Good:

```cpp
// Good: Explains empirical workload distribution justifying the smaller atlas
// Examining results across common web pages, SaveRecord keyed clips tend to be
// considerably smaller and rarer, so we use a smaller atlas here.
fSaveRecordKeyAtlasMgr(kClipAtlasWidth / 2, kClipAtlasHeight / 2,
                       /*plotWidth=*/kClipAtlasWidth / 2, /*plotHeight=*/kClipAtlasHeight / 2,
                       DrawAtlas::UseStorageTextures::kNo,
                       "SaveRecordKeyClipAtlas", recorder->priv().caps())
```
