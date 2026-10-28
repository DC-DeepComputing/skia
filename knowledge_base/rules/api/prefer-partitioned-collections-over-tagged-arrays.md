---
name: skia/api/prefer-partitioned-collections-over-tagged-arrays
description: When exposing query or introspection APIs for categorized items, return partitioned typed collections instead of a single flattened array with synthetic enum type tags
rubric: design
severity: should_fix
---

**Rule.** When designing query, inspection, or introspection APIs that expose
categorized or heterogeneous items, return separate partitioned typed
collections (such as distinct container fields in a result struct) rather than
flattening items into a single array with synthetic type-tag enums. Avoid
maintaining redundant cached collections of tagged items; generate partitioned
results on demand from authoritative data structures.

**Why.** Flattening categorized items into a single array with synthetic
type-tag enums forces callers to write boilerplate switch statements or
filtering loops to demultiplex elements by category. Returning partitioned typed
collections directly simplifies caller consumption, eliminates artificial tag
enums, and avoids redundant state caching and synchronization bookkeeping.

## Examples

Bad:

```cpp
enum class SlotType { kColor, kScalar, kImage, kText };
struct SlotInfo {
    SlotID   fSlotID;
    SlotType fType;
};

// Flattened array forces callers to loop and switch on fType; redundant cached state
const skia_private::TArray<SlotInfo>& getSlotInfo() const { return fSlotInfos; }
```

Good:

```cpp
struct SlotInfo {
    skia_private::TArray<SlotID> fColorSlotIDs;
    skia_private::TArray<SlotID> fScalarSlotIDs;
    skia_private::TArray<SlotID> fImageSlotIDs;
    skia_private::TArray<SlotID> fTextSlotIDs;
};

// Partitioned struct returned on demand; callers access categories directly without tags
SlotInfo getSlotInfo() const;
```
