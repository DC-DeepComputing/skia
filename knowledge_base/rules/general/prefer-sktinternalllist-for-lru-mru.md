---
name: skia/prefer-sktinternalllist-for-lru-mru
description: When maintaining MRU or LRU ordering across collections of objects, use SkTInternalLList instead of manually shifting array elements and tracking indices
rubric: readability
severity: should_fix
---

**Rule.** Use SkTInternalLList (paired with SK_DECLARE_INTERNAL_LLIST_INTERFACE)
to maintain MRU or LRU order across collections of objects rather than manually
shifting array elements and tracking element indices. Promote accessed items to
the front with .remove(entry) followed by .addToHead(entry).

**Why.** Maintaining MRU or LRU ordering by shifting array elements on every
access requires O(N) element copies or moves and manual tracking of updated
element indices, adding code complexity and risk of off-by-one index corruption.
Skia's intrusive doubly-linked list SkTInternalLList provides O(1) removals and
insertions without extra dynamic allocations or index synchronization.

## Examples

Bad:

```cpp
// Shifting array elements and manually updating element indices on every MRU access:
std::unique_ptr<Plot>* plotArray = fPages[pageIdx].fPlotArray.get();
std::unique_ptr<Plot> mruPlot = std::move(plotArray[plotIndex]);
for (int i = plotIndex; i > 0; --i) {
    plotArray[i] = std::move(plotArray[i - 1]);
    plotArray[i]->updateIndexInPageMru(i);
}
plotArray[0] = std::move(mruPlot);
plotArray[0]->updateIndexInPageMru(0);
```

Good:

```cpp
// Promoting accessed item to head using SkTInternalLList in O(1):
if (fPages[pageIdx].fPlotList.head() != plot) {
    fPages[pageIdx].fPlotList.remove(plot);
    fPages[pageIdx].fPlotList.addToHead(plot);
}
```
