---
name: skia/gpu/remove-stale-key-sizing-comments
description: When defining or refactoring cache maps, lookup tables, or GPU resource key builders, keep comments synchronized with actual key types, layouts, and deduplication semantics
rubric: readability
severity: should_fix
---

**Rule.** When defining or refactoring cache keys, lookup maps, or GPU resource
key builders (such as THashMap, THashSet, UniqueKey::Builder, or
GraphiteResourceKey::Builder), keep accompanying comments describing key
representation, layout, slot breakdowns, and capacity synchronized with actual
implementation semantics. Update comments when migrating from pointer/address
deduplication to unique IDs or hashes so comments never claim a map is keyed by
memory address when an integer ID is used. Additionally, when migrating from
dynamic sizing to fixed capacity or repackaging key fields, remove obsolete
variable-length notes and update field type annotations so word counts match
actual slot widths.

**Why.** Inaccurate comments that describe stale key types (such as claiming a
map is keyed by memory address when using unique IDs) mislead readers about
object lifetime requirements, equality, and deduplication semantics. Similarly,
stale layout comments describing obsolete variable-length allocations or
outdated field widths contradict key packing logic and create confusion when
maintaining GPU pipeline or resource caches.

## Examples

Bad:

```cpp
// Key type refactoring:
// We use the shader's address as a key to de-duplicate gradient data.
skia_private::THashMap<uint32_t, int> fGradientOffsetCache;

// Sizing refactoring:
// The uint32s needed for a RenderPass is variable number, so consult rpMetaData to reserve.
UniqueKey::Builder builder(&pipelineKey, get_domain(), kPipelineKeyData32Count, "GraphicsPipeline");

// Field width / slot count refactoring:
// 4 uint32_t's (render step id, paint id, uint64 renderpass desc, uint16 write swizzle key)
UniqueKey::Builder builder(&pipelineKey, get_domain(), kMtlGraphicsPipelineKeyData32Count, "MtlGraphicsPipeline");
```

Good:

```cpp
// Key type refactoring:
// We use the shader's unique ID as a key to de-duplicate gradient data.
skia_private::THashMap<uint32_t, int> fGradientOffsetCache;

// Sizing refactoring:
UniqueKey::Builder builder(&pipelineKey, get_domain(), kPipelineKeyData32Count, "GraphicsPipeline");

// Field width / slot count refactoring:
// 4 uint32_t's (render step id, paint id, renderpass desc, uint16 write swizzle key)
UniqueKey::Builder builder(&pipelineKey, get_domain(), kMtlGraphicsPipelineKeyData32Count, "MtlGraphicsPipeline");
```
