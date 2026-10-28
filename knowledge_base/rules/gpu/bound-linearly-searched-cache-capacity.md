---
name: skia/gpu/bound-linearly-searched-cache-capacity
description: When defining capacity limits for linearly searched caches or unindexed resource pools, keep the maximum conservative to prevent linear scan latency
rubric: safety
severity: should_fix
---

**Rule.** Keep the capacity limit of linearly searched caches, LRU lists, or
unindexed resource pools conservative (e.g., 8 to 32 entries rather than 100+).
Do not set large capacity thresholds on collections that require sequential
traversal for lookups or eviction checks without an aggregate or hash-based
index.

**Why.** In caches requiring linear traversal (such as intrusive LRU linked
lists or unindexed arrays), cache misses or near-misses must inspect every
element before falling back or allocating. Setting large capacities causes
worst-case O(N) scan latency on hot command recording paths, whereas small
conservative bounds preserve temporal locality while bounding scan overhead.

## Examples

Bad:

```cpp
// Bad: Large capacity limit on an unindexed linear-search buffer cache
static constexpr uint32_t kMaxNumBuffers = 128;
for (auto* curr = fBuffersLRU.head(); curr; curr = curr->fNext) {
    if (curr->matches(values)) return curr->info();
}
```

Good:

```cpp
// Good: Conservative capacity bounds worst-case O(N) scan latency on misses
static constexpr uint32_t kMaxNumBuffers = 16;
for (auto* curr = fBuffersLRU.head(); curr; curr = curr->fNext) {
    if (curr->matches(values)) return curr->info();
}
```
