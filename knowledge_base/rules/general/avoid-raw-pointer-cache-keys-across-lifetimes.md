---
name: skia/avoid-raw-pointer-cache-keys-across-lifetimes
description: When implementing caches or lookup maps, store owning smart pointers (sk_sp, unique_ptr) or persistent IDs instead of bare raw pointers for keys or values
rubric: safety
severity: blocking
---

**Rule.** Do not store bare raw pointers as cache keys or values in lookup
containers that outlive referenced objects. For cached entries, retain ownership
using smart pointers (such as sk_sp<T> for ref-counted objects or
std::unique_ptr<T>) rather than unowned raw pointers (T*), ensuring entries
remain valid independently of external callers. For cache keys, key by an
immutable unique ID (such as uniqueID()) or retain strong references to prevent
address recycling collisions.

**Why.** If a cache holds unowned raw pointers to ref-counted objects, entries
deallocate when external callers release references, leaving dangling pointers
that trigger use-after-free bugs on subsequent lookups. For cache keys, when a
freed object's memory address is recycled by the allocator, bare pointer keys
cause false cache hits that return stale or corrupted data.

## Examples

Bad:

```cpp
// Key hazard: Bare pointer key risks false hits from allocator address recycling:
skia_private::THashMap<const SkGradientBaseShader*, int> fGradientOffsetCache;

// Value hazard: Bare raw pointer leaves dangling entries when external callers drop references:
SkLRUCache<MaskGammaCacheKey, SkMaskGamma*, MaskGammaCacheKeyHash> kMaskGammaCache{kMaxEntries};
```

Good:

```cpp
// Key by persistent unique ID to prevent address reuse collisions:
skia_private::THashMap<uint32_t, int> fGradientOffsetCache;

// Store owning smart pointers so cached values remain valid independently of callers:
SkLRUCache<MaskGammaCacheKey, sk_sp<SkMaskGamma>, MaskGammaCacheKeyHash> kMaskGammaCache{kMaxEntries};
```
