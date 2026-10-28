---
name: skia/gpu/document-draw-traversal-pointer-ratcheting
description: When ordering or batching draws in Graphite draw lists, document multi-step ordering invariants, pointer ratcheting, and search-budget heuristics in inline comments
rubric: readability
severity: nit
---

**Rule.** When implementing draw list traversal and layer batching in Graphite,
document non-obvious execution-ordering invariants, list-advancement logic, and
search heuristics in inline comments. Explicitly explain why search boundaries
ratchet forward (such as selecting `start.fList->fNext` to prevent child steps
from self-intersecting with parent steps), why aggregate stencil flags apply
across all steps to maintain layer atomicity, and why traversal halts on overlap
(such as returning `isStencil`). Additionally, document the rationale for
search-budget adjustments and limits (such as subtracting `kMaxSearchLimit >>
1`), explaining how deeper searches reduce pipeline changes while bounding
worst-case complexity.

**Why.** In multi-step rendering pipelines (such as stencil-and-cover), child
steps depend on parent steps and must execute sequentially without interleaving
other draws between them. Without inline documentation, pointer advancement can
be misinterpreted as skipping unrelated layers rather than avoiding
self-intersections, aggregate stencil classification appears to contradict
per-step flags, and stale comments can obscure why traversals halt early.
Furthermore, heuristic search-limit adjustments look like arbitrary magic
numbers unless documented with their optimization benefit and algorithmic
complexity bounds.

## Examples

Bad:

```cpp
// 1) Stale comment claims unconditional search despite halting on stencil overlap:
// If !dependsOnDst, we just keep searching backwards.
return isStencil;

// 2) Pointer ratcheting and aggregate stencil flags undocumented:
BindingList* bound = isStencil ? start.fList->fNext : start.fList;

// 3) Search budget penalty and batching heuristic undocumented:
if (overlapType == BoundsTest::kCompatibleOverlap) {
    if (isStencil) {
        limit -= kMaxSearchLimit >> 1;
    } else {
        return true;
    }
}
```

Good:

```cpp
// 1) Inline comment documents multi-step atomicity invariant preventing interleaving:
// If !dependsOnDst and not a stencil, keep searching backwards; if a stencil,
// halt traversal here so insertion cannot interleave between parent and child steps.
return isStencil;

// 2) Inline comment explains pointer ratcheting past parent step:
// If we are a stencil renderer, ratchet starting list forward past parent step to avoid self-intersection.
BindingList* bound = isStencil ? start.fList->fNext : start.fList;

// 3) Inline comment explains batching benefit and worst-case complexity bound:
// In stencil-heavy scenes, search deeper than the first compatible overlap to find denser
// candidates and reduce pipeline changes, penalizing search budget to bound worst-case complexity.
if (overlapType == BoundsTest::kCompatibleOverlap) {
    if (isStencil) {
        limit -= kMaxSearchLimit >> 1;
    } else {
        return true;
    }
}
```
