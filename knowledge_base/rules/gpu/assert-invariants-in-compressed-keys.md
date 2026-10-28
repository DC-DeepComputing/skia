---
name: skia/gpu/assert-invariants-in-compressed-keys
description: When compressing GPU resource or cache keys by omitting invariant fields or compatibility attachments, document the omitted state and assert assumed invariants with SkASSERT
rubric: readability
severity: should_fix
---

**Rule.** When compressing GPU resource or cache keys by omitting fixed
descriptor fields or assuming constant property values, explicitly verify those
assumed invariants using SkASSERT on the input descriptor; do not rely solely on
code comments to document omitted fields. Additionally, when attachments or
descriptor fields are omitted from bit-packed keys because they are fixed or
unconditionally included across all instances for compatibility (such as input
attachments for dst reads), document this architectural invariant in the key
comment.

**Why.** Omitting fields packs descriptors into smaller keys, but unasserted
assumptions leave cache generation vulnerable to silent key collisions and cache
corruption if upstream caller configurations change. Furthermore, omitting
constant or fixed compatibility attachments without explanatory comments
confuses maintainers inspecting key layouts as to why expected attachments are
missing from the key.

## Examples

Bad:

```cpp
// 1. Missing runtime assertion when omitting assumed constant descriptor fields:
// Depth/stencil load op is always kClear, and store op is always kDiscard.
uint32_t key = (static_cast<uint32_t>(depthStencil.fFormat) << kFormatShift) |
               (depthStencil.fSampleCount << kSampleShift);

// 2. Missing documentation for omitted constant compatibility attachments:
// The render pass always has a color attachment, and maybe a depth-stencil attachment.
// In truth, resolve attachments do not affect the pipeline key.
```

Good:

```cpp
// 1. Assert assumed constants before omitting them from the packed key:
SkASSERT(depthStencil.fFormat == TextureFormat::kUnsupported ||
         depthStencil.fLoadOp == LoadOp::kClear);
SkASSERT(depthStencil.fFormat == TextureFormat::kUnsupported ||
         depthStencil.fStoreOp == StoreOp::kDiscard);
uint32_t key = (static_cast<uint32_t>(depthStencil.fFormat) << kFormatShift) |
               (depthStencil.fSampleCount << kSampleShift);

// 2. Document constant compatibility attachments omitted from the key layout:
// The render pass always has a color attachment, and maybe a depth-stencil attachment. An input
// attachment is always provided for compatibility purposes (even if not required).
```
