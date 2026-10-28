---
name: skia/api/avoid-new-features-in-legacy-ganesh
description: When developing new GPU capabilities, do not add new features, optional parameters, or public API extensions to legacy Ganesh without explicit client need
rubric: design
severity: should_fix
---

**Rule.** Do not add new features, optional parameters, or public API
capabilities to the legacy Ganesh backend (`include/gpu/ganesh/**`,
`src/gpu/ganesh/**`) when developing functionality for Graphite unless there is
an explicit requirement or active client need. Direct new GPU capabilities,
configuration options, and architectural extensions exclusively to Graphite.

**Why.** Ganesh is Skia's legacy GPU backend and is maintained only for existing
client integrations as the codebase migrates to Graphite. Extending Ganesh with
new features or optional configuration adds dead code and maintenance overhead
without clear client benefit, conflicting with migration goals.

## Examples

Bad:

```cpp
// Adding new optional configuration or feature parameters to legacy Ganesh API:
void performDeferredCleanup(
        std::chrono::milliseconds msNotUsed,
        GrPurgeResourceOptions opts = GrPurgeResourceOptions::kAllResources,
        const std::chrono::milliseconds* stopPurgingPastDuration = nullptr);
```

Good:

```cpp
// Keep legacy Ganesh signatures frozen; implement new features only in Graphite:
void performDeferredCleanup(
        std::chrono::milliseconds msNotUsed,
        GrPurgeResourceOptions opts = GrPurgeResourceOptions::kAllResources);
```
