---
name: skia/gpu/omit-unused-resource-from-caps-queries
description: When defining backend capability queries on Caps, omit per-resource parameters like TextureInfo when decisions are static or dynamic fallbacks are handled by passes
rubric: design
severity: should_fix
---

**Rule.** Do not pass per-resource metadata or descriptor parameters (such as
TextureInfo or TextureProxy) to Caps capability query methods when the
underlying strategy is static or device-wide. When dynamic decisions or
fallbacks (such as switching strategies for MSAA) are resolved by higher-level
draw or pass management, keep Caps query methods parameterless to avoid
unnecessary coupling across callers, precompilation, and serialization.

**Why.** Passing per-resource parameters to static capability queries forces
callers, precompilation helpers, and serialization utilities to construct or
forward irrelevant resource descriptors. Keeping static capability queries
parameterless decouples device capabilities from specific render targets and
centralizes dynamic fallback heuristics in higher-level draw and pass managers.

## Examples

Bad:

```cpp
// Caps.h
virtual DstReadStrategy getDstReadStrategy(const TextureInfo&) const;

// DrawContext.cpp
fDstReadStrategy = caps->getDstReadStrategy(fTarget->textureInfo());
```

Good:

```cpp
// Caps.h
virtual DstReadStrategy getDstReadStrategy() const;

// DrawContext.cpp
fDstReadStrategy = caps->getDstReadStrategy();
```
