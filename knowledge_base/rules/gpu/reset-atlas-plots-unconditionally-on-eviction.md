---
name: skia/gpu/reset-atlas-plots-unconditionally-on-eviction
description: When evicting or recycling GPU atlas plots, invoke internal plot resets unconditionally and encapsulate empty-plot eviction notification checks inside the compound reset helper
rubric: correctness
severity: should_fix
---

**Rule.** Do not guard compound eviction and reset operations with
!plot->isEmpty() at call sites when deactivating or recycling atlas plots.
Encapsulate the empty-plot check inside the helper to skip external eviction
notifications while ensuring internal state resets (such as rect allocators and
generation IDs) run unconditionally.

**Why.** Atlas plots track internal state, such as generation IDs and
sub-allocators, that must be refreshed when a plot is retired or recycled even
if it contains no active rect allocations. Guarding the compound helper at call
sites inadvertently skips these essential internal resets, leaving stale state
that can corrupt subsequent allocations.

## Examples

Bad:

```cpp
// Guarding compound reset at call site bypasses internal plot state reset
if (!currPlot->isEmpty()) {
    this->processEvictionAndResetRects(currPlot, /*freeData=*/true);
}
```

Good:

```cpp
// Call compound helper unconditionally
this->processEvictionAndResetRects(currPlot, /*freeData=*/true);

// Encapsulate empty check inside helper around notification only
inline void processEvictionAndResetRects(Plot* plot, bool freeData) {
    if (!plot->isEmpty()) {
        this->processEviction(plot->plotLocator());
    }
    plot->resetRects(freeData);
}
```
