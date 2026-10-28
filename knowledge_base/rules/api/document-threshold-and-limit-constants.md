---
name: skia/api/document-threshold-and-limit-constants
description: When declaring configuration structs, compile-time tuning constants, capacity limits, or reserved ID partitions, document controlling subsystems, units, layout constraints, cache-invalidation consequences, or migration TODOs
rubric: readability
severity: should_fix
---

**Rule.** When defining configuration structs, tuning parameters, capacity
limits, or reserved ID partitions in headers or source files (such as
SparseStripConfig, kMaxGPUPathRendererVerbs, or kSkiaBuiltInReservedCnt),
document their intent with clarifying comments. Group parameters by the
subsystem, operation, or phase they control, specify units, cross-module
constraints, and memory layout or dimension mappings, and explain which
implementations enforce limits, why sizing values were chosen, and what hazards
(such as OOM or thrashing) they prevent. Additionally, if modifying reserved
counts or partition boundaries invalidates caches (such as pipeline UniqueKeys)
or breaks compatibility, document these consequences and any invalidation TODOs.

**Why.** Undocumented configuration structs, arbitrary threshold limits, and
unannotated partition constants leave maintainers uncertain why specific values
were chosen, which subsystems depend on them, or how modifying them impacts
downstream state. Clear comments prevent misconfiguration, buffer under-sizing,
and subtle regressions, while documenting invalidation consequences and
migration TODOs ensures maintainers know when modifications require cache
flushes or version bumps.

## Examples

Bad:

```cpp
// Bad: Arbitrary threshold constants without clarifying which operations they govern
static constexpr auto kPlotRecentlyUsedCountForCompact = 32;
static constexpr auto kPlotRecentlyUsedCountForPurge = 4;

// Bad: Undocumented temporary limit on a base class constrained by a legacy subclass
class DrawListBase {
public:
    static constexpr int kMaxRenderSteps = 4096;
    static_assert(kMaxRenderSteps <= std::numeric_limits<uint16_t>::max());
};

// Bad: Reserved partition counts without documenting cache invalidation consequences
static constexpr int kSkiaBuiltInReservedCnt = 100;
static constexpr int kSkiaKnownRuntimeEffectsReservedCnt = 100;
```

Good:

```cpp
// Good: Explains the flush units, tuning rationale, and which operations each threshold governs
// Atlas flushes beyond which a plot is considered unused, for compact() and purge().
static constexpr auto kPlotRecentlyUsedCountForCompact = 32;
static constexpr auto kPlotRecentlyUsedCountForPurge = 2;

// Good: Attributed TODO clarifies the limit is temporary debt tied to a legacy subclass
class DrawListBase {
public:
    // TODO(username): Remove or increase this limit once DrawListLayer replaces legacy DrawList
    static constexpr int kMaxRenderSteps = 4096;
    static_assert(kMaxRenderSteps <= std::numeric_limits<uint16_t>::max());
};

// Good: Documents invalidation consequences and tracks dynamic invalidation with a TODO
// WARNING: If any of these values are changed, UniqueKeys that have stably-keyed effects
// will need to be invalidated.
// TODO(b/238759147): add a revision number that can drive the invalidation.
static constexpr int kSkiaBuiltInReservedCnt = 500;
static constexpr int kSkiaKnownRuntimeEffectsReservedCnt = 500;
```
