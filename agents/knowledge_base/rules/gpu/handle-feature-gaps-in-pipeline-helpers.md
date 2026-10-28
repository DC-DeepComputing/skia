---
name: skia/gpu/handle-feature-gaps-in-pipeline-helpers
description: When handling unsupported hardware or backend features in Graphite, adjust descriptors in higher-level pipeline helpers rather than substituting fallbacks in backend classes
rubric: design
severity: should_fix
---

**Rule.** Keep Graphite backends low-logic by resolving unsupported hardware or
API capabilities in higher-level pipeline and key helpers (such as KeyHelpers or
GlobalCache). Query Caps (such as clampToBorderSupport()) at the frontend or
cache boundary to substitute descriptors or fall back to shader emulation, and
assert in backend resource classes (such as Sampler or Texture) that unsupported
configurations are never requested.

**Why.** Graphite backend implementations are intentionally designed to be thin,
direct wrappers over underlying graphics APIs without hidden state mutations or
policy decisions. Quietly substituting fallback states or rewriting descriptors
inside backend classes obscures hardware limitations from shader pipeline keys,
prevents shader-based emulation from running, and leads to caching
inconsistencies.

## Examples

Bad:

```cpp
// In DawnSampler.cpp:
wgpu::AddressMode DawnSampler::TileModeToAddressMode(SkTileMode tileMode) {
    switch (tileMode) {
        case SkTileMode::kClamp:  return wgpu::AddressMode::ClampToEdge;
        case SkTileMode::kRepeat: return wgpu::AddressMode::Repeat;
        case SkTileMode::kMirror: return wgpu::AddressMode::MirrorRepeat;
        case SkTileMode::kDecal:
            // Dawn doesn't support decal; quietly substitute ClampToEdge.
            return wgpu::AddressMode::ClampToEdge;
    }
}
```

Good:

```cpp
// In KeyHelpers.cpp:
if (!caps->clampToBorderSupport() &&
    (tileMode.first == SkTileMode::kDecal || tileMode.second == SkTileMode::kDecal)) {
    // Substitute clamp and rely on shader emulation in higher-level pipeline.
    tileMode = {SkTileMode::kClamp, SkTileMode::kClamp};
}
SamplerDesc samplerDesc{sampling, tileMode};

// In DawnSampler.cpp:
case SkTileMode::kDecal:
    // Dawn doesn't support kDecal; invalid if requested.
    SkASSERT(false);
    return {};
```
