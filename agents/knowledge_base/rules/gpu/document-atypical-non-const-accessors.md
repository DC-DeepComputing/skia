---
name: skia/gpu/document-atypical-non-const-accessors
description: When exposing atypical non-const accessors for shared or normally const-held objects in ResourceProviders, document the mutating operations and requirements that necessitate non-const access
rubric: readability
severity: nit
---

**Rule.** When exposing an atypical non-const accessor for a shared or normally
const-held object (such as a SharedContext in a backend ResourceProvider),
document the specific mutating operation and subsystem requirement (e.g.,
pipeline cache persistence updates) that necessitates non-const access. Clarify
why the object cannot be held or accessed via standard const references.

**Why.** In Graphite, ResourceProvider implementations generally treat
SharedContext as const to respect immutability and thread-safety invariants
across context operations. Exposing an atypical non-const accessor breaks
standard const-correctness conventions and confuses readers unless documented
with the specific mutating subsystem operations that necessitate it.

## Examples

Bad:

```cpp
const VulkanSharedContext* vulkanSharedContext() const;
VulkanSharedContext* nonConstVulkanSharedContext();
```

Good:

```cpp
const VulkanSharedContext* vulkanSharedContext() const;
// VulkanSharedContext::pipelineCompileWasRequired() - which drives PipelineCache persistence
// - modifies the SharedContext so, when creating Pipelines, the ResourceProvider must
// provide a non-const SharedContext.
VulkanSharedContext* nonConstVulkanSharedContext();
```
