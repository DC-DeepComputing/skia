---
name: skia/gpu/traverse-tasks-to-inspect-resources
description: When inspecting or aggregating resources used by recorded tasks in Graphite, traverse the TaskList and Task hierarchy via visitor methods instead of querying external caches
rubric: design
severity: should_fix
---

**Rule.** When inspecting, collecting, or aggregating GPU resources (such as
TextureProxy or GraphicsPipeline) used by recorded tasks, traverse the task
hierarchy using visitor methods on TaskList and Task (such as visitProxies or
visitPipelines). Do not query external caches or sidecar data structures on
Recorder or DrawContext to discover resources associated with recorded tasks.

**Why.** In Graphite, tasks own draw commands and their associated GPU
resources. External caches may contain stale, unreferenced, or deduplicated
entries that do not match the actual execution graph, whereas traversing the
task hierarchy visits exactly the active dependencies of the snapped recording.

## Examples

Bad:

```cpp
// In Recorder::snap(), querying an external cache to discover lazy proxies:
fTextureDataCache->foreach([&](TextureDataBlock block) {
    for (int j = 0; j < block.numTextures(); ++j) {
        const TextureDataBlock::SampledTexture& tex = block.texture(j);
        if (tex.first->isLazy()) {
            lazyProxies.insert(tex.first);
        }
    }
});
```

Good:

```cpp
// Traverse the task hierarchy via TaskList visitor methods:
fRootTaskList->visitProxies([&](const TextureProxy* proxy) {
    if (proxy->isLazy()) {
        lazyProxies.insert(sk_ref_sp(proxy));
    }
    return true;
});
```
