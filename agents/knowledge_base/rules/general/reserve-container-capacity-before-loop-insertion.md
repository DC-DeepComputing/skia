---
name: skia/reserve-container-capacity-before-loop-insertion
description: When populating dynamic containers with known element counts, preallocate capacity or bulk copy contiguous buffers instead of incremental appends or shrink_to_fit
rubric: safety
severity: should_fix
---

**Rule.** When populating dynamic containers (such as skia_private::TArray or
std::vector) or builder objects when the element count is known or computable up
front, preallocate capacity using reserve() or reserve_exact() before inserting
elements. Do not rely on incremental appends followed by shrink_to_fit();
pre-reserve container capacity or pass a pre-built, pre-reserved container into
constructors and factory methods instead. When copying from a contiguous buffer
or array of trivially copyable types, bulk copy (via resize() and std::memcpy,
range assign/construction, or push_back_n) rather than looping emplace_back or
push_back.

**Why.** Dynamically appending to containers without pre-reserving capacity
triggers repeated heap reallocations and element copies as the buffer grows,
while calling shrink_to_fit() afterwards incurs another allocation, copy, and
deallocation. Looping scalar push_back or emplace_back over contiguous trivially
copyable buffers also incurs per-element loop overhead compared to
single-allocation bulk memory copies.

## Examples

Bad:

```cpp
// Variant 1: Populating builder without reserving known capacity
outMapping->reserve(lines.size());
for (const auto& line : lines) {
    outPolyline->appendPoint({line.p0.fX, line.p0.fY});
    outPolyline->appendPoint({line.p1.fX, line.p1.fY});
    outPolyline->appendSentinel();
}

// Variant 2: Incremental appends followed by shrink_to_fit()
auto group = sksg::Group::Make();
for (auto& child : children) {
    group->addChild(std::move(child));
}
group->shrink_to_fit();

// Variant 3: Looping scalar emplace_back over contiguous buffer
std::vector<size_t> vec;
for (int i = 0; i < count; i++) {
    vec.emplace_back(data[i]);
}
```

Good:

```cpp
// Variant 1: Preallocate capacity before loop insertion
outMapping->reserve(lines.size());
outPolyline->reserve(lines.size() * 3);
for (const auto& line : lines) {
    outPolyline->appendPoint({line.p0.fX, line.p0.fY});
    outPolyline->appendPoint({line.p1.fX, line.p1.fY});
    outPolyline->appendSentinel();
}

// Variant 2: Pre-reserve container and pass to factory
std::vector<sk_sp<sksg::RenderNode>> children;
children.reserve(child_count);
for (size_t i = 0; i < child_count; ++i) {
    children.push_back(...);
}
auto group = sksg::Group::Make(std::move(children));

// Variant 3: Pre-resize and bulk copy contiguous buffer
std::vector<size_t> vec;
vec.resize(count);
std::memcpy(vec.data(), data, count * sizeof(size_t));
```
