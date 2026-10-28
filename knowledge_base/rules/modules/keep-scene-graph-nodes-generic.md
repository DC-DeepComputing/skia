---
name: skia/modules/keep-scene-graph-nodes-generic
description: When implementing format-specific effects in Skottie or sksg, keep scene graph nodes generic using primitives like ExternalColorFilter and implement effects in Skottie adapters
rubric: design
severity: should_fix
---

**Rule.** Do not introduce domain- or format-specific effect nodes (such as
After Effects or Lottie feature filters) directly into the scene graph library
(sksg). Keep sksg nodes generic using minimal primitives (such as
sksg::ExternalColorFilter or sksg::ShaderEffect), and implement format-specific
property translation and lifecycle management in client adapters within
modules/skottie.

**Why.** The sksg library is designed as a minimal, format-agnostic
retained-mode scene graph. Baking animation- or client-format-specific effects
into sksg pollutes the core scene graph with high-level domain semantics,
whereas keeping nodes generic allows sksg to remain lean and reusable while
adapters handle client-specific animation logic.

## Examples

Bad:

```cpp
// modules/sksg/include/SkSGColorFilter.h
class LevelsColorFilter final : public ColorFilter {
public:
    static sk_sp<LevelsColorFilter> Make(sk_sp<RenderNode> child);
    SG_ATTRIBUTE(Channels, uint32_t, fChannels)
    SG_ATTRIBUTE(InBlack , float   , fInBlack )
    SG_ATTRIBUTE(InWhite , float   , fInWhite )
    ...
};
```

Good:

```cpp
// modules/sksg/include/SkSGColorFilter.h (generic primitive)
class ExternalColorFilter final : public EffectNode {
public:
    static sk_sp<ExternalColorFilter> Make(sk_sp<RenderNode> child);
    SG_ATTRIBUTE(ColorFilter, sk_sp<SkColorFilter>, fColorFilter)
};

// modules/skottie/src/effects/LevelsEffect.cpp (client adapter)
class LevelsEffectAdapter final : public DiscardableAdapterBase<LevelsEffectAdapter,
                                                               sksg::ExternalColorFilter> {
    ...
};
```
