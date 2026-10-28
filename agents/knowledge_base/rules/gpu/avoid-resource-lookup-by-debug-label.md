---
name: skia/gpu/avoid-resource-lookup-by-debug-label
description: When retrieving or identifying GPU resources in caches or test helpers, do not search by debug label strings; retain or pass direct object pointers
rubric: design
severity: should_fix
---

**Rule.** Do not search for or identify GPU resources, buffers, or cache entries
by matching their debug label strings (such as getLabel() or label()). Instead,
retain and pass direct object pointers (e.g., sk_sp<Resource> or sk_sp<Buffer>)
from where the resource is created or held.

**Why.** Resource debug labels are intended solely for GPU debug markers (such
as Metal, Direct3D, or Vulkan labels), Tracy profiling, and memory dumps, not
object identification or retrieval. Matching on debug label strings is fragile,
incurs string comparison overhead, and tightly couples caller logic to debug
names that may change or be stripped in release builds.

## Examples

Bad:

```cpp
// Searching a resource list by debug label string:
for (const auto& r : fStaticResource) {
    if (r->getLabel() == "StaticVertexBuffer") {
        return sk_ref_sp(static_cast<skgpu::graphite::Buffer*>(r.get()));
    }
}
return nullptr;
```

Good:

```cpp
// Caller passes the direct buffer handle rather than searching by label:
globalCache->testingOnly_SetStaticVertexInfo(
        copyRanges,
        sk_ref_sp(fVertexBufferInfo.fData[0].fTarget->fBuffer));
```
