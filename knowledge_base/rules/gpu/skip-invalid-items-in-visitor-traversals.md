---
name: skia/gpu/skip-invalid-items-in-visitor-traversals
description: When implementing visitor traversals like visitProxies or visitPipelines, skip invalid entries rather than aborting, and traverse child pass resources alongside direct task members
rubric: correctness
severity: should_fix
---

**Rule.** In boolean visitor traversal methods (such as visitProxies or
visitPipelines), skip invalid, null, or discarded entries rather than returning
false; abort only when the visitor callback itself evaluates to false.
Additionally, in composite tasks or containers holding sub-tasks or child passes
(such as RenderPassTask owning DrawPasses), traverse resources held by child
passes in addition to direct task-level members.

**Why.** Visitor traversals return false as an early-abort signal, so returning
false on invalid entries terminates traversal prematurely and skips valid items.
Omitting child passes or sub-tasks in composite visitors causes callers (such as
lazy proxy resolution, purge tracking, or pipeline pre-compilation) to miss
active GPU resource dependencies.

## Examples

Bad:

```cpp
// Variant 1: Aborting prematurely on invalid entries
bool UploadTask::visitProxies(const Visitor& visitor) override {
    for (int32_t i = 0; i < fInstances.size(); ++i) {
        if (!fInstances[i].isValid() || !visitor(fInstances[i].fTextureProxy.get())) {
            return false;
        }
    }
    return true;
}

// Variant 2: Omitting child pass resources in composite tasks
bool RenderPassTask::visitProxies(const Visitor& visitor) override {
    if (fDstCopy && !visitor(fDstCopy.get())) { return false; }
    if (fTarget && !visitor(fTarget.get())) { return false; }
    // Bug: fDrawPasses sampled textures never visited
    return true;
}
```

Good:

```cpp
// Variant 1: Skip invalid entries and only abort when visitor returns false
bool UploadTask::visitProxies(const Visitor& visitor) override {
    for (int32_t i = 0; i < fInstances.size(); ++i) {
        if (fInstances[i].isValid() && !visitor(fInstances[i].fTextureProxy.get())) {
            return false;
        }
    }
    return true;
}

// Variant 2: Traverse child pass resources alongside direct task members
bool RenderPassTask::visitProxies(const Visitor& visitor) override {
    for (const auto& pass : fDrawPasses) {
        for (const auto& proxy : pass->sampledTextures()) {
            if (!visitor(proxy.get())) { return false; }
        }
    }
    if (fDstCopy && !visitor(fDstCopy.get())) { return false; }
    if (fTarget && !visitor(fTarget.get())) { return false; }
    return true;
}
```
