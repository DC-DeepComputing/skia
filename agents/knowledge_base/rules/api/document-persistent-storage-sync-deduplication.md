---
name: skia/api/document-persistent-storage-sync-deduplication
description: When declaring public API methods for persistent pipeline cache synchronization or queries, document heuristic change-detection semantics, false positives, deduplication, and backend fallbacks
rubric: readability
severity: should_fix
---

**Rule.** When declaring or updating public API methods and interfaces for
persistent cache synchronization or change queries (such as `syncPipelineData`,
`hasNewVkPipelineCacheData`, or `PersistentPipelineStorage`), accurately
document change-detection behavior. Clarify whether the engine deduplicates
store calls or makes best-effort attempts to avoid redundant writes, and
explicitly document heuristic guarantees, potential false positives, and
fallback semantics across backends or extension availability.

**Why.** Clients rely on public header contracts to determine whether to
implement external hashing/caching layers or how to react to cache queries.
Inaccurate or oversimplified documentation misleads consumers into assuming
query returns are exact rather than heuristic, or into introducing redundant
deduplication layers around expensive disk I/O.

## Examples

Bad:

```cpp
// Persistent storage sync method:
/**
 * Stores the current pipeline cache data into the PersistentPipelineStorage
 * object passed via ContextOptions.
 */
void syncPipelineData(size_t maxSize = SIZE_MAX);

// Change-detection query method:
/**
 * Returns true if the data in the pipeline cache has changed since the last call to
 * `storeVkPipelineCacheData`. Always returns true when `canDetectNewVkPipelineCacheData` is false.
 */
bool hasNewVkPipelineCacheData() const;
```

Good:

```cpp
// Persistent storage sync method:
/**
 * Stores the current pipeline cache data into the PersistentPipelineStorage
 * object passed via ContextOptions. Skia attempts to only call store() when
 * the data is likely to be different from what was last sync'ed.
 */
void syncPipelineData(size_t maxSize = SIZE_MAX);

// Change-detection query method:
/**
 * For Vulkan implementations, returns true if pipeline cache data could have changed
 * since `storeVkPipelineCacheData`. Always returns true on non-Vulkan implementations.
 * Pipeline creation occasionally results in false positives; VK_EXT_pipeline_creation_cache_control
 * eliminates false positives when enabled.
 */
bool hasNewVkPipelineCacheData() const;
```
