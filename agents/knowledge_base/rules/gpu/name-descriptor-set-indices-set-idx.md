---
name: skia/gpu/name-descriptor-set-indices-set-idx
description: When naming fields, constants, or variables for GPU descriptor set or bind group indices, include SetIdx or BindGroupIndex instead of generic Idx
rubric: readability
severity: nit
---

**Rule.** When declaring fields, constants, or variables representing GPU
resource descriptor set or bind group indices, include SetIdx, DescSetIndex, or
BindGroupIndex in the identifier rather than a generic Idx suffix. Reserve
Binding or Slot for binding indices within a set to clearly distinguish set
indices from binding slot indices.

**Why.** In modern GPU pipelines (such as Vulkan and WebGPU/Dawn), resources are
identified hierarchically by both a descriptor set or bind group index and a
binding slot index within that set (layout(set=X, binding=Y)). Naming a set
index with a generic Idx suffix (such as fUniformsIdx) leaves it ambiguous
whether the identifier represents the set index or a buffer/texture binding
slot.

## Examples

Bad:

```cpp
struct ResourceBindingRequirements {
    int fUniformsIdx = kUnassigned;
    int fTextureSamplerIdx = kUnassigned;
    int fRenderStepBufferBinding = kUnassigned;
};
```

Good:

```cpp
struct ResourceBindingRequirements {
    int fUniformsSetIdx = kUnassigned;
    int fTextureSamplerSetIdx = kUnassigned;
    int fRenderStepBufferBinding = kUnassigned;
};
```
