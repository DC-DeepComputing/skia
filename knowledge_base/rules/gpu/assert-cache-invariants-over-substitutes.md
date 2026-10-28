---
name: skia/gpu/assert-cache-invariants-over-substitutes
description: When retrieving resources from GPU caches or lookup tables, assert expected cache invariants with SkASSERT instead of silently returning arbitrary substitute entries
rubric: correctness
severity: should_fix
---

**Rule.** Do not paper over missing, unallocated, or unsupported cache entries
by returning an arbitrary valid substitute resource to avoid crashes. Assert
expected cache invariants and non-null existence with SkASSERT, forcing upstream
callers to handle missing configurations or allocate resources correctly.

**Why.** Returning an arbitrary substitute (such as the first valid entry in an
array) prevents an immediate segfault or null pointer dereference but masks
upstream logic bugs and causes subtle rendering corruption, visual artifacts, or
state mismatches. Enforcing cache invariants via SkASSERT ensures invalid
lookups fail fast in debug builds and keeps cache contracts explicit.

## Examples

Bad:

```cpp
// If the target sampler is not found, substitute with the first valid sampler instead of
// returning nullptr, which results in a segfault.
if (!fDynamicSamplers[desc.desc()]) {
    SKGPU_LOG_W("Dynamic sampler not found in global cache, substituting!");
    for (uint32_t i = 0; i < kNumDynamicSamplers; ++i) {
        if (fDynamicSamplers[i]) {
            return fDynamicSamplers[i];
        }
    }
    SkASSERT(fDynamicSamplers[desc.desc()]);
}
return fDynamicSamplers[desc.desc()];
```

Good:

```cpp
SkASSERT(!desc.isImmutable() && desc.asSpan().size() == 1 && fDynamicSamplers[desc.desc()]);
return fDynamicSamplers[desc.desc()];
```
