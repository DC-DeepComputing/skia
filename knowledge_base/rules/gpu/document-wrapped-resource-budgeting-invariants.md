---
name: skia/gpu/document-wrapped-resource-budgeting-invariants
description: When documenting GPU resource budgeting invariants, clarify that resources backing client-owned objects and wrapper objects for client-provided backend handles never count against cache limits
rubric: readability
severity: nit
---

**Rule.** When documenting GPU resource budgeting invariants or cache limits,
clarify that resources backing client-owned objects (such as SkSurface or
SkImage) and wrapper objects for client-provided backend handles (such as
BackendTexture with Ownership::kWrapped) do not count against cache limits and
must never be budgeted. Explicitly define what constitutes a wrapper object with
concrete examples to distinguish client-provided handles from internal resource
wrapping.

**Why.** Graphite only tracks internally created shared or available scratch
resources against cache limits. Client-owned and client-wrapped resources are
not managed or evictable by the cache budget; imprecise comments that describe
wrapped objects ambiguously can lead maintainers to confuse client wrappers with
internal cache management or incorrectly budget external handles.

## Examples

Bad:

```cpp
// Resources that back client owned objects (e.g. SkSurface or SkImage) are not budgeted and do not count against cache limits. Wrapped objects (not inserted in a cache) are never budgeted.
Budgeted fBudgeted = Budgeted::kNo;
```

Good:

```cpp
// Resources that back client-owned objects (e.g. SkSurface or SkImage) and wrapper objects (e.g. BackendTexture) do not count against cache limits and therefore should never be budgeted.
Budgeted fBudgeted = Budgeted::kNo;
```
