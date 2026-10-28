---
name: skia/gpu/purge-refcounted-cache-entries-without-unique-check
description: When purging ref-counted cache resources by access time, do not check unique(); evict based solely on expiration time so active users retain safe ownership
rubric: correctness
severity: should_fix
---

**Rule.** When purging ref-counted resources (such as sk_sp<GraphicsPipeline>)
from a cache by access time, do not require the resource to be uniquely
referenced (unique()). Evict expired entries based solely on access time,
relying on smart pointer ownership to safely keep resources alive while outside
consumers hold active references.

**Why.** Evicting an entry from a cache drops the cache's owning reference
without destroying the underlying object if in-flight consumers (such as active
Recordings) still hold references. Requiring unique() prevents evicting stale
cache entries that have transient in-flight users and unnecessarily couples
cache eviction to internal reference counting.

## Examples

Bad:

```cpp
if ((*pipeline)->unique() && (*pipeline)->lastAccessTime() < purgeTime) {
    toRemove.push_back(*key);
}
```

Good:

```cpp
if ((*pipeline)->lastAccessTime() < purgeTime) {
    toRemove.push_back(*key);
}
```
