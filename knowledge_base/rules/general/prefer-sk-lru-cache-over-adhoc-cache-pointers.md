---
name: skia/prefer-sk-lru-cache-over-adhoc-cache-pointers
description: When caching multiple configurations, defaults, or recent instances, use SkLRUCache with fixed capacity instead of ad-hoc singleton pointers and manual eviction
rubric: design
severity: should_fix
---

**Rule.** Prefer using a bounded cache container (such as SkLRUCache) with a
small fixed capacity over maintaining multiple ad-hoc singleton or global
pointers with hand-rolled eviction logic for special-case configurations (e.g.,
default, linear, or last-used). Retain cached entries using smart pointers (such
as sk_sp) or value types, and encapsulate lookup and eviction within the cache
rather than open-coding pointer reallocations.

**Why.** Propping up multiple ad-hoc pointers for individual special cases
requires manual, error-prone reference counting, lifetime tracking, and custom
eviction branching. A small SkLRUCache encapsulates eviction policies cleanly,
prevents memory leaks or dangling pointers across reallocations, and seamlessly
scales to multiple distinct configurations without custom branches.

## Examples

Bad:

```cpp
static SkMaskGamma* gLinearMaskGamma = nullptr;
static SkMaskGamma* gDefaultMaskGamma = nullptr;
static SkMaskGamma* gMaskGamma = nullptr;

if (is_linear) {
    if (!gLinearMaskGamma) { gLinearMaskGamma = new SkMaskGamma(...); }
    return *gLinearMaskGamma;
}
if (!gMaskGamma || gContrast != contrast || gGamma != gamma) {
    SkSafeUnref(gMaskGamma);
    gMaskGamma = new SkMaskGamma(...);
}
return *gMaskGamma;
```

Good:

```cpp
static constexpr int kMaxMaskGammaEntries = 4;
static SkLRUCache<MaskGammaCacheKey, sk_sp<SkMaskGamma>, MaskGammaCacheKeyHash>
        kMaskGammaCache{kMaxMaskGammaEntries};

if (sk_sp<SkMaskGamma>* entry = kMaskGammaCache.find(key)) {
    return **entry;
}
sk_sp<SkMaskGamma> gamma = SkMaskGamma::Make(...);
kMaskGammaCache.set(key, gamma);
return *gamma;
```
