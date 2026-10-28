---
name: skia/general-qualify-stats-structs-by-resource-type
description: When declaring statistics or metrics structs in multi-resource managers or caches, qualify struct names and counter fields with the specific resource type
rubric: readability
severity: nit
---

**Rule.** When declaring statistics, metrics, or diagnostic structs and counter
fields in a class that manages or caches multiple distinct resource types,
qualify the struct name and its member variables with the specific resource type
being measured (e.g., PipelineStats with fGraphicsCacheHits) rather than using
generic names like Stats or fCacheHits.

**Why.** In classes that manage heterogeneous resources or multiple pipeline
types, generic names like Stats or fCacheHits are ambiguous about which asset
type is being measured and prevent clean additions of metrics for other managed
resources in the future.

## Examples

Bad:

```cpp
// In a class managing multiple resources (pipelines, shaders, buffers):
struct Stats {
    int fCacheHits = 0;
    int fCacheMisses = 0;
    int fCacheAdditions = 0;
    int fRaces = 0;
};
Stats getStats() const;
```

Good:

```cpp
// In a class managing multiple resources (pipelines, shaders, buffers):
struct PipelineStats {
    int fGraphicsCacheHits = 0;
    int fGraphicsCacheMisses = 0;
    int fGraphicsCacheAdditions = 0;
    int fGraphicsRaces = 0;
};
PipelineStats getStats() const;
```
