---
name: skia/modules/bentleyottmann-reuse-sweep-line-cleanup-helper
description: When removing ending segments from sweep-line status collections in polygon division, call existing cleanup helpers rather than inlining erase-remove loops
rubric: design
severity: should_fix
---

**Rule.** When removing segments or cleaning up status collections during
sweep-line processing, call existing member cleanup helpers (such as
handleEndings) instead of inlining manual std::remove and erase loops. Reuse
common status line update helpers across all event handling paths to keep
assertion invariants and container mutations single-sourced.

**Why.** Inlining container removal and status cleanup logic across multiple
event handlers duplicates container mutation boilerplate and risks bug
divergence or missing debug assertions when collection invariants change.

## Examples

Bad:

```cpp
for (const Segment& s : horizontals) {
    auto removedPoint = std::remove(fStatus.begin(), fStatus.end(), s);
    SkASSERT(removedPoint != fStatus.end());
    fStatus.erase(removedPoint, fStatus.end());
}
```

Good:

```cpp
this->handleEndings(horizontals);
```
