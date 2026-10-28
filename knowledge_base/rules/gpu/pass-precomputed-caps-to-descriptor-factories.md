---
name: skia/gpu/pass-precomputed-caps-to-descriptor-factories
description: When creating render descriptors from context objects, pass precomputed or cached Caps properties like DstReadStrategy into factory functions rather than redundantly re-querying Caps
rubric: design
severity: should_fix
---

**Rule.** When initializing render pass or pipeline descriptors from an owning
context (such as DrawContext), pass precomputed target capabilities or
strategies (such as DstReadStrategy) into descriptor factory functions (such as
RenderPassDesc::Make). Do not redundantly re-query Caps methods inside the
factory function when the context already caches the resolved property.

**Why.** Context objects already resolve and cache device and target
capabilities upon initialization. Forwarding precomputed properties avoids
duplicated Caps query logic and prevents potential desynchronization between
context-level state and descriptor configuration.

## Examples

Bad:

```cpp
// RenderPassDesc::Make re-queries Caps for a property already cached by DrawContext:
RenderPassDesc RenderPassDesc::Make(const Caps* caps,
                                    const TextureInfo& targetInfo,
                                    ...) {
    desc.fDstReadStrategy = caps->getDstReadStrategy(targetInfo);
    return desc;
}
```

Good:

```cpp
// DrawContext passes its cached strategy into RenderPassDesc::Make:
RenderPassDesc RenderPassDesc::Make(const Caps* caps,
                                    const TextureInfo& targetInfo,
                                    ...,
                                    DstReadStrategy dstReadStrategy) {
    SkASSERT(dstReadStrategy != DstReadStrategy::kNoneRequired);
    desc.fDstReadStrategy = dstReadStrategy;
    return desc;
}
```
