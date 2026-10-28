---
name: skia/api/document-raw-pointer-table-ownership
description: When declaring non-owning lookup tables or arrays of raw pointers, document the lifetime owner and indexing scheme in an explanatory comment
rubric: readability
severity: nit
---

**Rule.** When declaring non-owning lookup tables or fixed arrays of raw
pointers, document where the referenced objects are owned (such as a companion
owning container or resource cache) and how elements are indexed (such as by
descriptor bitmasks, flags, or enum combinations). Clearly distinguish the
non-owning index from the backing storage in class header comments.

**Why.** Raw pointer lookup tables and arrays do not encode lifetime, ownership,
or indexing guarantees in their C++ types. Documenting where ownership resides
(such as in an owning resource pool or companion array) and how indices are
derived (such as from descriptor bitmasks or flags) prevents memory safety
issues and indexing misuse during maintenance.

## Examples

Bad:

```cpp
// Bad: Undocumented raw pointer table leaves ownership and index mapping unclear.
std::array<const Sampler*, kNumDynamicSamplers> fDynamicSamplers SK_GUARDED_BY(fSpinLock);
```

Good:

```cpp
// Good: Clarifies object ownership and bitmask-to-index mapping.
// Lookup table for dynamically created samplers. Each sampler represents a specific
// combination of tile modes and sampling options, indexed by a bitmask generated from
// those properties. The actual Sampler objects are owned by `fStaticResource`.
std::array<const Sampler*, kNumDynamicSamplers> fDynamicSamplers SK_GUARDED_BY(fSpinLock);
```
