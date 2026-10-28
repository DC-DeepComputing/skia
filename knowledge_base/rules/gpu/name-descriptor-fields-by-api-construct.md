---
name: skia/gpu/name-descriptor-fields-by-api-construct
description: When defining backend descriptor or metadata struct fields in GPU code, name fields after generic API constructs rather than frontend renderer use cases
rubric: readability
severity: should_fix
---

**Rule.** Name low-level GPU backend descriptor and metadata struct fields (such
as render pass, pipeline, or attachment descriptors) after their structural
API/hardware construct or attachment type (e.g., `fHasInputAttachment`) rather
than high-level renderer use cases (e.g., `fSupportReadingDstAsInput`). Keep
backend configuration structs agnostic to why higher-level layers requested the
configuration so descriptors remain modular and reusable.

**Why.** Low-level backend structs, render passes, and pipeline keys operate on
hardware and driver API states (such as subpass dependencies and input
attachments), not renderer intentions (such as reading from dst). Generic
structural naming decouples backend implementations from frontend rendering
strategies and prevents misleading or redundant field names when multiple
features share the same API mechanism.

## Examples

Bad:

```cpp
// Bad: Naming a backend descriptor field after a high-level renderer use case.
struct VulkanRenderPassMetaData {
    bool fLoadMSAAFromResolve;
    bool fSupportReadingDstAsInput;
};
```

Good:

```cpp
// Good: Naming the field after the structural API construct or attachment type.
struct VulkanRenderPassMetaData {
    bool fLoadMSAAFromResolve;
    bool fHasInputAttachment;
};
```
