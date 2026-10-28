---
name: skia/do-not-add-container-size-to-incremental-reserve
description: When calling incremental container reservation APIs like incReserve or reserve_back, pass only the additional element count rather than adding container size
rubric: correctness
severity: should_fix
---

**Rule.** Do not pass container.size() + extra_count to incremental container or
builder reservation methods (such as incReserve or reserve_back). Incremental
reservation APIs add current element count internally; pass only the additional
element count (extra_count) to avoid double-counting existing elements and
over-allocating storage. When total capacity is intended, call absolute
reservation methods (such as reserve()) instead.

**Why.** Incremental reservation methods expect only the additional element
count beyond current size. Passing size() + extra effectively reserves 2 *
size() + extra, causing excessive heap allocation and premature buffer growth.

## Examples

Bad:

```cpp
void SkPathBuilder::incReserve(int extraPtCount, int extraVbCount) {
    // reserve_back() already adds current size; adding size() double-counts existing elements
    fPts.reserve_back(Sk32_sat_add(fPts.size(), extraPtCount));
    fVerbs.reserve_back(Sk32_sat_add(fVerbs.size(), extraVbCount));
}
```

Good:

```cpp
void SkPathBuilder::incReserve(int extraPtCount, int extraVbCount) {
    // Pass only the delta of additional elements to incremental reservation APIs
    fPts.reserve_back(extraPtCount);
    fVerbs.reserve_back(extraVbCount);

    // Alternatively, call reserve() when calculating total capacity
    // fPts.reserve(Sk32_sat_add(fPts.size(), extraPtCount));
}
```
