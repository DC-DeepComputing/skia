---
name: skia/gpu/encapsulate-draw-atlas-index-calculations
description: When mapping PlotLocator to linear plot indices in atlas managers, expose calculation methods on DrawAtlas rather than duplicating layout arithmetic in file-local helpers
rubric: design
severity: should_fix
---

**Rule.** When calculating linear plot indices or mapping coordinates from a
PlotLocator in atlas managers (such as ClipAtlasManager or PathAtlas), expose
and invoke index calculation methods directly on DrawAtlas (such as
DrawAtlas::getListIndex(PlotLocator)). Do not reimplement atlas layout
arithmetic or flat-array index formulas in file-local static helper functions
across individual manager files.

**Why.** Atlas managers track per-plot data in flat arrays whose indexing
depends on DrawAtlas's internal layout (pageIndex * numPlots + plotIndex).
Encapsulating layout calculations as member methods on DrawAtlas keeps internal
atlas geometry unified, avoids code duplication across managers, and prevents
index calculation drift if the underlying plot layout changes.

## Examples

Bad:

```cpp
// In ClipAtlasManager.cpp
static uint32_t mask_key_list_index(const PlotLocator& locator, const DrawAtlas* drawAtlas) {
    return locator.pageIndex() * drawAtlas->numPlots() + locator.plotIndex();
}
...
uint32_t index = mask_key_list_index(locator.plotLocator(), fDrawAtlas.get());
fKeyLists[index].addToTail(keyEntry);
```

Good:

```cpp
// In DrawAtlas.h
uint32_t getListIndex(const PlotLocator& locator) const {
    return locator.pageIndex() * fNumPlots + locator.plotIndex();
}

// In ClipAtlasManager.cpp
uint32_t index = fDrawAtlas->getListIndex(locator.plotLocator());
fKeyLists[index].addToTail(keyEntry);
```
