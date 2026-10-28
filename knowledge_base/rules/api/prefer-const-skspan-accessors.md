---
name: skia/api/prefer-const-skspan-accessors
description: When exposing collection or query accessors for stable data, return SkSpan<const T> or const container references instead of returning containers by value or copying elements
rubric: safety
severity: blocking
---

**Rule.** When exposing collection, query, or buffer accessors for stable
contiguous data, return SkSpan<const T> (or return container members by const
reference when a span view is not applicable) instead of returning concrete
containers (such as std::vector or skia_private::TArray) by value, wrapping
container returns in std::optional, or copying elements into caller buffers;
rely on an empty span or container to represent absence. For streamed or
computed elements, expose an index accessor (e.g. returning std::optional<T>) or
an iterator range. Do not expose non-const span or buffer accessors (such as
SkSpan<T> or non-const data()) when callers only need read-only access. On
immutable or thread-shared types (such as SkData), restrict mutation to explicit
methods like writable_data().

**Why.** Returning containers by value or copying into caller-provided arrays
forces redundant heap allocations and copying overhead when callers only need
read access, while wrapping collection return types in std::optional creates
redundant empty states when an empty span already indicates absence. Returning
SkSpan<const T>, const container references, index accessors, or iterator ranges
enables zero-allocation, zero-copy access while preserving caller flexibility.
Restricting read-only accessors to const views and requiring explicit methods
like writable_data() on shared types also prevents unintended mutation and
silent data races.

## Examples

Bad:

```cpp
// 1. Returning containers by value or wrapping in std::optional:
std::vector<SlotInfo> getSlotInfo();
std::vector<LayerInfo> getLayerInfo() const { return fLayerInfo; }
std::optional<skia_private::TArray<Range>> getRanges() const;
int getVerbs(uint8_t verbs[], int max) const;

// 2. Non-const span accessor exposes internal storage when callers only read:
SkSpan<SkPoint> points() { return {fPoints, fNumPoints}; }
SkSpan<const SkPoint> points() const { return {fPoints, fNumPoints}; }

// 3. Non-const data() on immutable/shared type enables silent data races:
const std::byte* data() const { return fSpan.data(); }
std::byte* data() { return fSpan.data(); }
```

Good:

```cpp
// 1. Return SkSpan<const T> or const container reference:
SkSpan<const LayerInfo> getLayerInfo() const { return SkSpan(fLayerInfo); }
const std::vector<SlotInfo>& getSlotInfo() const { return fSlotInfos; }
SkSpan<const Range> getRanges() const { return fRanges; }
SkSpan<const SkPathVerb> verbs() const { return fVerbs; }
std::optional<SkScalar> getSegmentDistance(size_t index) const;

// 2. Const accessor returning const elements safely serves all callers:
SkSpan<const SkPoint> points() const { return {fPoints, fNumPoints}; }

// 3. Immutable/shared data buffer restricts mutation to explicit cautionary method:
const void* data() const { return fSpan.data(); }
void* writable_data() { return fSpan.data(); }
```
