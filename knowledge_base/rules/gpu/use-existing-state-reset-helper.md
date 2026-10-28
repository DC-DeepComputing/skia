---
name: skia/gpu/use-existing-state-reset-helper
description: When resetting object or pipeline state on error or early-exit branches, call established local or member cleanup helpers rather than open-coding member assignments
rubric: readability
severity: should_fix
---

**Rule.** When exiting early or handling errors in functions managing stateful
components (such as command buffers or pipeline state), call established local
cleanup helpers or member reset methods instead of manually open-coding
repetitive member variable assignments. Reusing existing cleanup routines
prevents state desynchronization when fields are modified or added.

**Why.** Open-coding state cleanup duplicates logic across branches and risks
desynchronization if additional state fields are introduced to the helper later.
Invoking existing reset helpers guarantees uniform state across all exit paths
and reduces boilerplate.

## Examples

Bad:

```cpp
if (!texture || !sampler) {
    SKGPU_LOG_E("Invalid texture or sampler for dst-copy path.");
    fNumTextureSamplers = 0;
    fTextureSamplerDescSetToBind = VK_NULL_HANDLE;
    fBindTextureSamplers = false;
    return;
}
```

Good:

```cpp
if (!texture || !sampler) {
    SKGPU_LOG_E("Invalid texture or sampler for dst-copy path.");
    resetTextureAndSamplerState();
    return;
}
```
