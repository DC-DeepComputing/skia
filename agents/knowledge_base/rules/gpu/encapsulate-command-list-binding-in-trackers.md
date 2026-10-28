---
name: skia/gpu/encapsulate-command-list-binding-in-trackers
description: When designing GPU resource binding trackers in Graphite, encapsulate command-list binding logic inside the tracker rather than returning intermediate binding structs to callers
rubric: design
severity: should_fix
---

**Rule.** Encapsulate command-list emission and buffer-binding logic inside
resource binding tracker classes (such as UniformTracker or TextureTracker).
Update or write methods should return a boolean indicating whether rebinding is
required, and callers should invoke dedicated bind methods on the tracker that
emit directly to the command list rather than receiving intermediate binding
descriptor structs.

**Why.** Returning intermediate binding structures (such as BindBufferInfo)
forces callers to unpack them and handle low-level binding quirks—such as
zeroing out offsets for storage buffers versus uniform buffers—while the tracker
independently manages related state like SSBO indices. Encapsulating binding
emission on the tracker prevents state desynchronization, keeps caller draw
loops clean, and ensures buffer-binding invariants remain localized.

## Examples

Bad:

```cpp
// Bad: writeUniforms returns intermediate binding struct that caller must unpack and bind
const UniformTracker::BindingChange geomBindingChange =
        geometryUniformTracker.writeUniforms(uniformDataCache, bufferMgr, key.geometryUniformIndex());
if (geomBindingChange.fNeedsRebind) {
    BindBufferInfo binding = geomBindingChange.fBufferInfo;
    drawPass->fCommandList.bindUniformBuffer(binding, UniformSlot::kRenderStep);
}
```

Good:

```cpp
// Good: writeUniforms returns a boolean, and tracker directly emits to the command list
const bool geomBindingChange =
        geometryUniformTracker.writeUniforms(uniformDataCache, bufferMgr, key.geometryUniformIndex());
if (geomBindingChange) {
    geometryUniformTracker.bindUniforms(UniformSlot::kRenderStep, &drawPass->fCommandList);
}
```
