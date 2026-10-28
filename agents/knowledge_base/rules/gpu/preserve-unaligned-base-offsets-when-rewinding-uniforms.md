---
name: skia/gpu/preserve-unaligned-base-offsets-when-rewinding-uniforms
description: When rewinding shared base uniform storage across multi-phase steps, restore the base block's unaligned ending offset and alignment while tracking step-specific alignment separately
rubric: correctness
severity: should_fix
---

**Rule.** When rewinding uniform storage to reuse a shared base uniform block
across multiple steps (such as RenderSteps sharing a Paint in Graphite), record
and rewind to the base block's unaligned ending offset and required alignment.
Track any subsequent step's aligned start offset separately rather than padding
the base storage in-place, so that alignment requirements from one step do not
contaminate rewound state for subsequent steps.

**Why.** Shared base uniform data may be followed by steps with differing
alignment or shading requirements (such as shading RenderSteps that append
directly versus non-shading RenderSteps requiring separate alignment). Baking
step alignment into the base rewind mark or storage corrupts layout calculations
and prevents subsequent steps from correctly appending to the original base
uniform boundary.

## Examples

Bad:

```cpp
void UniformManager::markRewindOffset() {
    fRewindOffset = fStorage.size();
    fMarkedAlignment = fReqAlignment;
}
void UniformManager::alignForNonShading() {
    // Bad: In-place alignment alters base storage, preventing accurate rewind
    // to the unaligned paint boundary for subsequent steps.
    if (fReqAlignment) {
        this->alignTo(fReqAlignment);
    }
    fNonShadingOffset = fStorage.size();
}
```

Good:

```cpp
void UniformManager::markOffset() {
    fEndPaintOffset = fStorage.size();
    fEndPaintAlignment = fReqAlignment;
}
void UniformManager::alignForNonShading(int requiredAlignment) {
    // Good: Track aligned step offset separately without corrupting the base mark.
    this->alignTo(requiredAlignment);
    fNonShadingOffset = fStorage.size();
    fReqAlignment = std::max(fReqAlignment, requiredAlignment);
}
void UniformManager::rewindToMark() {
    fStorage.resize(fEndPaintOffset);
    fReqAlignment = fEndPaintAlignment;
    fNonShadingOffset = 0;
}
```
