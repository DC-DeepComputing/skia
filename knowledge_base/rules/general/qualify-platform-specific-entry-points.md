---
name: skia/qualify-platform-specific-entry-points
description: When defining platform-specific or specialized entry points, qualify function and header names explicitly instead of reusing generic public API names
rubric: readability
severity: should_fix
---

**Rule.** Do not reuse generic public API function names or misleading 'Public'
header prefixes for platform-specific, internal, or specialized entry points.
Explicitly qualify function and header names with the targeted platform or
subsystem scope (e.g., AndroidSpecific...) to clearly distinguish them from
canonical public API functions.

**Why.** Overloading generic public function names or naming internal headers
with 'Public' prefixes creates ambiguity about which interface is canonical and
where callers should direct requests. Explicit qualification prevents collision
with public API declarations, clarifies caller intent, and avoids confusion for
maintainers auditing API entry points.

## Examples

Bad:

```cpp
// In src/gpu/graphite/PublicPrecompile.h
bool Precompile(PrecompileContext*,
                RuntimeEffectDictionary* rteDict,
                const GraphicsPipelineDesc& pipelineDesc,
                const RenderPassDesc& renderPassDesc);
```

Good:

```cpp
// In src/gpu/graphite/AndroidSpecificPrecompile.h
bool AndroidSpecificPrecompile(PrecompileContext*,
                               RuntimeEffectDictionary* rteDict,
                               const GraphicsPipelineDesc& pipelineDesc,
                               const RenderPassDesc& renderPassDesc);
```
