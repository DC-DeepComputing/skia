---
name: skia/core/provide-generous-headroom-for-reserved-key-ranges
description: When reserving contiguous ID blocks or ranges for stable cache keys, allocate generous headroom beyond current counts to prevent shifting downstream keys and cache invalidation
rubric: safety
severity: should_fix
---

**Rule.** When partitioning contiguous numeric ID spaces or reserving fixed ID
blocks for stable cache keys (such as known runtime effects or precompiled
shader snippets), allocate generous capacity headroom beyond current counts. Do
not set reservation limits close to existing element counts where adding new
items would alter subsequent range base offsets and invalidate persistent
caches.

**Why.** In contiguous partitioned ID spaces, expanding a block pushes all
subsequent block start offsets. Sizing reservations too tightly forces
downstream IDs to shift whenever new entries are added, invalidating previously
compiled pipelines, shaders, or persistent cached keys.

## Examples

Bad:

```cpp
// Reserving tight capacity (~57 known effects exist):
static constexpr int kSkiaBuiltInReservedCnt = 100;
static constexpr int kSkiaKnownRuntimeEffectsReservedCnt = 100;
static constexpr int kSkiaKnownRuntimeEffectsStart = kSkiaBuiltInReservedCnt;
```

Good:

```cpp
// Provide generous headroom so additions do not shift downstream key ranges:
static constexpr int kSkiaBuiltInReservedCnt = 500;
static constexpr int kSkiaKnownRuntimeEffectsReservedCnt = 500;
static constexpr int kSkiaKnownRuntimeEffectsStart = kSkiaBuiltInReservedCnt;
```
